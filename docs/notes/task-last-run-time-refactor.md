# Task Last Run Time Refactor Plan

Date: 2026-06-23

## Goal

Track the most recent successful run time for each task and align that state
with task update messages.

The new task run state should be visible through:

- local C++ `Task` objects
- task update device messages
- network task snapshots and network task update messages
- the `stidevicepy` task and message APIs

The last run time should only live for the lifetime of the running process. It
should not be written into `LocalTaskManager` persistence.

## Current State

`Task` currently stores task identity, status, and metadata. It has no base
class run-time state.

`TaskScheduler::run()` is the central scheduler path for task execution. It now
creates a local-clock timestamp before calling `task->run()` and emits a run
event after `run()` returns.

`LocalTaskManager` listens for scheduler run events and sends a
`TaskUpdateMessage` containing the task ID and timestamp string.

`TimeStamp()` means "current local time". A default-constructed `TimeStamp`
cannot represent an unset last-run value.

Some concrete task types already have their own scheduling state. For example,
`IntervalTask` has an internal `std::chrono::system_clock::time_point` used to
compute the next run interval. That scheduling state is separate from the new
public "last successful run" state.

## Design Decisions

### Store unset with `std::optional<TimeStamp>`

Do not modify `TimeStamp` to support an invalid or null state.

Use optional state in `Task`:

```cpp
std::optional<STI::Utils::TimeStamp> lastRunTime;
```

Expose it as:

```cpp
virtual bool hasLastRunTime() const;
virtual std::optional<STI::Utils::TimeStamp> getLastRunTime() const;
```

These accessors should be virtual but not pure virtual. The base `Task` should
own the normal storage and implementation. Proxy and wrapper task types can
override if they need to delegate to an underlying object or remote snapshot.

### `runNow()` is the public execution API

Move C++ task execution through a public, non-virtual wrapper:

```cpp
STI::Utils::TimeStamp runNow();
```

The existing virtual `run()` should become the private C++ implementation hook:

```cpp
private:
    virtual void run() = 0;
```

C++ subclasses can still override a private pure virtual method. The override
does not need to be public.

`Task::runNow()` should:

1. Create one `STI::Utils::TimeStamp` using the local clock.
2. Call the subclass `run()` hook.
3. Store the timestamp as `lastRunTime` after `run()` returns successfully.
4. Return the same timestamp to the caller.

This makes the timestamp mean "start time of the last successful task run".
If `run()` throws, `lastRunTime` should not be updated and no run update should
be emitted by the scheduler.

Manual C++ execution should use:

```cpp
task->runNow();
```

Direct calls to `run()` should no longer be part of the C++ public API.

### Scheduler uses `runNow()`

`TaskScheduler::run()` should call:

```cpp
const auto runTime = task->runNow();
```

The returned `TimeStamp` should be used for the scheduler run event. This keeps
the `Task` state and the `TaskUpdateMessage` payload aligned exactly.

Skipped tasks should not update `lastRunTime`. Non-repeating tasks should still
retain their last run time after they are deactivated.

### Keep Python `run()` as the subclass hook

Python users should continue to implement:

```python
class MyTask(stipy.Task):
    def run(self):
        ...
```

The Python API should also expose `runNow()` as the public tracked execution
method:

```python
task.runNow()
```

`TaskManager.runTask(taskID)` should go through the scheduler path and therefore
also update `lastRunTime` and emit the run update message.

Directly calling a Python subclass's `run()` hook may still bypass tracking. The
documented manual execution path should be `runNow()`.

### Use `TimeStamp` in `TaskUpdateMessage`

Change task run messages from string timestamps to typed timestamps.

Use optional timestamp state in C++:

```cpp
std::optional<STI::Utils::TimeStamp> timestamp;
```

Status updates should not have a timestamp. Run updates should have one.

This lets message receivers choose their own formatting through the `TimeStamp`
API instead of receiving only the default `toString()` representation.

### Represent optional timestamps explicitly over CORBA

CORBA IDL does not have `std::optional`, so use presence booleans.

For `TTask`:

```idl
boolean hasLastRunTime;
TTimeStamp lastRunTime;
```

For `TTaskUpdateMessage`:

```idl
boolean hasTimestamp;
TTimeStamp timestamp;
```

When the boolean is false, consumers must ignore the timestamp field.

This keeps unset semantics unambiguous and avoids treating a default-generated
`TTimeStamp` as a real run time.

### Do not persist last run time

`LocalTaskManager` persistence should remain focused on task ID and active
status. It should not store `lastRunTime`.

Rationale:

- Last run time is operational process state, not durable task configuration.
- Persisting it would imply stale run history after a process restart.
- Avoiding persistence keeps the migration smaller and avoids compatibility
  issues with old task XML files.

## Proposed C++ API Shape

```cpp
class Task
{
public:
    STI::Utils::TimeStamp runNow();

    virtual bool hasLastRunTime() const;
    virtual std::optional<STI::Utils::TimeStamp> getLastRunTime() const;

private:
    virtual void run() = 0;
    void setLastRunTime(const STI::Utils::TimeStamp& runTime);

    std::optional<STI::Utils::TimeStamp> lastRunTime;
};
```

`setLastRunTime()` should be private. `runNow()` should be the normal mutating
path. If a narrowly scoped setter is needed for local wrapper construction or
network snapshot reconstruction, prefer a private friend relationship or a
protected snapshot constructor over making the setter public.

## Network Behavior

Task snapshots should include the optional last run time:

- local task with no successful run: `hasLastRunTime == false`
- local task after a successful run: `hasLastRunTime == true`, populated
  `lastRunTime`

Task update messages should include an optional timestamp:

- status update: `hasTimestamp == false`
- run update: `hasTimestamp == true`, populated `timestamp`

The network converters should map:

```cpp
std::optional<TimeStamp> <-> boolean has... + TTimeStamp
```

Remote task objects need a clear snapshot/proxy policy. The current remote task
object already fetches status live from the remote manager while other fields
come from the `TTask` snapshot.

`RemoteTask::getLastRunTime()` should be live when the remote task is attached
to a `RemoteTaskManager`. A caller that actively pulls this value should receive
the most recent version from the source task manager, even if it missed a task
update message. Frontend clients can still use task update messages to maintain
local cached state without polling, but pull should remain a refresh backup.

Add an explicit task-manager query rather than routing this through a full
`getTask()` snapshot refresh:

```cpp
virtual std::optional<STI::Utils::TimeStamp> getTaskLastRunTime(
    const std::string& taskID) const;
```

The network IDL can represent this as a boolean return plus an output
`TTimeStamp`:

```idl
boolean getTaskLastRunTime(in string taskID, out TTimeStamp lastRunTime);
```

For local tasks, `TaskManager::getTaskLastRunTime()` should read the current
task object. For remote tasks, `RemoteTask::getLastRunTime()` should delegate to
`RemoteTaskManager::getTaskLastRunTime()`. If a `RemoteTask` is not attached to
a manager, it can fall back to any snapshot value it was constructed with.

## STIPy Behavior

Expose:

```python
task.hasLastRunTime()
task.getLastRunTime()
task.runNow()
```

Python `getLastRunTime()` should return:

- `None` when unset
- a `TimeStamp` object when set

Expose `TaskUpdateMessage.timestamp` as:

- `None` for status updates
- a `TimeStamp` object for run updates

Keep `Task.run()` available as the Python override hook.

## Implementation Todo

- [ ] Extend `Task`.
  - Add optional last-run storage.
  - Add `hasLastRunTime()`.
  - Add `getLastRunTime()`.
  - Add public `runNow()`.
  - Move the C++ `run()` hook to private.
  - Keep last-run access protected by the existing task mutex.

- [ ] Update concrete C++ tasks.
  - Move `IntervalTask::run()` implementation under a private override.
  - Move `AppointmentTask::run()` implementation under a private override.
  - Move `RemoteTask::run()` implementation under a private override.
  - Update tests and examples to call `runNow()` for manual execution.

- [ ] Update `TaskScheduler`.
  - Replace direct `task->run()` calls with `task->runNow()`.
  - Use the returned `TimeStamp` in the run event.
  - Keep skip behavior unchanged.
  - Keep deactivation after non-repeating runs unchanged.

- [ ] Update task update messages.
  - Replace string timestamp storage with optional `TimeStamp`.
  - Update constructors for status and run updates.
  - Update string/debug formatting if needed.

- [ ] Update `LocalTaskManager`.
  - Send run `TaskUpdateMessage` using the `TimeStamp` from the scheduler event.
  - Do not add last-run time to `StoredTask`.

- [ ] Update network IDL and generated stubs.
  - Add `hasLastRunTime` and `lastRunTime` to `TTask`.
  - Add `hasTimestamp` and `timestamp` to `TTaskUpdateMessage`.
  - Add `TTaskManager::getTaskLastRunTime(in string taskID, out TTimeStamp lastRunTime)`.
  - Run `src/network/compileIDL.sh`.
  - Treat this as a wire-ABI change.

- [ ] Update network conversion.
  - Convert optional task last-run state in `Convert_Task`.
  - Convert optional task-update timestamp state in `Convert_DeviceMessage`.
  - Reuse existing `TTimeStamp` conversion helpers.

- [ ] Update task-manager pull APIs.
  - Add `TaskManager::getTaskLastRunTime(taskID)` returning optional `TimeStamp`.
  - Implement it in `LocalTaskManager` by reading the local task.
  - Implement it in `RemoteTaskManager` with the new IDL call.
  - Have `RemoteTask::hasLastRunTime()` and `RemoteTask::getLastRunTime()` pull live values through `RemoteTaskManager` when attached.
  - Fall back to snapshot state only when a `RemoteTask` is detached.

- [ ] Update `stidevicepy`.
  - Expose `Task.runNow()`.
  - Expose `Task.hasLastRunTime()`.
  - Expose `Task.getLastRunTime()` returning `None` or `TimeStamp`.
  - Keep Python `Task.run()` as the override hook.
  - Expose `TaskUpdateMessage.timestamp` as `None` or `TimeStamp`.

- [ ] Update tests.
  - New tasks start with no last run time.
  - `runNow()` updates last run time after successful execution.
  - `runNow()` does not update last run time if the task throws.
  - Scheduler run events use the same timestamp stored on the task.
  - Skipped tasks do not update last run time.
  - Non-repeating tasks remain inactive after run but retain last run time.
  - `TaskUpdateMessage` network round trips preserve timestamp presence/value.
  - `TTask` network round trips preserve last-run presence/value.
  - `RemoteTask::getLastRunTime()` pulls the latest remote task value after a run message is missed.
  - Python task getter returns `None` before run and `TimeStamp` after run.

## Open Questions

- Should `Task::runNow()` be `virtual`? The current recommendation is no. It
  should be the invariant-preserving wrapper around the private hook.
- Should Java/SWIG wrappers be updated in the same pass? The current task scope
  mentions the device library, network library, and `stipy`; Java may need a
  follow-up audit if it is still supported.
