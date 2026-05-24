# Network integration test suite: goals and approach

## Goal

Create a new integration/system test area for simulated STI device networks. These tests should exercise full-network behavior that is too broad for the current Catch2 unit tests under `test/device/` and `test/network/`.

The suite should support two modes:

- Automated assertions for parse/play correctness, scheduler behavior, error handling, and result collection.
- Observable runs where the simulated network stays alive long enough for the frontend to connect and show scheduler, engine, log, and device-collection behavior.

This should live under `test/`, not `examples/`, because the primary purpose is validation, failure injection, and regression coverage. Examples can later reuse polished scenarios from the harness, but the harness itself should be test-focused.

## Recommended directory name

Use `test/integration/` as the top-level directory.

Suggested eventual layout:

```text
test/integration/
  README.md
  python/
    pytest.ini
    conftest.py
    sti_testnet/
      __init__.py
      devices.py
      topology.py
      shots.py
      waits.py
    test_delegated_trigger.py
    test_scheduler_contention.py
    test_overlap_parse_play.py
    test_server_hierarchy.py
    test_stress.py
  cpp/
    CMakeLists.txt
    ...
```

Rationale:

- `test/device/` and `test/network/` already read as focused C++ unit-test areas.
- `test/integration/` gives these tests a distinct identity and leaves room for both Python and C++ implementations.
- Keeping `python/` and `cpp/` below the same integration root makes mixed-language scenarios easier to document and avoids implying that all network tests belong under the existing `test/network/` unit-test path.

## Recommended first approach

Start with Python integration tests built around programmatically generated `stidevicepy.LocalDevice` subclasses.

Python is the better first implementation path because:

- It is faster to generate many devices, channels, partner links, parse delays, play delays, and failure behaviors.
- It can directly exercise the public `stipy` and `stidevicepy` APIs that users rely on.
- It is a natural fit for delegated trigger and future server-hierarchy tests, since those features are exposed through the Python timing-file workflow.
- It makes manual/observable frontend scenarios easier to launch without rebuilding C++ test binaries for each topology change.

Use `pytest` for the Python harness.

Reasons to prefer `pytest`:

- fixtures for name-service setup, temporary persistence/log directories, device-network lifetimes, and cleanup;
- parametrization for scale tests and topology variants;
- markers for `integration`, `stress`, `slow`, and `observe`;
- simple command-line selection for interactive frontend runs.

`unittest` would avoid a new test dependency, but it would make topology fixtures and parametrized stress cases more awkward. Catch2 should remain the default for focused C++ tests.

## Test harness shape

Build a small reusable Python harness before adding individual scenarios.

Core pieces:

- A simulated device factory that can create `LocalDevice` subclasses with configurable channels, parse behavior, play behavior, measurement behavior, and error injection.
- A topology builder that can create servers, sub-servers, shared devices, trigger devices, and partner-event relationships.
- Shot builders for common timing patterns: single-device shots, shared-resource shots, delegated-trigger shots, overlapping parse/play shots, and variable-override shots.
- Wait/assert helpers that poll scheduler state, parse tickets, result tickets, engine messages, and shot results with bounded timeouts.
- A name-service strategy that can either start an isolated test name service on a temporary port or attach to an externally managed name service.
- An observe mode, for example a pytest option such as `--observe`, that leaves the network running or pauses at a known point so the frontend can connect.

Prefer a staged realism model:

1. Single Python process with a `NetworkDeviceHub` and generated devices for fast smoke tests.
2. Multiple Python processes/hubs for most network realism tests, especially tests that need actual process isolation, device loss, restart, or frontend observation.
3. C++ integration binaries only where Python cannot represent the timing, lifetime, or network failure mode cleanly.

## Detailed plan for the first Python harness pieces

The first implementation pass should create harness infrastructure, not scenario-specific tests. The goal is to make later test cases short enough that they read as topology plus assertions.

### 1. Pytest entry point and selection

Add `test/integration/python/pytest.ini` with markers for `integration`, `slow`, `stress`, `observe`, and `requires_nameservice`.

Add `test/integration/python/conftest.py` with command-line options:

- `--sti-nameservice`: attach to an existing omniORB name service, for example `127.0.0.1:2809`.
- `--sti-nameservice-mode`: choose how the harness obtains a name service: `auto`, `external`, or `spawn`.
- `--observe`: enable frontend-observation mode for tests marked `observe`.
- `--observe-timeout`: maximum time to keep an observable topology alive before pytest exits.
- `--keep-network-alive`: leave spawned test processes alive for manual inspection after setup.

The first pass should only register options and fixtures. It should not auto-start a name service until the process-management behavior is understood and cleanup is reliable.

Name-service policy:

- Observe mode should attach to an externally managed name service by default, usually the developer's normal instance on port `2809`, so the frontend can use known connection settings.
- Automated non-observe tests should eventually default to spawning a dedicated name service on an unused non-default port. This avoids interfering with the developer's hand-test name service on `2809`.
- `auto` should mean: use `external` when `--sti-nameservice` is supplied; otherwise use `spawn` for automated tests. Observe tests should require explicit external connection details unless a test intentionally documents a spawned connection string.

### 2. Test-network package

Create a small package at `test/integration/python/sti_testnet/`.

Initial modules:

- `devices.py`: simulated `LocalDevice` subclasses and event classes.
- `topology.py`: topology specs and lifecycle handles for hubs, devices, servers, and clients.
- `shots.py`: reusable STIPy timing functions and shot builders.
- `waits.py`: bounded polling helpers for tickets, scheduler state, and frontend-observable state changes.

Keep these modules independent of individual test files. Scenario files should describe a topology, build a shot, parse/play it, and assert outcomes using shared helpers.

### 3. Device model

Start with one configurable Python device class, for example `SimulatedDevice`, backed by a simple spec object.

The spec should cover:

- device identity and target server;
- output/input channel count and names;
- parse delay and parse failure injection;
- load delay and load failure injection;
- play delay and play failure injection;
- measurement value or generated file measurement behavior;
- partner-event targets;
- trigger-capable behavior, once delegated-trigger assertions are added.

Use one corresponding `SynchronousEvent` subclass for normal output events and a second one for measurement/file events. Add more event types only when a scenario needs them.

### 4. Topology lifecycle

Add a topology builder that can run in two phases:

1. In-process topology: one Python process, one `NetworkDeviceHub`, generated devices, fastest cleanup.
2. Process topology: child Python processes for devices/hubs, needed for crash/restart, stale object references, and frontend-observation scenarios.

The first implementation should complete the in-process path and define the process-topology interface without fully implementing it. Most later integration scenarios should use one generated device per process for realism. In-process topologies remain useful for fast smoke tests and for stress comparisons between shared-hub and one-process-per-device layouts.

Every topology handle should expose:

- device IDs and server IDs;
- name-service address;
- frontend connection details: name-service IP, name-service port, server device name, server address, server module, and full server `DeviceID`;
- references to local objects when running in-process;
- `start()`, `shutdown()`, and context-manager cleanup;
- a textual network summary suitable for debugging and frontend connection instructions.

### 5. Wait and assertion helpers

All waits must be bounded. Avoid tests that rely on unbounded `ticket.wait()` calls unless the harness wraps them with an external timeout.

Initial helpers:

- wait for a parse ticket to complete, fail, or time out;
- wait for a result ticket to complete, fail, or time out;
- poll a scheduler until a job reaches an expected state;
- poll a device collection until expected devices appear;
- collect diagnostic state on timeout before raising.

Timeout errors should include the name-service address, known devices, known jobs, and the last observed state.

### 6. Shot builders

Add reusable timing-function builders rather than writing ad hoc nested functions in each test.

Initial builders:

- single-device output shot;
- multi-device shot;
- delegated-trigger shot;
- shared-resource contention shot;
- long-play shot for overlap testing;
- generated stress shot with configurable device count, event count, and time spacing.

The builders should return plain Python callables that can be passed to `server.makeshot(...)`.

### 7. First automated smoke test

After the harness pieces exist, add one small smoke test:

1. Build one server and one simulated device in-process.
2. Generate a single output event.
3. Parse and play through `stipy`.
4. Assert parse completed, play completed, and the simulated event recorded load/play calls.

This smoke test should be tagged `integration` and should become the harness health check before adding delegated-trigger or scheduler-contention scenarios.

### 8. First observe-mode scenario

Add one opt-in `observe` test only after cleanup and smoke-test behavior are reliable.

The first observable topology should:

- start one server and several generated devices;
- print the name-service IP/port, server `DeviceID`, and network summary;
- keep the network alive while the frontend connects;
- optionally run a slow parse/play loop so engine and scheduler state changes are visible.

This should never run in normal automated test selection.

For now, the frontend is connected manually through its settings dialog. The observe-mode output should therefore use stable, readable server IDs and print exactly the connection values a user needs to enter.

## Initial test list

### Delegated triggers

- Verify that `stipy.set_trigger(...)` delegates the global network trigger to the requested device.
- Verify that the delegated trigger device appears in parse metadata and scheduler state as expected.
- Verify that play waits for the delegated trigger device to arm before triggering the network.
- Verify failure behavior when the delegated trigger device is missing, disconnected, not parsed, or enters `Error`.
- Verify duplicate global trigger declarations fail with a useful parse error.

### Multiple servers and shared resources

- Run two servers that both parse/play shots requiring the same shared device.
- Verify the scheduler takes turns and does not allow conflicting simultaneous ownership of the shared resource.
- Verify no starvation when one server submits repeated jobs while another server waits.
- Verify cancellation of one server's job releases the shared device for the other server.
- Verify frontend state updates make the shared-device contention visible and coherent.

### Overlapping parse and play

- Start playing shot N while parsing shot N+1 using the engine system.
- Verify the playing shot continues without state corruption.
- Verify parse result N+1 can be played after shot N finishes.
- Verify overlapping work reports correct engine/job state transitions to connected clients.
- Add variants with slow parsing, long playback, partner-generated events, and measurement/file results.

### Server hierarchy

- Create two sub-servers, each with its own devices, plus a top-level server that coordinates both sub-servers.
- Parse the same timing function for two sub-servers with different overridden variables.
- Verify the top-level parse result preserves the two sub-server dependency trees.
- Verify top-level play coordinates both sub-server plays and aggregates results.
- Verify failures in one sub-server are reported without corrupting the other sub-server's result.

This area depends on planned `stipy` support for parsing the same timing file against multiple sub-servers with different variable overrides.

### System stress tests

- Generate a large number of simulated devices and verify discovery, parse, play, and shutdown remain bounded.
- Generate a single device with a large number of events and verify parse/play/result handling.
- Compare shared-hub and one-process-per-device layouts for the same generated network.
- Run rapid parse/play cycles and watch for leaked jobs, stale engine state, stale CORBA objects, or persistence/log buildup.
- Run repeated startup/shutdown cycles for hubs and devices.
- Run repeated parse failures and play cancellations to verify recovery.
- Run concurrent STIPy clients submitting work to the same server.

## Resolved design decisions

- Name-service handling should support both externally managed and spawned name services. Observe mode should normally use an existing name service with known connection details. Automated non-observe tests should eventually spawn a dedicated name service on an unused non-default port.
- Frontend observation should be manual for now. Observe-mode tests should print a well-known set of connection parameters: name-service IP, name-service port, and the server `DeviceID` fields needed by the frontend settings dialog.
- Most realistic integration scenarios should run one generated device per process. Shared-hub/in-process topologies should remain available for fast smoke tests and for stress comparisons.

## Additional useful scenarios

- Device disconnect, crash, reconnect, and replacement during parse and during play.
- Slow or stalled device parse/play behavior, including timeout classification.
- Partner-generated events across server boundaries.
- Network file-transfer and measurement-result collection across simulated remote devices.
- Attribute, channel, monitor, and log update bursts while shots are being parsed and played.
- Frontend observation of device collection churn, engine job churn, warnings, errors, and shot-result updates.
- Version or channel-schema mismatch between a timing file and a simulated device.
- Persistence cleanup after canceled, failed, and successful shots.
- Recovery after name-service restart or stale object references, if the harness can model this reliably.

## C++ role

Keep C++ integration support available under `test/integration/cpp/`, but do not start there unless a scenario needs C++-only control.

C++ integration tests are useful for:

- lower-level scheduler and network lifetime behavior that should not depend on Python bindings;
- failure modes that require precise threading or object-lifetime control;
- validating that a Python-observed issue is not caused by the binding layer.

When C++ integration tests are added, register them separately from the existing unit-test executable and label them as integration tests in CTest so they can be excluded from fast unit-test runs.

## Suggested pytest markers

Use markers to keep the suite selectable:

- `integration`: normal automated integration tests.
- `slow`: longer but still automated tests.
- `stress`: scale and repeated-cycle tests.
- `observe`: tests intended for frontend/manual observation.
- `requires_nameservice`: tests that need a running or spawned omniORB name service.

Example future commands:

```bash
pytest test/integration/python -m "integration and not slow and not observe"
pytest test/integration/python -m stress
pytest test/integration/python -m observe --observe
```

## Open questions

- Which tests should run in CI or regular developer validation, and which should remain opt-in because they are slow or interactive?
- What minimum `stipy` API changes are required before the server-hierarchy tests can be implemented cleanly?
