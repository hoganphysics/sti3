from collections.abc import Callable
import builtins
import importlib.util
from os import PathLike
import pathlib
import sys
import threading
import uuid
import warnings

from stipy.stipy import makeshot as _global_makeshot
from stipy.stipybase.stipybase import RawEventGroup
from stipy.stipybase.stipybase import ShotType


_IMPORT_ISOLATION_LOCK = threading.RLock()
_STIPY_PACKAGE_ROOT = pathlib.Path(__file__).resolve().parents[1]


def _is_relative_to(path, root):
    try:
        path.relative_to(root)
    except ValueError:
        return False
    return True


def _resolved_path(path):
    return pathlib.Path(path).expanduser().resolve()


def _unique_paths(paths):
    unique = []
    for path in paths:
        resolved = _resolved_path(path)
        if resolved not in unique:
            unique.append(resolved)
    return unique


def _import_root_paths(main_file, import_roots):
    roots = [pathlib.Path(main_file).resolve().parent]

    if import_roots is not None:
        if isinstance(import_roots, (str, PathLike)):
            roots.append(import_roots)
        else:
            try:
                roots.extend(import_roots)
            except TypeError as exc:
                raise ValueError("import_roots must be a path or iterable of paths.") from exc

    return _unique_paths(roots)


def _protected_roots():
    roots = [_STIPY_PACKAGE_ROOT]
    for prefix in (sys.prefix, sys.base_prefix, sys.exec_prefix, sys.base_exec_prefix):
        if prefix:
            roots.append(pathlib.Path(prefix).resolve())
    return _unique_paths(roots)


def _is_protected_path(path, protected_roots):
    return any(_is_relative_to(path, root) for root in protected_roots)


def _module_path(path):
    if path is None:
        return None
    if not isinstance(path, (str, bytes, PathLike)):
        return None

    path_text = str(path)
    if path_text in {"built-in", "frozen"} or path_text.startswith("<"):
        return None

    return pathlib.Path(path).resolve()


def _module_paths(module):
    if module is None:
        return []

    paths = []
    path = _module_path(getattr(module, "__file__", None))
    if path is not None:
        paths.append(path)

    spec = getattr(module, "__spec__", None)
    if spec is not None:
        path = _module_path(getattr(spec, "origin", None))
        if path is not None:
            paths.append(path)

    module_path = getattr(module, "__path__", None)
    if module_path is not None:
        for entry in module_path:
            path = _module_path(entry)
            if path is not None:
                paths.append(path)

    return _unique_paths(paths)


def _module_is_under_roots(module, roots, protected_roots):
    for path in _module_paths(module):
        if _is_protected_path(path, protected_roots):
            continue
        if any(_is_relative_to(path, root) for root in roots):
            return True
    return False


def _path_is_under_roots(path, roots):
    return any(_is_relative_to(path, root) for root in roots)


def _private_module_name(path):
    safe_stem = "".join(ch if ch.isalnum() or ch == "_" else "_" for ch in path.stem)
    return "_stipy_makeshot_{0}_{1}".format(uuid.uuid4().hex, safe_stem)


class _IsolatedTimingImports:
    def __init__(self, main_file, import_roots):
        self.main_file = pathlib.Path(main_file).resolve()
        self.roots = _import_root_paths(self.main_file, import_roots)
        self.protected_roots = _protected_roots()
        self.start_module_names = set()
        self.outside_root_stipy_imports = set()
        self.original_import = None
        self.original_sys_path = None

    def __enter__(self):
        _IMPORT_ISOLATION_LOCK.acquire()
        try:
            self.original_import = builtins.__import__
            self.original_sys_path = list(sys.path)
            self._prepend_import_roots()
            importlib.invalidate_caches()
            self._remove_timing_modules(sys.modules.keys())
            self.start_module_names = set(sys.modules)
            builtins.__import__ = self._import
            return self
        except Exception:
            self._restore_import()
            self._restore_sys_path()
            _IMPORT_ISOLATION_LOCK.release()
            raise

    def __exit__(self, exc_type, exc, traceback):
        try:
            try:
                self._restore_import()
                new_module_names = set(sys.modules) - self.start_module_names
                self._remove_timing_modules(new_module_names)
            finally:
                self._restore_sys_path()
            self._warn_outside_root_stipy_imports()
        finally:
            _IMPORT_ISOLATION_LOCK.release()

    def _prepend_import_roots(self):
        for root in reversed(self.roots):
            entry = str(root)
            while entry in sys.path:
                sys.path.remove(entry)
            sys.path.insert(0, entry)

    def _restore_import(self):
        if self.original_import is not None:
            builtins.__import__ = self.original_import

    def _restore_sys_path(self):
        if self.original_sys_path is not None:
            sys.path[:] = self.original_sys_path

    def _import(self, name, globals=None, locals=None, fromlist=(), level=0):
        self._record_outside_root_stipy_import(name, globals, level)
        return self.original_import(name, globals, locals, fromlist, level)

    def _record_outside_root_stipy_import(self, name, globals, level):
        if level != 0 or name.split(".", 1)[0] != "stipy":
            return

        caller_file = _module_path((globals or {}).get("__file__"))
        if caller_file is None:
            return
        if _is_protected_path(caller_file, self.protected_roots) or _path_is_under_roots(caller_file, self.roots):
            return

        self.outside_root_stipy_imports.add(caller_file)

    def _remove_timing_modules(self, module_names):
        names_to_remove = set()
        for name in list(module_names):
            module = sys.modules.get(name)
            if _module_is_under_roots(module, self.roots, self.protected_roots):
                names_to_remove.add(name)

        for name in names_to_remove:
            sys.modules.pop(name, None)

    def _warn_outside_root_stipy_imports(self):
        for path in sorted(self.outside_root_stipy_imports):
            warnings.warn(
                "Module {0} imported during makeshot appears to use STIPy "
                "but is outside the timing import roots. Add its directory to "
                "import_roots if it should be reloaded for each shot.".format(path),
                RuntimeWarning,
                stacklevel=2,
            )


def load_module(path, name=None):
    path = pathlib.Path(path).resolve()

    dir_path = str(path.parent)
    if dir_path not in sys.path:
        sys.path.insert(0, dir_path)

    spec = importlib.util.spec_from_file_location(name or path.stem, str(path))
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def _call_makeshot(raw_makeshot, owner, *args):
    if owner is None:
        return raw_makeshot(*args)
    return raw_makeshot(owner, *args)


def _overwritten_vars(vars):
    g = RawEventGroup()

    if isinstance(vars, dict):
        for key, value in vars.items():
            g.bindvar(key, value)
    elif isinstance(vars, set):
        for var in vars:
            g.bindvar(var.name, var.value())
    else:
        raise ValueError("Vars must be a dictionary or a set of ParsedVar objects.")

    return g.overwrittenVars()


def _makeshot_callable(raw_makeshot, owner, shotmaker, vars, shot_type):
    if vars is None:
        return _call_makeshot(raw_makeshot, owner, shotmaker, shot_type)
    return _call_makeshot(raw_makeshot, owner, shotmaker, _overwritten_vars(vars), shot_type)


def _makeshot_file(raw_makeshot, owner, filename, vars, shot_type, import_roots):
    main_file = pathlib.Path(filename).resolve()

    def execute_file():
        with _IsolatedTimingImports(main_file, import_roots):
            load_module(main_file, _private_module_name(main_file))

    if vars is None:
        return _call_makeshot(raw_makeshot, owner, execute_file, str(main_file), shot_type)
    return _call_makeshot(raw_makeshot, owner, execute_file, _overwritten_vars(vars), str(main_file), shot_type)


def make_shot(raw_makeshot, owner=None, source=None, vars=None, shot_type=None, import_roots=None):
    if shot_type is None:
        shot_type = ShotType.Single

    if source is None and vars is None:
        if import_roots is not None:
            raise ValueError("import_roots is only supported for filename sources.")
        return _call_makeshot(raw_makeshot, owner, shot_type)
    if isinstance(source, (str, PathLike)):
        return _makeshot_file(raw_makeshot, owner, source, vars, shot_type, import_roots)
    if isinstance(source, Callable):
        if import_roots is not None:
            raise ValueError("import_roots is only supported for filename sources.")
        return _makeshot_callable(raw_makeshot, owner, source, vars, shot_type)

    raise ValueError("Source must be a string filename, pathlib path, or callable.")


def makeshot(source=None, vars=None, shot_type=None, import_roots=None):
    return make_shot(
        _global_makeshot,
        source=source,
        vars=vars,
        shot_type=shot_type,
        import_roots=import_roots,
    )
