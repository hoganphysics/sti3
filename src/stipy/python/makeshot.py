from collections.abc import Callable
import importlib.util
from os import PathLike
import pathlib
import sys

from stipy.stipy import makeshot as _global_makeshot
from stipy.stipybase.stipybase import RawEventGroup
from stipy.stipybase.stipybase import ShotType


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


def _makeshot_file(raw_makeshot, owner, filename, vars, shot_type):
    main_file = str(pathlib.Path(filename).resolve())

    def execute_file():
        load_module(main_file)

    if vars is None:
        return _call_makeshot(raw_makeshot, owner, execute_file, main_file, shot_type)
    return _call_makeshot(raw_makeshot, owner, execute_file, _overwritten_vars(vars), main_file, shot_type)


def make_shot(raw_makeshot, owner=None, source=None, vars=None, shot_type=None):
    if shot_type is None:
        shot_type = ShotType.Single

    if source is None and vars is None:
        return _call_makeshot(raw_makeshot, owner, shot_type)
    if isinstance(source, (str, PathLike)):
        return _makeshot_file(raw_makeshot, owner, source, vars, shot_type)
    if isinstance(source, Callable):
        return _makeshot_callable(raw_makeshot, owner, source, vars, shot_type)

    raise ValueError("Source must be a string filename, pathlib path, or callable.")


def makeshot(source=None, vars=None, shot_type=None):
    return make_shot(_global_makeshot, source=source, vars=vars, shot_type=shot_type)
