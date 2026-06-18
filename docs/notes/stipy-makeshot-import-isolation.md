# STIPy file-backed makeshot import isolation plan

Date: 2026-06-18

## Background

`stipy.makeshot("file.py")` and `server.makeshot("file.py")` are intended to
execute timing files inside an active shot-building context. Top-level calls to
`setvar()`, `settag()`, `event()`, `meas()`, and `group()` should therefore add
state to the shot every time `makeshot()` is called.

The current file-backed path re-executes only the submitted main file. Helper
modules imported by that main file use Python's normal `sys.modules` cache. If a
helper file has already been imported in the kernel, Python does not re-run that
helper's top-level code. This breaks timing files that define parse variables at
module import time:

```py
# helper.py
from stipy import *

setvar("load_time", 25_000_000)

def add_events(output):
    event(output, var("load_time"), 0.0)
```

On the first `makeshot("main.py")`, `helper.py` is imported and `setvar()` runs.
On later calls, `helper.py` may be reused from `sys.modules`, so `setvar()` does
not run. Functions imported from the helper can still execute, which explains
why events created inside helper functions may still appear while top-level
helper variables are missing.

The current implementation is in `src/stipy/python/makeshot.py`:

- `_makeshot_file()` creates an `execute_file()` callback.
- `execute_file()` calls `load_module(main_file)`.
- `load_module()` adds the main file's directory to `sys.path`, creates a module
  from the file, stores it in `sys.modules[path.stem]`, and executes it.

This behavior is not isolated enough for timing files in a long-running Jupyter
kernel.

## Goals

- Each file-backed `makeshot()` call should build the shot from a fresh execution
  of the submitted timing-file graph.
- Timing-file modules imported during `makeshot()` should not remain as
  long-term kernel memory after `makeshot()` returns.
- Helper files changed during development should be picked up by the next
  `makeshot()` call without restarting the kernel.
- Globals defined by the submitted timing file should not become globals in the
  caller's Python kernel namespace.
- Stable environment modules such as `stipy`, IPython, Jupyter frontend helpers,
  `numpy`, standard-library modules, and installed site-packages should not be
  evicted or reloaded by default.
- The same semantics should apply to local `stipy.makeshot()` and
  `server.makeshot()`.

## Non-goals

- This is not full process isolation. Mutations to pre-existing non-timing
  modules cannot be rolled back by restoring `sys.modules`.
- This should not depend on IPython-specific autoreload behavior. STIPy should
  work the same in notebooks, scripts, tests, and server workflows.
- This should not run the timing file in a subprocess as a first implementation.
  The current `setvar()` and `event()` functions write into the active C++/pybind
  `STIPyGlobal.currentShot` in the current process. A child process would need a
  serialization protocol to return all vars, tags, groups, events, stack traces,
  devices, and errors to the parent.

## Important limitations

A `sys.modules` snapshot can restore which modules are registered under which
names. It cannot deep-copy and restore arbitrary module internals.

For example, if a timing file does this:

```py
import logging
import random

logging.getLogger().setLevel(logging.DEBUG)
random.seed(123)
```

then the existing `logging` and `random` module objects have been mutated. Import
cleanup cannot undo that. True rollback of arbitrary side effects requires a
separate process.

The practical boundary for file-backed `makeshot()` should be:

- Timing modules are transient.
- Stable libraries are shared process state.
- Timing files should avoid persistent side effects in stable libraries.

## Proposed behavior

File-backed `makeshot()` should run inside a Python import-isolation context.

At a high level:

```py
with isolated_timing_imports(main_file, import_roots):
    execute_main_file_with_private_module_name(main_file)
```

The context should:

1. Save the starting `sys.path`.
2. Add the main file directory to `sys.path`, preserving existing behavior.
3. Temporarily wrap Python imports while the main file executes.
4. Force selected timing modules to import fresh during this `makeshot()` call.
5. Record selected timing modules imported during this `makeshot()` call.
6. In `finally`, remove recorded timing modules from `sys.modules`.
7. Remove the private main-file module from `sys.modules`.
8. Restore `sys.path`.

This gives timing modules transient semantics: they can be imported while
building a shot, but they should not remain cached afterward.

The submitted main file should continue to execute in its own module globals,
not in the caller's globals. This preserves the current useful behavior where:

```py
# main_a.py
x = 1
```

does not make `x` available as a bare name in the notebook or in an unrelated
later `makeshot("main_b.py")` call.

## Timing module policy

The import wrapper can see import requests as they happen, but it still needs a
policy for deciding which modules are timing modules and which are stable
libraries.

Use path-based classification, not import-name classification.

Reasons:

- Local timing files often use ordinary absolute imports such as
  `import mot_helpers` or `from ramps import common`.
- Installed packages can use relative imports internally.
- `from stipy import *` is common in timing files but is not a reliable
  classifier. Non-timing notebook/frontend code can import `stipy`, and helper
  modules may indirectly use STIPy without importing it themselves.

Default timing roots:

- The submitted main file's directory.
- Subdirectories under that directory.
- Do not include the current working directory by default. This avoids
  accidentally treating notebook, frontend, or other kernel support code as
  transient timing code.

Optional timing roots:

- Add an `import_roots` keyword to file-backed `makeshot()`:

```py
shot = makeshot("timing/main.py", import_roots=["timing", "shared_timing"])
shot = server.makeshot("timing/main.py", import_roots=["timing", "shared_timing"])
```

Any imported module whose resolved file path is under one of these roots is a
timing module. Built-in modules, frozen modules, standard-library modules,
site-packages, conda packages, and the STIPy package itself should not be treated
as timing modules unless a user explicitly includes such a path as an import
root. Even then, STIPy should probably reject roots that point inside the active
Python environment or the installed STIPy package.

## Import tracking design

The isolation context should wrap `builtins.__import__` only while the main file
is executing.

Before delegating to the real import:

- Resolve the requested module name in the current import context when possible.
- If the requested module, package, or one of its cached children is already in
  `sys.modules` and its `__file__` is under a timing root, remove it from
  `sys.modules` before import.
- This pre-import eviction is needed for robustness. Cleanup at the end prevents
  future stale imports, but it does not fix modules that were already cached
  before the current `makeshot()` call started.

After delegating to the real import:

- Inspect the returned module and relevant `fromlist` children.
- Record any loaded module whose `__file__` is under a timing root.

In `finally`:

- Remove all recorded timing modules from `sys.modules`.
- Also remove any new modules under timing roots found by a final diff of
  `sys.modules`; this catches modules loaded by lower-level import machinery
  that the wrapper did not directly observe.
- Restore `sys.path`.

The default post-condition should be that timing modules are absent from
`sys.modules` after file-backed `makeshot()` returns. This intentionally clears
timing-file modules rather than restoring previously cached timing modules.

## Main file module name

The current `load_module()` uses `path.stem` as the module name. For
`timing/main.py`, that means `sys.modules["main"]` is replaced. This can collide
with unrelated modules named `main`, `server`, `helpers`, etc. It also means two
different timing files with the same basename collide.

The submitted main file should be executed under a private unique name, for
example:

```py
_stipy_makeshot_6f2a1c2d_main
```

That module should be removed from `sys.modules` before `makeshot()` returns.

This does not create a private namespace for all imports. Python's normal import
system still uses process-global `sys.modules`. The isolation context controls
that global state by evicting and cleaning selected timing modules.

## API sketch

Existing calls should keep working:

```py
shot = makeshot("timing/main.py")
shot = server.makeshot("timing/main.py")
```

Add an optional keyword for helper directories outside the main file tree:

```py
shot = makeshot("timing/main.py", import_roots=["timing", "shared_timing"])
shot = server.makeshot("timing/main.py", import_roots=["timing", "shared_timing"])
```

Possible future keyword, if needed:

```py
shot = makeshot("timing/main.py", import_roots=["timing"], isolate_imports=True)
```

`isolate_imports=True` should be the default for filename sources. Callable
sources should keep normal Python behavior because callable-based workflows may
intentionally use already-imported Python functions and objects.

Do not add a legacy opt-out for cached timing-module behavior initially.
Filename-backed `makeshot()` should consistently use import isolation.

## Diagnostics

STIPy should warn when a module imported during file-backed `makeshot()` appears
to be timing-related but is outside all timing roots.

Useful evidence:

- Files recorded in the shot's `StackTraceData` are definitely timing files.
  Calls to `event()`, `meas()`, `setvar()`, `settag()`, and similar STIPy timing
  helpers add stack trace entries to the root event group's `StackTraceData`.
  At the end of `makeshot()`, any file in that list that is outside the timing
  roots should trigger a warning telling the user to add the relevant directory
  to `import_roots` if that file should refresh on each shot.
- Modules imported during file-backed `makeshot()` that directly import `stipy`
  are likely timing-related. This is a weaker signal than `StackTraceData`
  because notebook/frontend code can also import `stipy`, and a helper may use
  STIPy indirectly.
- Files under the default timing root or explicit `import_roots` are timing
  modules by policy, even if they do not directly import STIPy or call STIPy
  helpers themselves.

A useful first import-time heuristic is:

- The module is imported while file-backed `makeshot()` is executing.
- The module's resolved path is outside all timing roots.
- The module imports `stipy` directly, or it exposes evidence that it is using
  STIPy timing helpers.

Preferred direct-import detection:

- Use the temporary import wrapper itself, not a source scan, as the primary
  signal.
- When the wrapper sees an import whose top-level package is `stipy`, inspect
  the caller globals passed to `__import__`.
- If the caller has a `__file__`, and that file is outside all timing roots,
  record it as an outside-root module that directly imported STIPy.
- This catches `import stipy`, `import stipy as st`,
  `from stipy import setvar`, and `from stipy import *`.
- Do not search globals by common symbol names such as `event`, `setvar`, or
  `version`. Those names are too easy to collide with unrelated code.
- If module globals are inspected as a fallback, compare object identity or
  module provenance against the actual loaded `stipy` package rather than
  matching only by symbol name.

This should be cheaper and more accurate than AST scanning because it observes
actual runtime imports while the import-isolation context is already active. AST
scanning can be reserved as a fallback or diagnostic experiment if the import
wrapper misses important cases.

This warning should start as a Python warning using the standard `warnings`
module, not as a STIPy parse warning. `makeshot()` runs before the server parse
step, and the existing engine `ParsingMessage` path belongs to parse results
produced after a shot has already been created.

Example:

```py
import warnings

warnings.warn(
    "Module shared_helpers imported during makeshot appears to use STIPy "
    "but is outside the timing import roots. Add its directory to "
    "import_roots if it should be reloaded for each shot.",
    RuntimeWarning,
)
```

This should be non-fatal. It is intended to help users discover helper
directories that need to be passed through `import_roots`.

The hardest case is a pure support file that does not import `stipy` and does
not call timing helpers directly, but is semantically part of the timing project.
For example, a sibling `common/` directory might define waveform constants or
helper functions used by timing files. Such files may not appear in
`StackTraceData` and may not have any STIPy imports. The planned solution for
that case is explicit `import_roots`.

## Threading and reentrancy

Wrapping `builtins.__import__` is process-global while active. The implementation
should protect file-backed import isolation with a process-local lock.

`STIPyGlobal` already rejects recursive shot creation, but the import wrapper
should also prevent overlapping file-backed `makeshot()` executions in the same
process. This is especially important in Jupyter kernels that may have
background threads or frontend communication code.

The wrapper should be active for the shortest possible time: only while
executing the submitted main file inside the active shot context.

## Why not use IPython autoreload?

IPython autoreload solves a different problem. It reloads modules so interactive
notebook code sees edited function bodies. It does not provide deterministic
shot construction or cleanup.

Problems for STIPy timing files:

- Autoreload can reload a timing helper before `makeshot()` starts. Top-level
  `setvar()` would then run when no shot is active.
- `importlib.reload()` re-executes code in an existing module object. It does
  not clear stale globals reliably.
- Autoreload does not remove timing modules from `sys.modules` after the shot.
- STIPy should not depend on IPython internals because `makeshot()` is also used
  outside notebooks.

## Test plan

Add focused integration tests in `test/integration/python/test_stipy_makeshot_file.py`.

Suggested local tests:

1. Repeated helper top-level `setvar()`
   - Create `main.py` that imports `helper.py`.
   - `helper.py` calls top-level `setvar("helper_var", 1)`.
   - Call `stipy.makeshot(str(main_file))` twice.
   - Assert both shots have `helper_var` in `shot.rootgroup().vars()`.

2. Helper edit is picked up
   - First helper defines `setvar("helper_var", 1)`.
   - First `makeshot()` sees value `1`.
   - Rewrite helper to define value `2`.
   - Second `makeshot()` sees value `2` without restarting Python.

3. Stale pre-imported helper is ignored
   - Import `helper.py` manually before calling `makeshot()`.
   - Change the helper file.
   - `makeshot()` should evict the cached helper and execute the file version
     from disk inside the shot context.

4. Timing modules do not persist
   - After `makeshot()` returns, assert the helper module name is not present in
     `sys.modules`.

5. Stable modules are not evicted
   - Ensure `stipy`, `numpy` if available, and a standard-library module remain
     in `sys.modules` after `makeshot()`.

6. Extra import roots
   - Put `main.py` in one directory and helper timing code in another directory.
   - Pass `import_roots=[main_dir, helper_dir]`.
   - Assert helper top-level `setvar()` runs on repeated calls and helper module
     state is cleaned afterward.

7. Main-file globals do not leak into caller namespace
   - Create `main_a.py` containing `x = 1`.
   - Call `stipy.makeshot(str(main_a))`.
   - Assert `x` is not added to the test function's globals.
   - Create `main_b.py` that references bare `x`.
   - Assert `stipy.makeshot(str(main_b))` raises `NameError`.

Suggested server-backed tests:

- Repeat the core helper top-level `setvar()` test through
  `server.makeshot(str(main_file))` using the existing in-process topology test
  pattern.

Regression coverage should explicitly call `makeshot()` twice in the same Python
process. The existing file-backed tests only exercise one call, which misses this
bug.

## Implementation steps

1. Add an import-isolation helper in `src/stipy/python/makeshot.py`.
2. Change file-backed `_makeshot_file()` to execute the main file inside that
   helper.
3. Use a private unique module name for the main file.
4. Add optional `import_roots` support to local and server Python wrappers.
5. Add repeated-call regression tests before or alongside the implementation.
6. Update `docs/src/stipy.rst` to document file-backed import isolation and
   `import_roots`.

## Open questions

- Should Python warnings from `makeshot()` later be mirrored into a STIPy-owned
  diagnostic object on the shot, separate from engine parse messages?
