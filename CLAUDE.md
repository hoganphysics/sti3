# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

@AGENTS.md

The import above is the source of truth for build/test commands, dependency
boundaries, C++ conventions, and git workflow — don't duplicate it here.
Everything below is either missing from `AGENTS.md` or Claude-specific.

## What this is

STI ("Stanford Timing Interface") is a C++ library/application for building
modular, distributed, hard-timing data acquisition and control systems for
physics experiments. It translates high-level, human-readable timing
instructions (sequences of events on device I/O channels) into low-level
hardware behavior, optionally synchronized to FPGA/DAQ hard-timing hardware,
and lets devices on a network discover and trigger each other.

## Build notes beyond AGENTS.md

- `build-tsan/` is a separate out-of-source build tree configured for
  ThreadSanitizer; normal local development uses `build/`.

## Test notes beyond AGENTS.md

```bash
# Filter to one module while iterating instead of running the full suite
build/test/sti3_test_device "[channel]"
build/test/sti3_test_network "[convert]"
```

Python integration tests are a separate, opt-in suite (not part of the
CTest/Catch2 flow): multi-process/network scenario tests under
`test/integration/python/` (smoke, stress, persistence, partner
distribution, etc.), run via `test/integration/run-python-tests.sh`. The
runner defaults to `build` + the `sti3-build` conda env and spins up a
temporary `omniNames` (CORBA name service) unless `--sti-nameservice` is
given.

```bash
test/integration/run-python-tests.sh                       # default: all non-slow
test/integration/run-python-tests.sh -m "integration and not slow and not observe"
test/integration/run-python-tests.sh -m stress
test/integration/run-python-tests.sh --check-env            # sanity-check Python env
```

See `test/integration/README.md` for the `ProcessTopology` vs in-process
harness and env var overrides (`STI3_BUILD_DIR`, `STI3_CONDA_ENV`,
`STI3_INTEGRATION_PYTHON`).

There are two Catch2 binaries, registered as separate CTest targets:
`sti3_test_device` (`test/device/*_tests.cpp`) and `sti3_test_network`
(`test/network/*_tests.cpp`). New tests are added by listing the file in
`test/CMakeLists.txt`.

## Architecture

The repo is organized as a stack of libraries with one-directional
dependencies; understanding the boundaries matters more than any single file.

```
src/device/src   stidevice   — core domain library, no network/CORBA knowledge
src/network/src  stinetwork  — CORBA/omniORB distribution layer, depends on stidevice
src/server/src   STIServer   — executable: instantiates a Device, exposes it via NetworkDeviceHub
src/stipy        stipy / stidevicepy / stipybase — pybind11 Python bindings
src/stijava      stijava     — SWIG/JNI Java bindings (+ stilib Gradle JAR)
```

`stidevice` never references `stinetwork`. Network capability is added by
wrapping local objects behind abstract interfaces in `include/sti/`
(`FileHolder`, `FileHolderFactory`, `FileServer`, `PersistenceManager`) rather
than leaking CORBA types upward — see the "Dependency boundaries" section of
`AGENTS.md` before adding any cross-module include.

### stidevice (`src/device/src`, headers in `include/sti/device`, `include/sti/engine`)

The device/instrument-control domain model:
- `Device` (abstract) / `LocalDevice` (concrete base for real devices) — implements
  `Device` + `DeviceEventParser` + `EngineTriggerTarget`.
- `Channel` (`include/sti/device/Channel.h`) — typed I/O point (Input/Output) with
  metadata and measurement storage; managed by `LocalChannelManager`.
- `LocalAttributeManager` — device configuration/attributes.
- `EventEngineScheduler` (`include/sti/engine/`) — parses and plays `Sequence`s and
  `Shot`s built from `RawEvent`/`SynchronousEvent`.
- `Sequence`, `Shot`, `ShotRepository` — a batch of timed events and its results.
- `DeviceMessage` / `DeviceMessageDispatcher` — device-to-device/UI notification system.
- `Logger`, `LogManager` — structured device logging.
- `TaskManager`, `PersistenceManager`, `ProfileManager` — supporting subsystems.

Lightweight forward-declaration headers live in `include/sti/fwd/*_fwd.h`
(public) and `src/device/src/fwd/*_fwd.h` (internal) to keep compile-time
coupling low — prefer including the `_fwd.h` header over the full header when
only a declaration is needed.

### stinetwork (`src/network/src`, public header `include/sti/NetworkDeviceHub.h`)

Distributes the device model over CORBA so devices on different hosts can
discover and call each other:
- `NetworkDeviceHub` — main public entry point; owns a `LocalDeviceHub`, the
  omniORB ORB (via `ORBManager`), and NameService registration/connection.
- `LocalDeviceHub` / `RemoteDeviceHub` — local vs. remote sides of a
  `Hub<DeviceID, Device>`; hubs federate and discover each other via NameService,
  exchanging device references (`TNodeWalker` topology).
- `RemoteDevice` — client-side proxy implementing `Device` by wrapping a
  `TNetwork::TDevice` CORBA reference; `RemoteChannel`, `RemoteAttributeManager`,
  `RemoteEventEngineScheduler` mirror the local managers over the wire.
- `T*_i` classes (e.g. `TDevice_i`) — POA servant implementations exporting a
  local `Device`/manager as a CORBA object.
- `src/network/src/convert/Convert_*.cpp` — bidirectional conversion between
  stidevice types and IDL-generated types (e.g. `TMixedValue`, `TDeviceID`).

IDL sources (`src/network/idl/{orbTypes,deviceNet,logsNet,tasks}.idl`) define
the CORBA interfaces; `omniidl` codegen output lands in
`src/network/src/generated/` (regenerate via `compileIDL.sh`, per `AGENTS.md`
— never hand-edit the generated files).

`stinetwork` builds as C++17 (lower than stidevice's C++20) for omniORB ABI
compatibility — don't bump its standard without checking that constraint.

### src/server (`STIServer` executable)

`main.cpp` parses `-f` (config file) / `-n` (NameService address) args,
constructs a `NetworkDeviceHub`, instantiates `ServerDevice` (a `LocalDevice`
subclass implementing `parseEvents()`), attaches a `LegacyShotRepository`
(XML-based shot persistence via embedded tinyxml2), and calls `hub->run(true)`
to serve CORBA requests. This is the reference example of wiring a concrete
device into the network layer.

### stipy (`src/stipy`) — Python bindings

- `stidevicepy/src/` builds `sticommonpy` (internal static lib of shared pybind11
  glue, links `stidevice`), and the `stipybase` / `stidevicepy` extension
  modules (link `stidevice` + `stinetwork` + `sticommonpy`).
- `src/` builds the top-level `stipy` extension module (same link set).
- Each compiled module is paired with a pure-Python convenience layer that
  wraps the C++ API: `python/` (`makeshot.py`, `group.py`, `server.py`, …),
  `stidevicepy/python/` (`localdevice.py`, `hub.py`, `logs.py`, …),
  `stipybase/python/` (`mixedvalue.py`, `rawevent.py`, `sequence.py`, …).
  `__init__.py` re-exports the C++ extension modules under the `stipy`
  namespace. When changing binding behavior, check whether the pure-Python
  wrapper layer also needs updating (e.g. `localdevice.py` wraps event-parsing
  error handling around the C++ call).
- All three extension modules install under
  `${CMAKE_INSTALL_PYTHONDIR}/stipy/...`; set `CMAKE_INSTALL_PYTHONDIR` when
  configuring a local build, while conda-build supplies its staging
  site-packages directory through the recipe.

### stijava (`src/stijava`) — Java bindings

`src/swig/*.i` define SWIG interfaces; `src/` contains the generated glue
(`sti_wrap.cpp`) plus hand-written `J*` wrapper classes, built into
`libstijava.so` (links `stidevice` + `stinetwork`). `stilib/` is a separate
Gradle project that bundles the native libraries into a JAR for Java
consumers.

## Test layout

- `test/device/`, `test/network/` — Catch2 unit tests, one `*_tests.cpp` per
  production `.cpp`/module (see `test/AGENTS.md` for naming/tagging rules and
  the shared-dummy-class convention).
- `test/integration/python/` — pytest-based, multi-process/network scenario
  tests, opt-in and separate from the CTest flow (see Test notes above).
- `test/integration/cpp/` — placeholder for future lower-level
  scheduler/network integration targets (not yet implemented).
- `src/stipy/test/`, `src/stipy/stidevicepy/test/` — exploratory/manual Python
  scripts and notebooks for the bindings layer (not part of CI-style runs).
