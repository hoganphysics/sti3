# Robust play after device loss: investigation and fix plan

## Implementation status

Completed in investigation:

- Traced the parse-to-play path through `LocalEventEngineScheduler`, `EventEngineManager`, `LocalEventEngine`, `MasterTrigger`, and the remote event-engine wrappers.
- Identified the play path waits that can block indefinitely when an owned device disappears or stops responding.
- Confirmed existing targeted tests still pass:
  - `ctest --test-dir build-ninja --output-on-failure -R "Missing partner|stopEngine"`

Completed in first implementation pass:

- Added a regression for an owned target that successfully parses, then loses its parsed engine before the server attempts play.
- Added pre-play validation in `LocalEventEngine` for directly owned targets captured during parse.
- Play now cancels before scheduling child play jobs if an owned target engine is missing or not in `EngineState::Parsed`.
- Fixed the local parsed-state guard to reject play when either the state is not `Parsed` or the parse ID differs.
- Added the play message name `Owned device state invalid`.
- Verified:
  - `ctest --test-dir build-ninja --output-on-failure -R "Server cancels play when owned target loses parsed engine before play|Missing partner|Server-owned partner-generated target resolves|Connected partner-generated target resolves|stopEngine"`
  - `ctest --test-dir build-ninja --output-on-failure`

Completed in second implementation pass:

- Added bounded waits for:
  - owned devices reaching `PlayReady`;
  - trigger arming through `MasterTrigger`;
  - owned devices sending `PlayComplete`.
- Added play errors for timeout paths:
  - `Owned device PlayReady timeout`;
  - `Owned device trigger timeout`;
  - `Owned device PlayComplete timeout`.
- `PlayComplete` waiting now uses the expected end time of owned target event groups plus a conservative grace interval.
- Added regressions for:
  - the bounded `MasterTrigger` wait API reporting pending devices;
  - an owned target accepting a play job but never reporting `PlayReady`;
  - an owned target entering play but never reporting `PlayComplete`.
- Verified:
  - `ctest --test-dir build-ninja --output-on-failure -R "MasterTrigger bounded arm wait|Server cancels play when owned target|Missing partner|Server-owned partner-generated target resolves|Connected partner-generated target resolves|stopEngine"`

Completed in third implementation pass:

- Made playback timeout values configurable in the existing `EngineManager` config section:
  - `PlayReady Timeout ms`;
  - `Trigger Timeout ms`;
  - `PlayComplete Grace ms`.
- `LocalEventEngineScheduler` now owns the configured values and applies them to every `LocalEventEngine` it creates, including engines recreated through `setEngineFactory()`.
- Added an integration regression for a target that reports `PlayReady` but never arms for trigger.
- Updated timeout regressions to use short configured values and assert they cancel quickly.
- Verified:
  - `ctest --test-dir build-ninja --output-on-failure -R "Server cancels play when owned target never reports PlayReady|Server cancels play when owned target never reports PlayComplete|Server cancels play when owned target reports PlayReady but never arms|MasterTrigger bounded arm wait"`

Not started:

- Per-device completion grace tuning based on real hardware behavior.

## Problem

The server builds a dependency tree during parse and can play a successfully parsed shot multiple times. This works while the device network remains stable, but a device in the parsed dependency tree can disappear after parse and before play. The current play path can then wait forever for messages from that missing device.

The server should reject or cancel play when a required device is missing, unresponsive, in `Error`, or not in the expected state. It should also recover if a device disappears after play has already started.

## Current code path notes

- `LocalEventEngineScheduler::play()` creates a play job from a completed parse job but does not validate the parsed dependency tree before enqueueing the play job.
- `LocalEventEngineScheduler::assignPlayJobs()` checks that a local engine is parsed for the requested `ParseID`, but it does not validate downstream devices.
- `LocalEventEngine::play()` calls `scheduleAllPlayJobs()` for `ownedTargets`, then waits for all owned devices to send `PlayReady`.
- `scheduleAllPlayJobs()` records `"Failed to contact owned device"` if it cannot get a device scheduler at that moment, but if a remote call is accepted and the child later disappears, the parent waits indefinitely.
- `LocalEventEngine::waitForPlayAll()` waits for all owned devices to send `PlayComplete`.
- `MasterTrigger::waitForArm()` waits for all armed devices to report trigger-ready status.
- `RemoteEventEngine` and `RemoteEventEngineScheduler` methods currently swallow CORBA exceptions and often return no success/failure signal to the caller.

## Failure points to harden

The following waits should become bounded and recoverable:

1. Waiting for owned devices to reach `PlayReady`.
2. Waiting for owned devices to arm before trigger.
3. Waiting for owned devices to send `PlayComplete`.
4. Waiting for local play completion when a child error should abort the whole shot.

Avoid remote calls while holding `playMutex` or the trigger mutex. Timeout loops should snapshot pending device IDs, release local locks, probe state, then reacquire locks to record errors and stop/cancel.

## Phase 1: add pre-play dependency validation

Before scheduling child play jobs, validate every directly owned device needed by the parsed shot.

Implementation shape:

1. Add a private helper on `LocalEventEngine`, for example `validateOwnedTargetsReadyForPlay()`.
2. For every `ownedTarget`, prefer the parsed `engines` reference captured during parse.
3. Query each child engine state through the `EventEngine` abstraction.
4. Require `EngineState::Parsed` and the correct parse ID where that is available through the existing abstraction.
5. Treat `Missing`, `Unknown`, `Error`, null engine references, and any non-`Parsed` state as play errors.
6. Add play messages listing each unavailable device and its observed state.
7. Cancel the play job before triggering if validation fails.

Keep this logic in `stidevice` and use existing abstract interfaces. Do not add CORBA or network headers to device-library code.

## Phase 2: bound the `PlayReady` wait

Replace the indefinite wait for `playReadyOwnedTargets.size() == ownedTargets.size()` with a bounded wait loop.

Desired behavior:

- Wait in short intervals so normal play remains event-driven.
- When a target is late, probe only the missing targets.
- If a target reports `Missing`, `Unknown`, `Error`, or cannot be queried, record a play error and call `stop()`.
- If a target is still alive but slow, continue until a configurable total timeout or a conservative default is reached.

Possible first default:

- Use a small fixed readiness timeout, for example 2 seconds, for the first implementation.
- Later move the timeout into `Configuration` if the fixed value proves too aggressive for real devices.

## Phase 3: bound trigger arming

`MasterTrigger::waitForArm()` currently has no timeout. Add a bounded wait API that returns success/failure and exposes pending devices.

Implementation options:

1. Add `bool waitForArmFor(std::chrono::milliseconds timeout, std::vector<DeviceID>& pending)`.
2. Keep the existing `waitForArm()` for callers that still require indefinite behavior.
3. Use the bounded form from `LocalEventEngine::play()` before the owner triggers.
4. On timeout, record a play error naming the pending devices and stop the shot.

## Phase 4: bound `PlayComplete` wait

`LocalEventEngine::waitForPlayAll()` should not wait forever for missing `PlayComplete` messages.

Implementation shape:

1. Determine the expected completion time for the local subtree.
2. For the first pass, use a conservative timeout after entering `Playing`.
3. For a better follow-up, calculate expected child completion using the event list for each device plus a configurable grace interval.
4. When a child is late, query its engine state.
5. If the child is `Parsed`, it may have completed but the message was lost; record a warning or error depending on result availability.
6. If the child is `Playing` past the grace interval, `Error`, `Missing`, or `Unknown`, record an error and cancel the shot.

## Phase 5: make remote failure visible

The remote wrappers should expose failed calls where the local engine needs to react immediately.

Possible narrow changes:

- Add `bool`-returning helper methods internally where needed, without changing public abstract APIs unless necessary.
- Mark remote references disabled after repeated CORBA failures or failed `ping()`.
- Prefer querying `getState()` through the existing `EventEngine` interface before play and during anomaly handling.

Avoid broad API churn until the local timeout and validation behavior is covered by tests.

## Phase 6: regression tests

Add focused Catch2 tests under `test/device/`.

Suggested cases:

1. Parse a multi-device shot, remove an owned target from the device collection, then play. The play job should cancel quickly with a missing-device play error.
2. Use a test target that accepts a play job but never sends `PlayReady`. The server should cancel after the readiness timeout.
3. Use a test target that sends `PlayReady` but never sends `PlayComplete`. The server should cancel after the completion timeout.
4. Verify a normal multi-device shot still completes successfully.
5. Verify play remains repeatable for a parsed shot when all devices stay present.

## Message names

Add stable play message IDs for new failure modes:

- `Owned device unavailable`
- `Owned device PlayReady timeout`
- `Owned device trigger timeout`
- `Owned device PlayComplete timeout`
- `Owned device state invalid`

Keep messages concise and include the device ID, expected state, observed state, and shot ID.

## Verification

Targeted tests:

```bash
ctest --test-dir build-ninja --output-on-failure -R "Missing partner|stopEngine|robust play|device loss"
```

Full validation:

```bash
ctest --test-dir build-ninja --output-on-failure
```

If IDL or network interfaces change, regenerate CORBA stubs with:

```bash
src/network/compileIDL.sh
```

## Open questions

- What timeout values are appropriate for real hardware and slow network conditions?
- Should timeout configuration live in `LocalDevice` configuration, `ShotConfig`, or event-engine configuration?
- Should a child that is observed in `Parsed` state but never sent `PlayComplete` be treated as a warning or an error?
- Can expected completion time be computed per direct owned child from the already divided event groups without adding new persistent state?
