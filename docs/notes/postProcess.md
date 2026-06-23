# Post-processing targets and hooks

Date: 2026-06-20

> **Partially superseded (3.6.0, see `postProcess-hierarchical-refactor.md`).**
> The core types here — `PostProcessTarget`, `PostProcessRequest`,
> `PostProcessingManager`/`LocalPostProcessingManager`, the
> `PostProcessingComplete` message, and the `requestPostProcessing` RPC — are
> unchanged. Three mechanisms below were replaced before release and the
> sections describing them are historical:
>
> - **Storage.** The resolved request list is stored on the job owner's
>   `LocalEventEngine` (per shot), not in a `std::map<ShotID, …>` on the
>   scheduler and not copied onto the play job.
> - **Dispatch.** Post-processing is *not* dispatched from a `PlayComplete`
>   message listener (`PostProcessingDispatcher` was removed). The job owner's
>   engine dispatches inline at the end of `play()` (after the shot is
>   persisted), routing each request **along the shot's dependency tree**
>   (`distributePostProcessing`) so targets nested behind sub-servers are
>   reached at arbitrary depth — the original owner-collection-only resolution
>   and dispatch could not reach them. Post-process targets are added to the
>   dependency tree during parse (a second, additive `getDependants` pass).
> - **Callback contract.** The worker resolves the owning device's
>   `PersistenceManager` (self, or a partner declared with `addPartner`) and
>   pulls the `ShotResult` itself, then passes it to the callback. The signature
>   is `MetaData(const std::shared_ptr<ShotResult>&, const MetaData&)`, not
>   `(const ShotID&, …)`; an unreachable owner or missing result aborts with a
>   distinct `Failed` completion message instead of running the callback.

## Goal

Add a first-class notion of asynchronous post-shot post-processing to the
device library, so that any device (typically a dedicated, lightweight
"analysis" device on the STI network) can register named post-processing
targets. A timing file can declare a post-processing request against a named
target the same way it declares a `dev()`/`ch()` event target, with a small
options payload. When a shot finishes playing, the server notifies the
relevant device(s) so they can pull the shot's measurement data and run
arbitrary user code (e.g. a Python/numpy fit) in a background worker thread,
without blocking parsing or playing of subsequent shots. When the work
finishes, the device broadcasts a completion message with the results so
other interested devices can subscribe and consume them.

This is a two-phase effort:

- **Phase 1 (this document's main focus):** core abstractions in `stidevice`
  (`src/device/src`, `include/sti/`) — no network/CORBA dependency, consistent
  with the existing rule that `stidevice` must not know about `stinetwork`.
- **Phase 2:** network-enable the new manager and message type in
  `stinetwork` (`src/network/src`), following the existing
  Manager/Message-over-CORBA template (see "Existing pieces" below). Phase 2
  is sketched here but should be implemented after Phase 1 is solid, per
  AGENTS.md's dependency-boundary rule.

## Key design decisions and why

These were worked out by discussion before any code was written; record them
here so the rationale survives:

1. **Post-processing requests never enter the hard-timed event table.**
   `EventEngineScheduler`/`LocalEventEngine` only gates the next shot's
   parse/play on devices that have scheduled events
   (`LocalEventEngine.cpp:868-891`, the `eventsByTarget` check). If a
   post-processing request were modeled as a `RawEventType::Play`/
   `Measurement` on a channel, the post-processing device would be pulled
   into `ownedTargets` and would block `Parsed`/`PlayReady` waits
   (`LocalEventEngine.cpp:1078`, `:1167-1206`) until it finished analyzing —
   exactly the deadlock we need to avoid. So post-processing declarations live
   in a **separate side-list** on `RawEventGroup`, parallel to (not part of)
   the hard-timed events list, the same way `.var()`/`.tag()` already produce
   `ParsedVar`/`ParsedTag` metadata without entering the sequence table.

2. **A missing post-processing device produces a parse-time warning, never a
   play-time error.** The existing "missing device" path
   (`LocalEventEngine.cpp:895-900`) feeds `missingTargets`, which is exactly
   what `isAbstractShot()` (`LocalEventEngine.cpp:506-508`) checks to produce
   the **hard** play-time error at `LocalEventEngine.cpp:1274-1282`
   ("Cannot Play Abstract Shot"). Post-processing targets must warn the same
   way but must never set that flag. This means a **new, separate,
   non-fatal** missing-target set, populated by a **new** resolution pass
   (since post-processing targets are never in `eventsByTarget`, the existing
   reachability check doesn't see them at all — there is no warning to
   "inherit" for free here, unlike for the engine-state-machine gating above).

3. **Post-processing request delivery is a direct RPC, not a broadcast
   message.** `DeviceMessage`/`DeviceMessageDispatcher` is a pub-sub
   mechanism (any registered listener receives it, with `addFilter<T>` used to
   narrow). A post-processing request has exactly one sender and exactly one
   intended recipient — closer to `ChannelManager::writeChannel()` than to a
   broadcast. So: **request = direct manager RPC**
   (`PostProcessingManager::requestPostProcessing(...)`).
   **Completion = broadcast `DeviceMessage`** (`PostProcessingCompleteMessage`),
   because "other devices subscribe as listeners and import results" is
   genuinely pub-sub — this is what the existing dispatcher/relayer/filter
   machinery is built for. Request and Complete have different audiences and
   different payload shapes (input options vs. output results), so they are
   two distinct types rather than one message class with a phase enum (unlike
   `EngineJobUpdateDeviceMessage`'s `targetList`, whose phases all share one
   audience: `ParseTicketManager`/`ResultTicketManager` want the whole
   lifecycle).

4. **Options/results payload type is `MetaData`, not `Configuration`.**
   `Configuration` (`include/sti/utils/Configuration.h`) is scoped to
   ini-style startup configuration (paired with `ConfigFile`, used to
   construct a `LocalDevice`) and has **no existing network conversion code**
   — adopting it here would mean adding new IDL/Convert plumbing for the type
   itself. `MetaData` (`include/sti/utils/MetaData.h`) is a key-value bag
   backed by a single `MixedValue` and is **already** threaded across the
   network boundary for `Channel`, `Attribute`, `Monitor`, `Task`, and
   `RawEventGroup` (see `Convert_Attribute.cpp`, `Convert_Channel.cpp`,
   `Convert_Monitor.cpp`, `Convert_Task.cpp`, `Convert_RawEventGroup.cpp`,
   `deviceNet.idl`), and is already pybind-wrapped and used from Python in
   several wrap files. Using `MetaData` means zero new network plumbing for
   the payload type itself, and stipy users already know how to build one.
   `MetaData`'s edits are O(n) (linear scan via `tupleMatch`/`isTuple`), which
   is irrelevant here: options/results are built once before the RPC/message
   and read once on receipt, never mutated in a loop.

5. **Target addressing reuses the device-side idiom of `RawEventTarget`, but
   is a distinct type, not a reused one.** `RawEventTarget`
   (`include/sti/engine/RawEventTarget.h`) /
   `RawEventTargetChannel` (`include/sti/engine/RawEventTargetChannel.h`)
   already provide exactly the addressing semantics we want: a
   name-or-number identifier with an `isAbstract()` flag, meaning "specified
   by name only, not yet resolved against the live network" — which is what
   lets `dev()`/`ch()` work in a standalone timing file with no live
   connection. However, reusing `RawEventTargetChannel` directly risks a
   silent name/number collision: a device could have a hard-timed channel and
   a post-processing target that happen to share a name, and `RawEventTarget`
   carries no "kind" discriminator — two instances named `"Blue MOT"` are
   bitwise identical despite meaning different things. The current design
   would technically resolve correctly today (because resolution always knows
   which side-list/manager an entry came from), but that safety is a property
   of how the code happens to be written, not of the type — it's fragile
   under future refactors, and it conflates two different things in logs,
   diagnostics, and any future generic "list all targets in this shot" tool.
   **Fix: a new, small, distinct `PostProcessTarget` type** (not reusing
   `RawEventTargetChannel`), so the type system — not convention — prevents a
   `ch()`-built channel handle from ever being passed where a post-processing
   target is expected, or vice versa. `PostProcessTarget` reuses
   `RawEventTargetDevice` for the device half (device-name collisions are not
   a concern) and defines its own small name-or-number-plus-abstract-flag
   value for the target half, mirroring `RawEventTargetChannel.h:14-39`
   almost exactly (~40 lines, same shape, separate type).

6. **`LocalPostProcessingManager` is built into `stidevice` so any device can
   opt in, with one shared worker queue per device, not per target.** Devices
   are lightweight in this system and a network commonly has many of them;
   if a target needs true parallelism, the recommended path is to give it its
   own dedicated device rather than add per-target concurrency inside one
   manager. The manager owns a single `EventQueue`-backed worker thread (see
   "Existing pieces"), not the time-driven `TaskManager`/`TaskScheduler`
   (`IntervalTask`/`AppointmentTask` assume periodic/appointment-style
   `secondsToNextRun()` scheduling and are not a good fit for one-shot,
   externally-triggered jobs).

7. **Naming convention for new stipy functions: camelCase.** The dominant
   convention in `src/stipy/python/*.py` is camelCase because most public
   functions are thin wrappers mirroring C++ method names 1:1 (`addTask`,
   `addListener`, `openLog`, `getLogManager`, `getAllVars`, ...). The
   snake_case outliers (`set_trigger`, `run_shots`, `get_local_ip_address`)
   are pure-Python helpers with no C++ counterpart to mirror. New
   post-processing functions should follow the camelCase majority and mirror
   their C++ names: `postTarget()`, `addPostProcessingTarget()`,
   `getPostProcessingTargets()`.

8. **stipy function name: `postTarget()`, not `ppTarget()` or `target()`.**
   "PP" collides with "pump-probe," a standard abbreviation in the AMO/cold-atom
   experiments this library targets — a real ambiguity risk in a timing file.
   Bare `target()` is too generic (could be read as the general event-target
   concept). `postTarget()` is unambiguous and pairs with `postProcess()`.

9. **Post-processing is dispatched off the same `PlayComplete` notification
   `ResultTicketManager` already uses, not synchronously inside
   `jobComplete()`.** The original sketch placed the dispatch inside
   `LocalEventEngineScheduler::jobComplete()`, but that function holds
   `jobMutex` for its entire body (`LocalEventEngineScheduler.cpp:1019`), and
   for a *remote* target `requestPostProcessing(...)` is a CORBA round-trip —
   calling it there would block the scheduler under its own lock (and risk
   re-entrancy/deadlock), directly contradicting the requirement that dispatch
   never delay `jobComplete()`. Instead, mirror `ResultTicketManager`: it is a
   `DeviceMessageListener<EngineSchedulerMessage>` (registered in
   `LocalDevice.cpp:250`) whose ticket is marked complete when an
   `EngineSchedulerMessage` of type `PlayComplete` arrives
   (`ResultTicketManager.h` `handleMessage(EngineSchedulerMessage)` →
   `setComplete()`). That message is dispatched asynchronously on the
   dispatcher thread — **not** the scheduler thread, **not** under `jobMutex` —
   and, crucially, it is the same signal that guarantees the shot's
   measurement data are persisted and queryable (which is exactly why a
   `ResultTicket` becomes readable at that moment). Post-processing dispatch
   therefore hangs off the **same `PlayComplete` listener path**, so it (a)
   never touches the play-critical path, and (b) is guaranteed that a
   `ResultTicket`/`ShotResult` pull for that `shotID` will succeed. See
   "Job-completion integration" below.

## Existing pieces to reuse (do not reinvent)

- **Manager abstraction + Local/Remote split template.** Abstract base
  classes live in `include/sti/device/` (`ChannelManager.h`,
  `AttributeManager.h`) with pure-virtual interfaces; `LocalChannelManager`
  (`src/device/src/`) and `RemoteChannelManager`/`RemoteAttributeManager`
  (`src/network/src/`) both derive from them. `RemoteDevice::getChannelManager()`
  (`src/network/src/RemoteDevice.h:60`) hands back the abstract pointer so
  callers don't care which implementation backs it. `PostProcessingManager`
  follows this exactly: abstract in `include/sti/device/`, `LocalPostProcessingManager`
  in `src/device/src/` now, `RemotePostProcessingManager` in `src/network/src/`
  in Phase 2.

- **`EventQueue<T>`** (`include/sti/utils/EventQueue.h`) — generic,
  event-driven FIFO queue with its own worker thread (used internally by
  `LocalDeviceMessageDispatcher` itself). This is the queue primitive for
  `LocalPostProcessingManager`'s worker thread — not `TaskManager`/`TaskScheduler`,
  which is time-driven (`secondsToNextRun()`-based polling,
  `src/device/src/TaskScheduler.cpp:265-335`).

- **`RawEventTarget` / `RawEventTargetDevice` / `RawEventTargetChannel`**
  (`include/sti/engine/`) — the existing symbolic addressing primitive behind
  `dev()`/`ch()`. Constructible from pure strings
  (`RawEventTarget(deviceName, channelName)`), with an `isAbstract()` flag
  meaning "named but not yet resolved." `PostProcessTarget` mirrors this shape
  but is a distinct type (see decision 5 above).

- **`MetaData`** (`include/sti/utils/MetaData.h`) — already network-enabled
  key-value bag (see decision 4).

- **`DeviceMessage` / `DeviceMessageDispatcher` / `DeviceMessageRelayer`**
  (`include/sti/device/`, `src/device/src/LocalDeviceMessageDispatcher.*`) —
  fully async dispatch (`addMessage()` returns immediately; listeners run on
  the dispatcher's own thread, not the engine scheduler thread). This is the
  transport for `PostProcessingCompleteMessage`. `EngineJobUpdateDeviceMessage`
  (`include/sti/device/DeviceMessage.h:405-546`) is the precedent for a
  job/shot-lifecycle message and for the `jobOwner: DeviceID` field
  convention (reused below).

- **`ResultTicket` / `ResultTicketManager`** (`include/sti/engine/ResultTicket.h`,
  `ResultTicketManager.h`) — existing lazy pull-by-`ShotID` pattern against a
  `PersistenceManager`. Retrieval is local-only by construction (it just calls
  whatever `PersistenceManager` it was given); there is no automatic
  "find the owning device and route there" logic
  (`LocalPersistenceManager.cpp:760` has this commented out). Cross-device
  fetch is possible today via `RemoteDevice::getPersistenceManager()`
  (`src/network/src/RemoteDevice.cpp:432-437`) returning a
  `RemotePersistenceManager`, which can then back a `ResultTicket` the same
  way a local one does. `ShotResult.measurements` is already
  `map<DeviceID, MeasurementVector>` — i.e. **one** owning device's
  `PersistenceManager` holds the full aggregated result for a shot, so a
  post-processing device only ever needs to know **one** `DeviceID` (the job
  owner), not the whole device topology.

- **Missing-device / abstract-shot machinery**
  (`LocalEventEngine.cpp:506-508` `isAbstractShot()`, `:895-900` warning,
  `:1274-1282` play-time error, `LocalEventEngineJob.h:88`
  `missingTargetIDs`) — the pattern to mirror for a *separate*,
  non-fatal post-processing equivalent (decision 2 above).

## New types (Phase 1, stidevice)

### `PostProcessTarget`

`include/sti/engine/PostProcessTarget.h`. Mirrors
`RawEventTargetChannel`/`RawEventTarget` in shape but is unrelated by type.

```
class PostProcessTarget
{
public:
    PostProcessTarget(const RawEventTargetDevice& device, const std::string& name);
    PostProcessTarget(const std::string& deviceName, const std::string& name);

    bool isAbstract() const;          // true until resolved against the network
    const RawEventTargetDevice& device() const;
    std::string name() const;

    bool operator<(const PostProcessTarget&) const;
    bool operator==(const PostProcessTarget&) const;

    template<class Archive> void serialize(Archive&);
private:
    RawEventTargetDevice _device;     // reuse as-is, device-name collisions are not a concern
    std::string _name;
    bool _isAbstract;
};
```

No channel-number overload is needed (post-processing targets are
name-addressed only — there's no equivalent of a fixed hardware channel
number).

### `PostProcessRequest`

`include/sti/engine/PostProcessRequest.h`. Pairs a target with its per-call
options; this is what `RawEventGroup` collects into its new side-list.

```
class PostProcessRequest
{
public:
    PostProcessRequest(const PostProcessTarget& target, const STI::Utils::MetaData& options);

    const PostProcessTarget& target() const;
    const STI::Utils::MetaData& options() const;

    template<class Archive> void serialize(Archive&);
private:
    PostProcessTarget _target;
    STI::Utils::MetaData _options;
};
```

### `RawEventGroup` extension

Add a new member (parallel to the existing events/vars/tags storage) and a
new builder method, analogous to `addEvent()` but explicitly **not** routed
into the hard-timed event list:

```
void RawEventGroup::addPostProcessRequest(const PostProcessTarget& target,
                                           const STI::Utils::MetaData& options,
                                           const CompressedStackTrace& stackTrace);

const std::vector<PostProcessRequest>& RawEventGroup::postProcessRequests() const;
```

This list must **not** flow through `parseEventsDefault()`'s event-table path
or be wrapped in a `PseudoSynchronousEvent`. It is carried alongside the
group purely as metadata, the same way `ParsedVar`/`ParsedTag` are.

**Required network transport for the side-list (do not defer to Phase 2).**
`RawEventGroup` is built on the Python client and shipped to the server that
parses it, crossing CORBA as `TRawEventGroup` (`orbTypes.idl:430`) via
`Convert_RawEventGroup.cpp`. The server's `LocalEventEngine` only sees
`postProcessRequests()` if they survive that conversion, so the new side-list
needs its own field on the `TRawEventGroup` IDL struct plus matching
serialize/deserialize in `Convert_RawEventGroup.cpp` (and a re-run of
`compileIDL.sh`). This is unavoidable for the core end-to-end flow — a
`postProcess()` written in a timing file is useless if it never reaches the
parsing server — so although it edits `stinetwork`, schedule it as **Phase 1.5**
(immediately after the Phase 1 types compile), not bundled with the Phase 2
completion-message relay. `PostProcessRequest`/`PostProcessTarget` carry a
`MetaData` and reuse `RawEventTargetDevice`, both already network-converted, so
the conversion is mechanical.

### `PostProcessingManager` (abstract interface)

`include/sti/device/PostProcessingManager.h`. Follows the
`ChannelManager`/`AttributeManager` template.

```
using PostProcessingFunction =
    std::function<STI::Utils::MetaData(const STI::Engine::ShotID&, const STI::Utils::MetaData&)>;

struct PostProcessingTargetInfo
{
    std::string name;
    std::string description;
};

class PostProcessingManager
{
public:
    virtual ~PostProcessingManager() = default;

    // Device-author-facing: register a named target and its callback.
    virtual void addPostProcessingTarget(const std::string& name,
                                          PostProcessingFunction function,
                                          const std::string& description = "") = 0;

    // Discovery (interactive sessions, frontend).
    virtual std::vector<PostProcessingTargetInfo> getPostProcessingTargets() const = 0;

    // Entry point called when a shot is ready for processing. Enqueues and
    // returns immediately; returns false if `name` is not a registered
    // target on this device (caller should log, not treat as fatal).
    virtual bool requestPostProcessing(const std::string& name,
                                        const STI::Engine::ShotID& shotID,
                                        const STI::Device::DeviceID& shotOwnerID,
                                        const STI::Utils::MetaData& options) = 0;

    virtual void stop() = 0;
};
```

### `Device` interface accessor (required — this is the cross-device seam)

Every cross-device operation in this design reaches another device through a
`std::shared_ptr<STI::Device::Device>` obtained from the device collection
(`deviceCollection->get(id, device)`, e.g. `LocalEventEngine.cpp:866`). That
collection only ever yields the **abstract** `Device` interface
(`include/sti/device/Device.h:33`); when the target is remote it is backed by a
`RemoteDevice`, when local by a `LocalDevice`. Every existing manager is reached
this way via a pure-virtual getter on `Device` (`getPersistenceManager`,
`getTaskManager`, …). The post-processing manager is no exception, so the
abstract interface must gain a matching accessor — **without it there is no way
to get from a `Device*` to its `PostProcessingManager`, and the job-completion
dispatch below does not compile.**

```
// Add to include/sti/device/Device.h, mirroring getTaskManager():
virtual bool getPostProcessingManager(std::shared_ptr<PostProcessingManager>& manager) { manager.reset(); return false; }
```

Provide a non-pure default returning `false` (like `getVersionManager` at
`Device.h:59`) so existing `Device` implementations that never post-process —
and the Phase 2 servants/proxies until they are wired — keep compiling.
Implement it in:
- `LocalDevice` (Phase 1) — returns the device's `LocalPostProcessingManager`.
- `RemoteDevice` (Phase 2) — returns a `RemotePostProcessingManager` wrapping a
  `getTPostProcessingManager()` CORBA call (mirror `RemoteDevice.cpp:412` /
  `:488`).
- `PartnerDevice` (Phase 2) — add the pass-through getter alongside its other
  manager accessors (`include/sti/device/PartnerDevice.h:44-51`).

### `LocalPostProcessingManager`

`src/device/src/LocalPostProcessingManager.h/.cpp`. Concrete implementation.

- Holds `std::map<std::string, PostProcessingFunction>` +
  `std::map<std::string, std::string>` (descriptions).
- Holds one `EventQueue<PostProcessWorkItem>` and its worker thread, where
  `PostProcessWorkItem { std::string targetName; ShotID shotID; DeviceID shotOwnerID; MetaData options; }`.
  Lazily start the worker thread on the **first** `addPostProcessingTarget()`
  call rather than unconditionally in the constructor — this keeps devices
  that never register a target free of an idle thread, while still letting
  `LocalDevice` construct the manager unconditionally like its other managers
  (no special-cased optional member).
- **Dependency injection (boundary-respecting).** To resolve a remote owning
  device's `PersistenceManager` the manager needs device lookup, but it must
  **not** include `NetworkDeviceHub` (that is `stinetwork`; see AGENTS.md's
  dependency-boundary rule). Inject a `std::shared_ptr<STI::Device::DeviceCollection>`
  — the abstract collection that yields `Device` pointers — exactly as
  `LocalEventEngine` (`LocalEventEngine.h:240`) and `LocalPersistenceManager`
  (`LocalPersistenceManager.h:135`) already receive one. The remote-ness of the
  owning device is invisible here: `collection->get(ownerID, dev)` followed by
  `dev->getPersistenceManager(pm)` returns a local or remote `PersistenceManager`
  through the same abstract interface.
- **Target-registry thread safety.** The worker thread reads the
  `name -> PostProcessingFunction` map while `addPostProcessingTarget()` may
  insert into it. Registration is expected to happen at device startup, before
  any `requestPostProcessing()` arrives, but the design must not rely on that
  ordering implicitly — guard both the map and the descriptions map with a
  mutex (or document and enforce "all targets registered before the manager is
  served" as an invariant).
- `requestPostProcessing()`: validate `name` is registered; if not, return
  `false` (caller logs). Otherwise push a `PostProcessWorkItem` and return
  `true` immediately (non-blocking).
- Worker loop, per item:
  1. Resolve the `ShotResult` for `shotID` by backing a `ResultTicket` with the
     owning device's `PersistenceManager`, mirroring `ResultTicketManager`'s
     existing pattern: if `shotOwnerID` is this device's own ID, use the local
     `PersistenceManager`; otherwise `collection->get(shotOwnerID, dev)` then
     `dev->getPersistenceManager(pm)` (local or remote, via the abstract
     `Device` — see "Device interface accessor" above) and back the
     `ResultTicket` with that `pm`. Because dispatch is triggered off the
     `PlayComplete` notification (decision 9), the result is already persisted
     and the ticket resolves without an indefinite wait.
  2. Call the registered `PostProcessingFunction` with `(shotID, options)`
     inside a try/catch.
  3. On success: build `PostProcessingCompleteMessage` with
     `status = Success`, `results = <returned MetaData>`.
     On exception: `status = Failed`, `results` empty,
     `errorMessage = e.what()`.
  4. Dispatch the message via this device's `DeviceMessageDispatcher`
     (broadcast — see message type below).
- **Shutdown ordering.** `stop()` must join the worker thread, but the worker
  may be mid-callback inside user Python code (holding the GIL) when the device
  is torn down. Define the shutdown order explicitly — stop accepting new work
  items, then drain/cancel the queue and join the worker, releasing the GIL
  appropriately — and call `stop()` from `LocalDevice`'s teardown alongside the
  other managers, so a long-running user callback cannot wedge device shutdown.
- Wire into `LocalDevice` the same way `LocalTaskManager` is wired
  (`LocalDevice.cpp:192-255`): construct unconditionally in the constructor,
  add a `getPostProcessingManager()` accessor following the existing
  getter pattern (e.g. `LocalDevice.cpp:947-1011`-style
  `bool getX(std::shared_ptr<X>&)`), and expose
  `LocalDevice::addPostProcessingTarget(...)` as a thin pass-through
  convenience (mirrors how `LocalDevice::addTask(...)` already delegates to
  its task manager).

### `PostProcessingCompleteMessage`

Add to `include/sti/device/DeviceMessage.h`, alongside the other
`DeviceMessage` subclasses. New `DeviceMessageType::PostProcessingComplete`
entry in `include/sti/device/DeviceMessageType.h`.

```
enum class PostProcessingStatus { Success, Failed };

class PostProcessingCompleteMessage : public DeviceMessage
{
public:
    static DeviceMessageType getMessageClassType() { return DeviceMessageType::PostProcessingComplete; }

    STI::Engine::ShotID shotID;
    std::string targetName;
    PostProcessingStatus status;
    STI::Utils::MetaData results;     // empty when status == Failed
    std::string errorMessage;         // empty when status == Success
};
```

No `Request` message type is needed (decision 3) — request delivery is the
`requestPostProcessing()` RPC on `PostProcessingManager` itself.

## Parse-time integration

In `LocalEventEngine`'s parse path, alongside the existing dependency-tree
walk over `eventsByTarget`:

1. After building the dependency tree for hard-timed targets, walk
   `RawEventGroup::postProcessRequests()` for the shot being parsed.
2. For each `PostProcessRequest`, attempt to resolve its `PostProcessTarget`'s
   device against the hub, the same way `parseDevice()` resolves a regular
   event target's device (`LocalEventEngine.cpp:895-900`'s contact attempt).
3. On success: mark resolved, keep the request in the "present" list used at
   job-completion time (see below).
4. On failure: emit a new, separate, non-fatal parsing warning (new warning
   message, distinct from the existing "Missing device"/ID 101 and
   "Abstract Shot"/ID 100 — pick new IDs that don't collide, e.g. document
   them in the warning-message registry wherever IDs 70/100/101 are tracked)
   and record the target into a **new** `missingPostProcessingTargets` set
   — **not** `missingTargets`. Do **not** touch `isAbstractShot()`. This is
   the entire mechanism by which a missing post-processing device warns but
   never blocks play (decision 2).

### Retaining and reaching the resolved list (verified against the scheduler)

The resolved/present list from step 3 must survive from parse time to
`PlayComplete` time. Two facts about the existing code constrain how:

- **It must ride the *play* job, not just the parse job.** `PlayComplete` is a
  play-job event, and a play job is a distinct `LocalEventEngineJob` from the
  parse job. Parse-time data only reaches the play job because it is explicitly
  copied at construction — `LocalEventEngineScheduler.cpp:582`,
  `job->setMissingTargets(parseJob->getMissingTargetIDs())`. So mirroring
  `missingTargetIDs` is **mandatory, not stylistic**: add the resolved list as a
  new `LocalEventEngineJob` field *and* copy it at that same line. Stored only
  on the parse job, it is unreachable from the `PlayComplete` handler.
- **The `PlayComplete` listener can retrieve the job by `sid`.** A
  `DeviceMessageListener<EngineSchedulerMessage>` holding a scheduler reference
  (as `ResultTicketManager` holds `eventEngineScheduler`) can call
  `scheduler->findJob(sid, job)` (`LocalEventEngineScheduler.cpp:1704`, which
  searches `completedPlayJobs` → `runningJobs` → `queuedJobs`). That the
  `PlayComplete` message reaches such a listener at all is confirmed by
  `ResultTicketManager` already consuming it (`LocalDevice.cpp:250`).

**Eviction caveat — do not depend on the job cache.** `completedPlayJobs` is an
`OrderedBufferMap` capped at `setMaxSize(6)`
(`LocalEventEngineScheduler.cpp:149`). `ResultTicketManager` is immune to this
because it reads results from the `PersistenceManager` by `sid` (persistence
outlives the cache); the resolved-request list, by contrast, lives in memory on
the job. Under a fast sequence, 6+ play jobs can complete between the async
`PlayComplete` dispatch and the listener running, evicting the job so
`findJob(sid)` fails and post-processing is **silently skipped** for that shot —
unacceptable for a data pipeline. Therefore do **not** retrieve the list via
`findJob` at listener time. Instead stash it in a scheduler-owned
`std::map<ShotID, std::vector<PostProcessRequest>>` populated when the play job
is created and consumed/erased by the `PlayComplete` listener, decoupling it
from the bounded job cache. (Persisting it alongside the shot result, the way
`ResultTicketManager` leans on persistence, is the maximally robust alternative
but adds more plumbing; the scheduler-side map is the lighter sufficient fix.)

## Job-completion integration

Dispatch is triggered by the **`PlayComplete` `EngineSchedulerMessage`**, on the
dispatcher thread, the same notification `ResultTicketManager` already consumes
(decision 9) — **not** synchronously inside
`LocalEventEngineScheduler::jobComplete()`, which holds `jobMutex` for its whole
body (`LocalEventEngineScheduler.cpp:1019`) and must not be blocked by a
possibly-remote `requestPostProcessing(...)` RPC.

Mechanism: register a `DeviceMessageListener<EngineSchedulerMessage>` (the same
listener type and registration site as `ResultTicketManager`,
`LocalDevice.cpp:250`) that fires on `SchedulerMessageType::PlayComplete`. On
that message:

1. Look up the resolved `PostProcessRequest`s for `mess->jobID.sid` in the
   scheduler-owned `std::map<ShotID, std::vector<PostProcessRequest>>` and erase
   the entry (see "Retaining and reaching the resolved list" — this map, not
   `findJob()`, is the source of truth, because the in-memory play job can be
   evicted from the size-6 `completedPlayJobs` cache before this listener runs).
2. For each present request, look up the target device through the abstract
   `DeviceCollection` (`collection->get(targetDeviceID, dev)`) and call
   `dev->getPostProcessingManager(ppm)` then
   `ppm->requestPostProcessing(targetName, shotID, shotOwnerID, options)`
   (via the new `Device` accessor — local `LocalPostProcessingManager` in
   Phase 1, `RemotePostProcessingManager` RPC in Phase 2). `shotOwnerID` is the
   job's owning device ID (same `jobOwner` field tracked for
   `EngineJobUpdateDeviceMessage`).
3. Treat delivery failure (target unreachable *now*, despite resolving at parse
   time — e.g. it dropped off the network in between) as a best-effort,
   log-only condition via this device's `Logger`. The shot has already finished
   playing; there is no play-time error path to interact with here, by
   construction.

Because this runs on the dispatcher thread (not the scheduler thread, not under
`jobMutex`), the dispatch is structurally incapable of delaying `jobComplete()`,
and `PlayComplete` guarantees the shot's results are persisted before the
post-processing device pulls them — both properties fall out of reusing the
existing `ResultTicketManager` notification path rather than re-deriving the
timing.

## Phase 2 (stinetwork) — sketch, implement after Phase 1 is solid

Following the traced template for `ChannelUpdateMessage` end-to-end:

- **`RemotePostProcessingManager`** (`src/network/src/`) — implements
  `PostProcessingManager`, wraps a CORBA `TPostProcessingManager` reference.
- **`TPostProcessingManager_i`** (`src/network/src/`) — POA servant exposing
  a local device's `PostProcessingManager` over CORBA.
- **IDL additions:**
  - `src/network/idl/deviceNet.idl`: new `TPostProcessingManager` interface
    with `requestPostProcessing(...)` and `getTargets()`, mirroring the
    existing `TPersistenceManager`/`TChannelManager` interfaces
    (`deviceNet.idl:265-277` is the `TPersistenceManager` precedent). Also add
    a `getPostProcessingManager()` accessor to the `TDevice` interface that
    returns a `TPostProcessingManager` reference (mirror `TDevice`'s existing
    `getPersistenceManager()` — `RemoteDevice.cpp:432` is the client side that
    calls it), so `RemoteDevice::getPostProcessingManager()` has a CORBA call
    to back the new abstract `Device` accessor.
  - `src/network/idl/orbTypes.idl`: new `TPostProcessingCompleteMessage`
    struct (mirrors `TChannelUpdateMessage`, `orbTypes.idl:696-711`) and a
    new `TDeviceMessageType` enum entry.
- **`Convert_DeviceMessage.h/.cpp`**: new template specializations
  (`TNetwork::TPostProcessingCompleteMessage` <-> `PostProcessingCompleteMessage`)
  and a new case in the `TAnyMessage` dispatch switch
  (`Convert_DeviceMessage.cpp` around the existing switch, see line ~329 in
  the current file for the pattern).
- **Relay/listener registration**: mirror `RemoteChannelManager`'s
  `DeviceMessageListenerForwarder` usage (`RemoteChannelManager.h:76-94`) so
  remote listeners can subscribe to `PostProcessingCompleteMessage` the same
  way they subscribe to channel updates today.
- Run `src/network/compileIDL.sh` after any `.idl` edit, per AGENTS.md; never
  hand-edit `src/network/src/generated/`.

Estimated scope, based on tracing `ChannelUpdateMessage`'s full path: roughly
200-250 lines across IDL + converters + relay registration, all mechanical
and following an established example — no novel network-layer design work
expected.

## stipy / Python layer

### C++ / pybind additions

- Wrap `PostProcessTarget` (mirrors `RawEventTargetChannel_wrap`-style
  binding: constructor from `(deviceName, name)`, `isAbstract()`, `name()`,
  `device()`).
- Wrap `LocalDevice::addPostProcessingTarget(name, function, description="")`
  — `function` should accept a Python callable; translate Python exceptions
  into the same Failed-status path as C++ exceptions (mirror the existing
  `_parseEventsWrapper` pattern in `localdevice.py` that wraps C++ -> Python
  calls with error handling, since `requestPostProcessing`'s worker thread
  will be calling back into user Python code and needs the GIL acquired
  around that call).
- Wrap `getPostProcessingTargets()` discovery call on `LocalDevice`/`RemoteDevice`.
- Wrap `RawEventGroup::addPostProcessRequest(target, options, stackTrace)`.

### Python-facing API (camelCase, per decision 7)

In `src/stipy/python/group.py` (mirrors `ch()`/`event()`/`meas()`):

```python
def postTarget(device, name):
    target = PostProcessTarget(device, name)
    return target

def postProcess(self, target, options=None):
    return _addPostProcessRequest(self, target, options or MetaData(), makeStackTrace())
```

Note the deliberate absence of a `time` argument — this is the visible
signal, right in the call shape, that a `postProcess()` declaration is not
hard-timed (unlike `.event(target, time, value)`/`.meas(target, time, value)`).

In `src/stipy/stidevicepy/python/localdevice.py` (mirrors how channels are
registered/discovered today):

```python
def addPostProcessingTarget(self, name, function, description=""):
    ...

def getPostProcessingTargets(self):
    ...
```

`postTarget("DeviceName", "Blue MOT")` must work with no live connection
(standalone timing files), exactly like `dev()`/`ch()` do today, by virtue of
`PostProcessTarget` carrying the same name-only/`isAbstract()` semantics as
`RawEventTarget`. Live discovery via `getPostProcessingTargets()` is a
separate, complementary path for interactive sessions and the frontend —
both consume the same underlying named-target concept at different times
(script-authoring vs. network resolution), the same dual-mode split channels
already have.

## Test plan

Per `test/AGENTS.md` conventions — one `*_tests.cpp` per new module/class,
Catch2, tagged for filtering:

- `test/device/postprocesstarget_tests.cpp` — `PostProcessTarget`
  construction, `isAbstract()`, equality/ordering, serialization round-trip.
  Tag `[postprocessing]`.
- `test/device/postprocessingmanager_tests.cpp` — `LocalPostProcessingManager`:
  registering targets, `requestPostProcessing()` returning `false` for an
  unregistered name, queueing + worker execution (success and exception
  paths), `PostProcessingCompleteMessage` content on both paths. Tag
  `[postprocessing] [localdevice]`.
- Extend `test/device/eventengine_*_tests.cpp` or add a new
  `eventengine_postprocessing_tests.cpp` covering: a shot with a resolvable
  post-processing target plays normally; a shot with a missing
  post-processing target still plays (no error) but produces the new
  non-fatal warning; a regular missing hard-timed device still produces the
  existing hard error (regression check that the two paths stay separate).
  Tag `[postprocessing] [eventengine]`.
- `test/network/...` (Phase 2): `Convert_DeviceMessage` round-trip test for
  `PostProcessingCompleteMessage`, mirroring the existing convert tests for
  other message types.

Per AGENTS.md: do not modify production code while adding tests; compile and
run with the narrowest Catch2 filter while iterating
(`build-ninja/test/sti3_test_device "[postprocessing]"`), full suite only at
the end.

## Resolved during review (was previously under-specified)

- **Cross-device seam:** the abstract `Device` interface gains
  `getPostProcessingManager()` (see "Device interface accessor"); without it the
  job-completion dispatch cannot reach a remote target's manager.
- **Side-list network transport:** `TRawEventGroup` + `Convert_RawEventGroup.cpp`
  must carry `postProcessRequests` (Phase 1.5) or the feature never works over
  the network.
- **Dispatch timing / no scheduler stall / no result race:** dispatch hangs off
  the `PlayComplete` `EngineSchedulerMessage` listener path
  (decision 9, "Job-completion integration"), not synchronously inside
  `jobComplete()` under `jobMutex`. This reuses the exact timing
  `ResultTicketManager` relies on, so results are guaranteed persisted at
  dispatch time.

## Open questions (deferred, flag for implementer/reviewer)

- Exact new parse-warning message IDs (existing: 70 = "Cannot Play Abstract
  Shot", 100 = "Abstract Shot", 101 = "Missing device" — confirm the
  registry/convention for assigning new IDs before picking numbers).
- Behavior if `postProcess()` is called more than once for the same target
  within a single shot: queue multiple work items (current default
  assumption in this doc), warn, or error? Not yet decided.
- Whether `requestPostProcessing`'s "unregistered target name" case should
  also surface as a device-message/log event visible to the job owner (right
  now it's specified as a local log-only condition on the post-processing
  device itself).
- Final placement of the new parse-time resolution pass within
  `LocalEventEngine`'s existing parse flow (this doc specifies *what* it must
  do and *which sets it must not touch*, but the precise insertion point
  should be confirmed against the current parse function structure at
  implementation time, since that file is large and actively changes).
