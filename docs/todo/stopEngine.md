# stopEngine during play: investigation and fix plan

## Implementation status

Completed in this pass:

- Added a focused Catch2 regression for `stopEngine()` during active play.
- Changed `LocalEventEngineScheduler::stopEngine()` so stopping a running engine cancels the associated scheduler job instead of only stopping the engine state machine.
- Added `LocalEventEngineScheduler::jobCanceled()` for jobs that finish by cancellation inside their own engine thread, avoiding a recursive external abort path.
- Kept `EventEngineManager::running` true until the job thread actually exits, so the scheduler does not reuse the engine while the previous play thread is still unwinding.
- Added a durable `ShotResultStatus` field to `ShotResult` with string conversion, cereal serialization, Python exposure, and legacy XML read/write.
- Added `ShotResultStatus` to CORBA IDL and network conversion so `ShotResult` status is transmitted over CORBA.
- Added `Play canceled` messages to stopped play results and classify those results as `CanceledByUser`.
- Preserved `ShotResultStatus` through `LocalPersistenceManager` result collection.
- Added repository tests for status round-trip and old cereal shot files that do not contain a status field.

Verified:

- `ctest --test-dir build-ninja --output-on-failure -R "stopEngine|Missing partner|Declared missing partner|ShotResult"`
- `ctest --test-dir build-ninja --output-on-failure -R stopEngine --repeat until-fail:10`
- `ctest --test-dir build-ninja --output-on-failure`

Still deferred:

- Timeout-based detection of crashed, disconnected, or late devices during play.

## Problem

A server device can remain stuck in `Play` when an owned target device disappears during a shot. Calling `EventEngineScheduler::cancelJob()` on the active play job appears to recover the scheduler, but calling `EventEngineScheduler::stopEngine(engineID)` can leave the scheduler unable to run later parse/play jobs.

This pass should focus on two things:

1. Reproduce and fix the existing `stopEngine()` cancellation bug.
2. Add a durable play outcome to `ShotResult`, with cancellation/error information preserved in `ShotResult::messages`.

Timeout-based detection of dead or late devices is a separate follow-up and is intentionally out of scope until the explicit stop/cancel path is correct.

## Current code path notes

- `LocalEventEngineScheduler::stopEngine()` currently only finds the engine manager and calls `LocalEventEngine::stop()`.
- `LocalEventEngineScheduler::cancelJob()` goes through `_cancelJob()`, calls `EventEngineManager::abortJob()` for running jobs, moves the job out of `runningJobs`, marks it `Canceled`, stores it in the completed parse/play buffer, sends a job update, and notifies `jobCondition`.
- `EventEngineManager::runJob()` calls `engine->play(*currentJob)`. After that returns, it checks `engine->jobCancelled()` and then calls either `scheduler->cancelJob(currentJob->getJobID())` or `scheduler->jobComplete(currentJob->getJobID())`.
- `LocalEventEngine::play()` can wait on `playCondition` for owned devices to report `PlayReady` and later `PlayComplete`. `LocalEventEngine::stop()` sets cancellation state, changes play states back toward `Parsed` or `Error`, notifies play/trigger waiters, stops local events, and calls `stopOwnedDevices()`.
- A likely failure mode is that `stopEngine()` stops the engine but does not directly update the scheduler job lists. If the play thread does not return cleanly, the play job remains in `runningJobs`, which blocks later scheduler work.
- Also check for a possible thread-join deadlock in `EventEngineManager::submitJob()`: it joins the previous `jobThread` while holding `jobMutex`, while `runJob()` also takes `jobMutex` at the end.

## Phase 1: make a focused failing test

Add a Catch2 test for stopping an active play job through `EventEngineScheduler::stopEngine()`.

Preferred location:

- Add `test/device/eventengine_stop_engine_tests.cpp`.
- Register it in `test/CMakeLists.txt`.

Test shape:

1. Create a local server device with `EngineManager` engine count set to 1.
2. Use a simple test `LocalDevice` whose `parseEvents()` creates a `SynchronousEvent` at a delayed time, for example 1.0 seconds, so the engine reliably enters `Playing` before the shot completes.
3. Parse the shot and require the parse job to reach `EngineJobStatus::Completed`.
4. Start play and wait until:
   - `scheduler.getStatus(sid) == EngineJobStatus::Running`
   - `scheduler.getEngineState(engineID) == EngineState::Playing`
5. Call `scheduler.stopEngine(engineID)`.
6. Assert that the play job reaches `EngineJobStatus::Canceled` within a short bounded wait.
7. Assert that `EventEngineJobList::Running` no longer contains the play job.
8. Submit a second parse/play job and require it to complete or cancel according to its own shot behavior, not because the old play job is still blocking the scheduler.

The first test can be single-device. If that does not reproduce the bug, extend it to the more realistic two-device topology:

1. Create `server` and `testDevice`, with `testDevice` targeting `server`.
2. Parse a shot whose events are on `testDevice`.
3. Start play and wait until the server is waiting for owned-device play completion.
4. Simulate the target device disappearing as closely as the local test harness allows. Candidate approaches to investigate:
   - remove or disconnect `testDevice` from the local `DeviceCollection`;
   - stop or destroy `testDevice` after parse but before play completes;
   - use a test event on `testDevice` that never sends `PlayComplete` unless stopped.
5. Call `serverScheduler.stopEngine(serverEngineID)`.
6. Require the same recovery assertions as the single-device test.

If the target-device crash cannot be represented cleanly in a pure device-library test, keep the local stop test as the unit regression and add a note for a later network/integration test.

## Phase 2: identify where the scheduler gets stuck

Use the failing test to inspect these points before changing behavior:

- Does `LocalEventEngine::stop()` notify the wait that is actually blocking?
- Does `LocalEventEngine::play()` return after `stopEngine()`?
- Does `EventEngineManager::runJob()` reach the `engine->jobCancelled()` branch?
- Does `LocalEventEngineScheduler::_cancelJob()` run for the play job?
- Is the play job removed from `runningJobs` and inserted into `completedPlayJobs`?
- Does `EventEngineManager::jobRunning()` become false?
- Does the scheduler thread wake and call `assignJobs()` for later jobs?
- Is a later `submitJob()` blocked joining the previous `jobThread` while holding `jobMutex`?

Keep any debug instrumentation temporary unless it exposes a useful assertion or test helper.

## Phase 3: fix stopEngine job propagation

Desired behavior:

- Stopping an engine that is running a job should cancel the scheduler job associated with that engine.
- The job should leave `runningJobs`, enter the completed play/parse buffer as `EngineJobStatus::Canceled`, and notify the scheduler.
- The engine should be stopped exactly once from the caller's perspective; repeated stop/cancel calls should be harmless.
- `EventEngineManager::runJob()` should tolerate the job already being canceled by `stopEngine()` and should not later re-complete it.

Implementation options to evaluate after the red test exists:

1. Make `LocalEventEngineScheduler::stopEngine(engineID)` find the manager's current job and call the same cancellation path used by `cancelJob(jobID)`.
2. Add a narrow manager API that cancels the current job and returns its `EngineJobID`, so the scheduler can update job lists without exposing manager internals.
3. Make `_cancelJob()` idempotent for jobs that have already been removed from `runningJobs`.
4. Fix any confirmed `EventEngineManager::submitJob()` join deadlock by moving the join outside the locked region or restoring the earlier move-and-join pattern safely.

Avoid adding network-specific dependencies to `stidevice`. The fix should stay in the local scheduler/engine manager abstractions unless the failing test proves otherwise.

## Phase 4: persist play outcome in ShotResult

Add an explicit shot outcome field separate from `ShotResultRecord`.

Proposed enum:

```cpp
enum class ShotResultStatus {
    Unknown,
    Success,
    CompletedWithErrors,
    CanceledByUser,
    AbortedByError,
    AbortedByTimeout
};
```

Initial integration points to inspect and update:

- `include/sti/engine/ShotResult.h`: add the field and include/define the enum.
- `src/device/src/ShotResult.cpp`: initialize and serialize the field.
- `src/server/src/LegacyExperimentXMLBuilder.cpp`: write the outcome to legacy shot XML.
- `src/server/src/LegacyExperimentXMLReader.cpp`: read the outcome, defaulting old files to `Unknown`.
- `src/network/src/convert/Convert_ShotResult.*`: include the outcome in network conversion if `ShotResult` crosses CORBA boundaries.
- `src/stipy/stidevicepy/src/ShotResult_wrap.cpp`: expose the outcome to Python.
- Java/SWIG bindings can be assessed separately if generated files are not meant to be edited by hand.

Outcome assignment rules:

- Normal play completion with no play errors: `Success`.
- Play completes but messages contain errors: `CompletedWithErrors` or `AbortedByError`, depending on whether the engine finished the shot or stopped early.
- `cancelJob()` or `stopEngine()` cancellation: `CanceledByUser` for explicit user/API cancellation.
- Future timeout cancellation: `AbortedByTimeout`.
- Unexpected missing result or unknown cancellation reason: `AbortedByError` or `Unknown`, with a message explaining the condition.

Whenever a play job is canceled or stopped, add an `EnginePlayingMessage` to the `ShotResult::messages` list with:

- message type `Error`;
- a stable message name such as `Play canceled`;
- the source device ID;
- the `sid`, `pid`, and cancellation reason where available.

## Phase 5: tests for ShotResult outcome

After the scheduler cancellation bug is fixed, add tests that verify:

- `stopEngine()` during play produces a retrievable `ShotResult`.
- The shot result contains the cancellation/error message.
- The new outcome field is `CanceledByUser`.
- A normal completed shot is `Success`.
- Serialization round trips the new outcome through the local/cereal repository.
- Legacy XML read/write round trips the new outcome, with old/missing outcome fields loading as `Unknown`.

## Verification

Run focused tests first:

```sh
cd build-ninja && conda run --no-capture-output -n sti3-build cmake --build . --parallel 8
ctest --test-dir build-ninja --output-on-failure -R eventengine
```

Then run the full test suite before considering the work complete:

```sh
ctest --test-dir build-ninja --output-on-failure
```

## Later timeout work

Do not start this until the stop/cancel path and persisted shot outcome are reliable.

Follow-up questions for timeout design:

- Which layer should own timeout policy: scheduler, local engine, or server-side play coordination?
- Should the timeout be based on expected final event time plus configurable buffer?
- Should late but still responsive devices be allowed to continue?
- Should failed pings automatically abort the shot?
- Should timeout checks use periodic condition-variable wakeups or a separate monitor thread?
