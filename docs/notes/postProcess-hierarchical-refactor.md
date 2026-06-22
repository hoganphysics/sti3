# Post-processing refactor: per-engine state + hierarchical tree-routed dispatch

Date: 2026-06-22

Status: planning (no edits yet). Supersedes the dispatch/storage mechanism in
`postProcess.md`; the core types (`PostProcessTarget`, `PostProcessRequest`,
`PostProcessingManager`/`LocalPostProcessingManager`, `PostProcessingComplete`
message, `requestPostProcessing` RPC) are unchanged.

## Why

Three issues with the shipped implementation (3.6.0, commit `efa9077`):

1. **State lives on the scheduler, not the engine.** `resolvedPostProcessRequests`
   is a `std::map<ShotID, std::vector<PostProcessRequest>>` on
   `LocalEventEngineScheduler`, copied parse-job → play-job and stashed by `ShotID`.
   The EventEngine model is that each `LocalEventEngine` is the self-contained
   per-shot unit; shot state belongs there.

2. **Target resolution only searches the job owner's local collection.**
   `LocalEventEngine::resolvePostProcessDevice()` uses `deviceCollection->getIDs()`
   / `get()`. In a nested topology (root → server1 → dev1) the root owner's
   collection does not contain `dev1`, so the request is wrongly dropped with a
   "Missing post-processing target" warning.

3. **Dispatch only searches the job owner's local collection.**
   `PostProcessingDispatcher::dispatchRequest()` resolves the target with
   `deviceCollection->get(targetID)` on the owner — same hierarchy failure.

The event system already solves hierarchical targeting: parse builds an
`EventEngineDependencyTree` (`LocalEventEngineScheduler::parseJob`,
`LocalEventEngineDependencyParser::getDependants`) that locates every target's
owning-server chain, and routing uses `EventEngineDependencyTree::getBranchToTarget`
(`LocalEventEngine::addEvent`, `LocalEventEngine.cpp:328`). We reuse that.

## Design decisions (locked)

- **D1. Per-engine storage.** Resolved post-process requests are stored on the
  job owner's `LocalEventEngine` during parse and consumed at the end of that
  engine's play. Play is always assigned to the engine that parsed
  (`assignPlayJobs` → `findParsedEngine` → `manager->isParsed(pid)`,
  `LocalEventEngineScheduler.cpp:1328,1616`), so no copy into the play job and no
  scheduler-side map is needed.

- **D2. Do not parse/play post-process-only targets.** Post-process requests are
  *not* sent downstream during parse, and post-process targets are *not* forced
  into the engine state machine (no `Parsed`/`PlayReady` for a device that only
  post-processes). Post-processing is intentionally decoupled from the gated
  event play handshake (preserves the original "post-processing never blocks
  play" guarantee). Only the dependency-tree *location* of each target is learned
  during parse; nothing is delivered until play completes.

- **D3. Tree-routed dispatch at end of play.** When the job owner's engine
  finishes play and the shot result is persisted, it dispatches each resolved
  request along the dependency tree: requests for the owner itself or its
  directly-owned devices are delivered immediately; the rest are forwarded, as
  per-branch sublists, to the owned branch that leads to each target. Each branch
  device repeats the same routing with the same tree, to arbitrary depth. This
  mirrors how parse distributes events.

- **D4. Thin, synchronous delivery is acceptable.** The forwarding/delivery RPCs
  must be thin: `requestPostProcessing` only enqueues a work item on the target
  and returns; the actual analysis runs asynchronously on the target's worker
  thread. Dispatching from inside `play()` (right before sending `PlayComplete`)
  is allowed because each call returns immediately after enqueueing. The heavy
  work is out of the play path, which is within the spirit of the original spec.

## Target architecture

```
PARSE (job owner only)
  scheduler.parseJob:
    eventTargets   = findEventTargets(shot)                       (existing)
    ppTargets      = findPostProcessTargets(shot)                 (NEW: concrete target device IDs)
    getDependants(eventTargets, tree, eventMissing, ...)          (existing)
    getDependants(ppTargets,    tree, ppMissing,    ...)          (NEW: same tree, separate missing set)
    abstract-shot / "Missing Targets" use eventMissing ONLY       (decision 2 preserved)
    job.setDependencies(tree)                                     (existing; tree now also covers pp targets)
  engine.parse (reads tree from job):
    resolvePostProcessRequests(rootGroup, tree):
      for each request: resolve target to its canonical tree node
        in tree  -> keep (store resolved request on THIS engine)
        not in tree (incl. abstract) -> non-fatal "Missing post-processing target" warning, drop
    store resolvedRequests on the LocalEventEngine                (NEW per-engine member; replaces job/scheduler map)

PLAY-COMPLETE (job owner engine, end of LocalEventEngine::play, after saveShot)
  if isJobOwner and !resolvedRequests.empty():
    distributePostProcessing(resolvedRequests, tree, sid, ownerID)   // thin, returns fast

distributePostProcessing(requests, tree, sid, ownerID)   // runs on owner AND each hop
  for each request, branch = getBranchToTarget(localDeviceID, target):
    target == localDeviceID            -> local PostProcessingManager.requestPostProcessing(...)   [enqueue, return]
    branch == target (directly owned)  -> collection.get(target).getPostProcessingManager().requestPostProcessing(...)  [thin RPC]
    branch != target (deeper)          -> group by branch; collection.get(branch).getEngineScheduler()
                                            .distributePostProcessing(branchSublist, tree, sid, ownerID)   [thin recursive RPC]
    no branch                          -> best-effort log (should not happen if tree is correct)
```

The only data that crosses the network at dispatch time is the per-branch
request sublist plus the (already CORBA-serializable) dependency tree.

## Phased plan

### Phase A — Move resolved-request storage onto the engine (issue 1)

Goal: per-engine ownership of the resolved list; delete the scheduler map and the
play-job copy. No behavior change yet (still owner-collection resolution; fixed in
Phase B).

- `LocalEventEngine` (`src/device/src/LocalEventEngine.{h,cpp}`): add a member
  `std::vector<STI::Engine::PostProcessRequest> resolvedPostProcessRequests;`
  cleared in the per-parse reset block (alongside `missingPostProcessingTargets`,
  `LocalEventEngine.cpp:193`). `resolvePostProcessRequests()` stores into this
  member instead of `job.setPostProcessRequests(...)`.
- `LocalEventEngineScheduler` (`.h/.cpp`): delete `resolvedPostProcessRequests`
  map, `postProcessMutex`, `takePostProcessRequests()`, and the stash block in
  `play()` (`LocalEventEngineScheduler.cpp:579-588`).
- `EventEngineJob` (`include/sti/engine/EventEngineJob.h`) and
  `LocalEventEngineJob` (`.h/.cpp`): remove the `getPostProcessRequests()` /
  `setPostProcessRequests()` virtuals and the `postProcessRequests` field.
- The `PostProcessRequest` IDL transport on `TRawEventGroup`
  (`Convert_RawEventGroup`) **stays** — that is the client→server side-list and
  is unrelated to where the server stores the resolved list.

Risk: the engine is reused for the next shot after play; the dispatch (Phase C)
must read `resolvedPostProcessRequests` before a later parse resets it. The
scheduler will not reassign the engine until play finishes, and the dispatch
happens inside `play()`, so this holds. (Capture/clear the list at dispatch.)

### Phase B — Put post-process targets into the dependency tree (issue 2)

Goal: the job owner learns every post-process target's location and owning-server
chain via the existing dependency search, without affecting the abstract-shot
gating.

- `LocalEventEngineScheduler::parseJob` (`LocalEventEngineScheduler.cpp:435`):
  - Add `findPostProcessTargets(eventGroup, ppTargets)` mirroring
    `findEventTargets` (`:381`) but reading `eventGroup->postProcessRequests()`
    (recursing subgroups) and collecting the **concrete** target device IDs.
  - After the existing `getDependants(eventTargets, tree, missingTargets, ...)`,
    add a second call `getDependants(ppTargets, tree, ppMissing, ppMessages, 5)`
    on the **same** `tree`. The tree accumulates the post-process nodes and their
    server chains; `eventMissing`/abstract-shot logic is computed from
    `eventTargets` only, so a missing analysis device never makes the shot
    abstract (decision 2 / non-fatal preserved).
  - **Verify `getDependants` is additive on a pre-populated tree** (it adds
    vertices/edges and never clears the passed-in tree; the recursive overload
    only clears local *subtrees*). Add a focused test for "second call extends
    the tree."
- `LocalEventEngine::resolvePostProcessRequests` /
  `resolvePostProcessDevice` (`LocalEventEngine.cpp`): replace the
  `deviceCollection` name/ID scan with a **dependency-tree** lookup. For each
  request, resolve its target to the canonical tree node (the tree node carries
  the full server chain needed for routing). If present → keep (store on the
  engine, Phase A); if absent (unreachable or abstract/name-only) → the existing
  non-fatal `"Missing post-processing target"` warning (id 103) and drop. This
  stays job-owner-only (`if (!isJobOwner) return;`).

**Abstract (name-only) targets — resolved decision.** `postTarget("dev1", ...)`
(name only) is allowed in the timing file. Concretization is the user's
responsibility via an abstract→concrete target dictionary supplied at `makeshot`
or `parse`, exactly like timing events (this binding flow is shared and is still
WIP). Behavior:

- If a post-process target is bound to a concrete device ID before parse, it
  appears in the dependency tree (`getDependants` is `DeviceID`-based) and is
  dispatched normally.
- If it is still abstract/unresolved at parse time, emit the non-fatal warning at
  the **same point the other missing-target/abstract warnings are emitted**, do
  **not** block play, and **skip** that request during the final dispatch step.

Prerequisite (WIP, separate from this refactor): the existing event
target-binding mechanism (`RawEventGroup::bindDeviceTargets` /
`getConcreteTarget`) currently rewrites only the event list, not the
post-process side-list. For a supplied dictionary to concretize post-process
targets, binding must be extended to rewrite `PostProcessRequest` device targets
too. Until then, post-process targets must be authored concrete (e.g.
`postTarget(dev(name, addr, module), target)`) to be dispatched; abstract ones
take the warning-and-skip path above.

### Phase C — Tree-routed distributed dispatch (issue 3)

Goal: deliver requests to targets at arbitrary depth via the ownership tree;
remove the central, owner-collection-only dispatcher.

New network RPC on the per-device scheduler interface (the same seam used by
`addJob`/`getDependants`, reachable via `Device::getEngineScheduler()`):

- IDL (`src/network/idl/deviceNet.idl`, `TEventEngineScheduler` interface,
  near `addJob` line 189):
  ```
  void distributePostProcessing(in TPostProcessRequestSeq requests,
                                in TEventEngineDependencyTree tree,
                                in TShotID shotID, in TDeviceID jobOwnerID);
  ```
  `TPostProcessRequestSeq` and `TEventEngineDependencyTree` already exist and are
  already converted (`Convert_RawEventGroup`, `Convert_EventEngine`). Run
  `compileIDL.sh` after the edit.
- Abstract `EventEngineScheduler` (`include/sti/engine/EventEngineScheduler.h`):
  add the matching pure-virtual `distributePostProcessing(...)`.
- `LocalEventEngineScheduler::distributePostProcessing(...)` implements the
  routing routine described in "Target architecture":
  - partition `requests` by `tree.getBranchToTarget(localDeviceID, target, branch)`;
  - `target == localDeviceID` → deliver to the local device's
    `PostProcessingManager` (`localDevice->getPostProcessingManager(ppm)`);
  - `branch == target` (directly owned) →
    `deviceCollection->get(target, dev); dev->getPostProcessingManager(ppm);
    ppm->requestPostProcessing(name, sid, ownerID, options)` (thin; for a remote
    directly-owned device this is the existing `RemotePostProcessingManager`
    CORBA call);
  - `branch != target` → group requests by `branch`, and for each branch
    `deviceCollection->get(branch, dev); dev->getEngineScheduler(sched);
    sched->distributePostProcessing(branchSublist, tree, sid, ownerID)` (recursive
    RPC; `sched` is a `RemoteEventEngineScheduler` when the branch is remote);
  - unroutable target → best-effort `Logger` line (decision: log-only).
- `RemoteEventEngineScheduler` (`src/network/src/RemoteEventEngineScheduler.{h,cpp}`):
  implement `distributePostProcessing` as the marshalling CORBA call (mirror an
  existing void RPC such as `addJob`).
- `TEventEngineScheduler_i` (`src/network/src/TEventEngineScheduler_i.{h,cpp}`):
  servant method converts args and calls the local scheduler.
- Initiation: in `LocalEventEngine::play()` (`LocalEventEngine.cpp:~1535-1549`),
  after `persistenceManager->saveShot(...)` and guarded by `isJobOwner`, call the
  routing routine with the engine's `resolvedPostProcessRequests` + `dependencyTree`
  + `jobID.sid` + `job.getJobOwner()`. The engine reaches branch schedulers via
  `deviceCollection->get(branch)->getEngineScheduler()`, and the owner's own
  device `PostProcessingManager` via its `LocalDevice` (`deviceParser`). To avoid
  duplicating the routing logic, factor it into a shared helper invoked by both
  the engine (initiation) and `LocalEventEngineScheduler::distributePostProcessing`
  (recursion); the helper only needs `localDeviceID`, `deviceCollection`, the
  local `PostProcessingManager`, the tree, and the request list.
- **Remove** `PostProcessingDispatcher` (`src/device/src/PostProcessingDispatcher.{h,cpp}`),
  its construction and `EngineSchedulerMessage` listener registration in
  `LocalDevice.cpp`, and its CMake entry. The `PlayComplete`-listener mechanism is
  replaced by the in-`play()` initiation above.
- **Unchanged:** the new RPC replaces only the *delivery/routing* path. It ends at
  `LocalPostProcessingManager::requestPostProcessing`, which still validates the
  target name, enqueues a `PostProcessWorkItem`, and returns immediately. The
  manager's `EventQueue` worker thread still runs `handleEvent(item)`
  asynchronously to do the actual analysis and broadcast the completion message —
  that is the "thin dispatch, async heavy work" split and is not part of this
  routing change (but its callback contract changes in Phase D).

Blocking budget (decision D4): the owner's `play()` blocks for the duration of
the recursive dispatch. Each hop does N thin `requestPostProcessing` enqueues + M
forwarding RPCs; forwarding is synchronous, so the owner blocks for the full
post-process subtree traversal. This is bounded by the (small) number of
post-process targets and is acceptable. If a future topology makes this latency
matter, the forwarding RPCs can be made `oneway` or run on a worker thread
without changing the routing logic — flag, do not implement now.

### Phase D — Worker pulls the ShotResult; callback receives it

Goal: remove the pull-and-discard `ensureResultsAvailable` weakness, abort
cleanly when the shot is unavailable, and remove per-target boilerplate by
handing the pulled `ShotResult` to the user callback instead of a `ShotID`.

Today `ensureResultsAvailable` resolves the owner's `PersistenceManager`, calls
`getShotResult`, then **discards the result and only logs** — so it guarantees
nothing and forces every callback to re-resolve and re-pull the shot.

Changes:

- **Callback contract.** Change
  ```
  using PostProcessingFunction =
      std::function<MetaData(const ShotID&, const MetaData&)>;
  ```
  to
  ```
  using PostProcessingFunction =
      std::function<MetaData(const std::shared_ptr<STI::Engine::ShotResult>&, const MetaData&)>;
  ```
  (`include/sti/device/PostProcessingManager.h`).
- **`LocalPostProcessingManager::handleEvent`** (`src/device/src/LocalPostProcessingManager.cpp`):
  replace `ensureResultsAvailable` (best-effort log) with a real resolve+pull,
  distinguishing the two failure modes with **distinct error messages** (both
  `status = Failed`, but the message tells the user which step failed):
  1. Resolve the owner's `PersistenceManager`: self → local; else
     `collection->get(shotOwnerID, dev)` then `dev->getPersistenceManager(pm)`.
     If this fails (owner not in the collection — typically because it was not
     declared via `addPartner` — or it exposes no `PersistenceManager`) →
     **abort** without calling the user function; dispatch
     `status = Failed`, `errorMessage =
     "post-processing owner device '<ownerID>' is not reachable; declare it as a
     partner (addPartner) on the post-processing device"`.
  2. Otherwise `pm->getShotResult(shotID, result)`. If missing/null → **abort**;
     dispatch `status = Failed`, `errorMessage =
     "shot result '<sid>' not found on owner device '<ownerID>'"`.
  3. If found → call the user function with the `ShotResult` (and options), inside
     the existing try/catch (callback exceptions remain a third `Failed` path with
     `e.what()`).
- **`requestPostProcessing` signature is unchanged** (`name, shotID, shotOwnerID,
  options`) — the manager still needs `shotID` + `shotOwnerID` to know what to
  pull and from whom. Only the *callback* signature changes.
- **stipy** (`LocalDevicePy::addPostProcessingTarget` GIL wrapper): the bridged
  Python callable now receives a wrapped `ShotResult` (already pybind-exposed via
  `ShotResult_wrap.cpp`) and the options dict, and returns a results dict. Update
  the conversion accordingly.

Notes: the pull runs on the manager's worker thread (off the play path), so a
remote-owner pull via `RemotePersistenceManager` and lazy binary/image
materialization are fine here. Device authors now only need `addPartner(ownerID)`
for owners they analyze, and a callback `def fit(shot_result, options): ...`.

### Phase E — stipy, tests, docs

- stipy: no change to `postTarget`/`postProcess`; the `addPostProcessingTarget`
  callback now receives a `ShotResult` (Phase D). Document the abstract-target
  binding behavior (Phase B) and the new callback signature.
- Examples: update the committed `examples/cpp/postProcess` and
  `examples/python/postProcess` devices to the Phase D contract — `addPartner`
  the analyzed job owner(s) and write the callback as `fit(shotResult, options)`
  (dropping the manual `getPersistenceManager()/getShotResult()` boilerplate).
- Tests:
  - Keep `postprocesstarget_tests`. Update `postprocessingmanager_tests` for the
    Phase D callback contract: callbacks now take `(ShotResult, options)`; add a
    case where the shot result is unavailable → no callback, `Failed` completion
    message with the error; keep the success and exception-path cases (now driven
    through a backing `PersistenceManager` that returns a `ShotResult`).
  - Replace the scheduler-map assumptions in `eventengine_postprocessing_tests`
    with: (i) single device (owner == target) still works; (ii) **two-level
    hierarchy** (owner → sub-server → analysis device) where the analysis target
    is reachable only via the sub-server — verify the request is enqueued on the
    analysis device and the completion message is broadcast; (iii) missing target
    still produces the non-fatal warning and the shot still plays; (iv) a missing
    hard-timed device still hard-errors (regression that the two paths stay
    separate).
  - Add a dependency-parser test: `getDependants` second call extends a
    pre-populated tree with post-process-only target nodes.
  - Network: a convert/round-trip test for the new `distributePostProcessing`
    arguments is mostly covered by existing `TPostProcessRequestSeq` /
    `TEventEngineDependencyTree` conversions; add an integration check that a
    forwarded request reaches a nested target if a multi-process harness exists.
- Docs: update `docs/src/device.rst` and `docs/notes/postProcess.md` to describe
  tree-routed dispatch and the `ShotResult` callback; update the call-chain
  explanation. Fold the refactor into the existing **in-development 3.6.0**
  `versions.md` entry (this work ships as part of 3.6.0; no separate patch line).

## Invariants to preserve

- Post-processing never gates or blocks event play: post-process targets are not
  added to `ownedTargets` / the `PlayReady`/`PlayComplete` timeout machinery, and
  are not forced to `Parsed`. (Decisions D2; original decisions 1 and 9.)
- Missing post-process target = non-fatal parse warning, never
  `isAbstractShot()`/"Cannot Play Abstract Shot". (Original decision 2.)
- `requestPostProcessing` stays thin: enqueue and return; analysis runs on the
  target's worker thread; completion is a broadcast `PostProcessingCompleteMessage`.

## Side effects to check during implementation

- Adding post-process targets to the dependency tree means
  `LocalEventEngine::getOwnedDeviceIDs()` (`:218`, driven by
  `dependencyTree->getDependedentNodes`) may now surface post-process-only
  devices. Confirm this does not (a) misroute events in `divideEvents` (a
  post-process-only device has no events, so nothing routes to it), or (b)
  wrongly list a post-process device as a measurement owner in the `ShotResult`
  owned-IDs set (`LocalEventEngine.cpp:1419`). If (b) is a problem, filter
  `getOwnedDeviceIDs` consumers that must see event owners only.
- `getBranchToTarget` semantics for `target == root` and for a directly-owned
  target (expect `branch == target`): confirm against `addEvent`'s usage at
  `LocalEventEngine.cpp:328` before relying on it for routing.

## Open questions

1. **Abstract target binding (resolved; implementation WIP).** Name-only targets
   are allowed and concretized by a user-supplied abstract→concrete dictionary at
   `makeshot`/`parse`, shared with the event target-binding flow. The remaining
   work is extending that binding to rewrite the post-process side-list (see
   Phase B prerequisite); until then, author post-process targets concretely, and
   unresolved abstract targets warn-and-skip.
2. **Cross-hierarchy data pull (resolved).** A shot-analyzing device declares each
   anticipated job owner via `addPartner` in its constructor, which puts a
   *direct* reference to the owner in its collection, so
   `collection->get(ownerID)->getPersistenceManager()` resolves regardless of
   hierarchy depth (a partner reference is not subject to server-chain locality).
   Phase D distinguishes the two failure modes with separate `Failed` messages:
   "owner device not reachable / not declared as a partner" vs. "shot result not
   found on the owner device", so the user can tell whether to fix their
   `addPartner` configuration or look at why the shot data is missing.
3. **Dispatch latency bound.** Synchronous recursive forwarding blocks the owner's
   `play()` tail. Acceptable now; revisit with `oneway`/worker-thread forwarding if
   a topology makes it significant.
