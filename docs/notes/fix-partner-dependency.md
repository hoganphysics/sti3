# Partner dependency routing notes

Date: 2026-05-16

## Background

The original bug involved partner-generated events when a local device acts as the job owner/server for one of its event targets.

Example:

- `server1` is the normal target server.
- `dev1.targetServerID == server1`.
- `dev2.targetServerID == server1`.
- `dev1` declares `dev2` as an event target and can generate partner events for `dev2`.
- When connected to `server1`, parsing a shot that reaches `dev1` should allow `server1` to distribute `dev1`'s partner events to `dev2`.
- When connected directly to `dev1`, parsing a shot for `dev1` should also allow `dev1` to act as the server for its declared partner target `dev2`, even though `dev2.targetServerID` is still `server1`.

There is evidence that the current changes fix the direct-device case after a clean CMake configure and rebuild. The early feedback was misleading because the manually tested device was linked against an older library build. In light of that, some later changes may have gone beyond the minimum needed fix and may now be breaking the normal `server1` parsing path.

## Changes Made So Far

### Tests

`test/device/eventengine_missing_partner_tests.cpp` was expanded to cover partner dependency behavior.

Changes include:

- Added a `Distributer<DeviceID, Device>` helper so test devices can see each other in their local collections.
- Added `requireConcreteParse()` to assert that no `Abstract Shot` warning or missing targets were produced.
- Updated the test `PartnerGeneratingDevice` so it can:
  - Generate partner events through `partner(partnerID).addEvent(...)`.
  - Generate direct raw partner events with `DeviceEventParser::addEvent(...)`.
  - Avoid generating partner events when no partner target is configured.
- Added cases for:
  - A connected partner-generated target whose raw `targetServerID` differs from the parsing device.
  - A connected event target declared without `targetServerID`.
  - Direct raw partner events without `targetServerID`.
  - Nested partner-generated targets.
  - Event target declaration without `targetServerID` being normalized to local ownership.

These tests mostly exercise the "device acting as server for its partners" path. They do not yet cover the normal `server1` owner path where partner events are generated downstream and should be routed back through `server1`.

### DeviceID Canonicalization

`LocalEventEngineDependencyParser` gained `findCanonicalDeviceID()`.

Intent:

- Treat `LocalDeviceCollection::getIDs()` as the ground truth for connected devices.
- Match by `DeviceID::getID()` only, because `DeviceID::operator==` and `operator<` ignore `targetServerID`.
- Replace raw event target IDs with the canonical connected device ID when possible.
- For declared local event targets with an empty `targetServerID`, synthesize a local-server ID as a fallback.

This is still a good direction. It addresses the fragile dependence on the raw event target's `targetServerID`.

### Dependency Parser Behavior

`LocalEventEngineDependencyParser::addDeviceEventTargets()` now canonicalizes declared event target IDs before adding them to the dependency tree.

`LocalEventEngineDependencyParser::findTargetServerID()` now uses the canonical ID first.

`LocalEventEngineDependencyParser::addToTargetsByServer()` now:

- Canonicalizes raw target IDs.
- Skips adding targets to `targetsByServer` when the target is already reachable under the local device in the dependency tree.

`LocalEventEngineDependencyParser::getDependants()` now adds local event targets when either:

- The original target list includes the local device.
- The original target list includes a declared event target of the local device.

These changes are intended to let a local device become the acting server for its declared event targets. They should be retested against both the direct-device path and the normal server-owned path.

### Dependency Tree Routing

`EventEngineDependencyTree::getBranchToTarget()` was changed from following only the parent whose `getID()` matches `target.getTargetServerID()` to following any parent edge in the dependency tree.

Old behavior:

- The branch search followed the target-server chain.
- This aligned runtime routing with declared server ownership.

Current behavior:

- The branch search follows any graph parent.
- The dependency tree itself becomes the routing authority.

This is the most questionable change. It may be correct for some dependency-tree construction checks, but it is risky for runtime routing because `LocalEventEngine::addEvent()` and `LocalEventEngine::mergePartnerEvents()` use `getBranchToTarget()` to decide whether events are handled locally, routed to a child branch, or sent upstream.

Possible failure in the normal path:

- `server1` is the job owner.
- The shot initially targets `dev1`.
- `dev1` generates partner events for `dev2`.
- `dev1` declares `dev2` as an event target.
- `dev2.targetServerID == server1`.
- The dependency tree can contain both `server1 -> dev2` and `dev1 -> dev2`.
- When `dev1` receives partner events for `dev2`, the broadened `getBranchToTarget(dev1, dev2)` can return true because of the `dev1 -> dev2` edge.
- `dev1` may classify the `dev2` events as handled under its local branch even though `dev1` is not acting as server for `dev2` in this `server1`-owned parse.
- Those events may fail to propagate upstream to `server1`, breaking the usual server path.

### LocalEventEngine Canonicalization

`LocalEventEngine` gained:

- `findCanonicalDeviceID()`
- `canonicalizeEventTarget()`
- `canonicalizeEventGroupTargets()`
- `copyCanonicalEvents()`

`LocalEventEngine::addEvent()` now canonicalizes concrete event targets before routing.

`LocalEventEngine::mergePartnerEvents()` now canonicalizes the map key and event group targets before routing or merging.

This is conceptually reasonable, but it should be evaluated after the usual-path regression test exists. It may be unnecessary if dependency parsing already canonicalizes enough state, but it is low risk compared with the dependency-tree routing change.

### PartnerDevice

`PartnerDevice::getID()` now returns the connected device's actual ID when a device reference exists, and falls back to the stored partner ID otherwise.

This is reasonable. When a partner is connected, generated events should use the connected device's canonical ID.

### LocalDevice API

`LocalDevice::addEventTarget()` now normalizes event target declarations with an empty `targetServerID`:

```cpp
DeviceID triggerID("FPGA Trigger", getID().getAddress(), 8);
addEventTarget(triggerID, "trigger");
```

is stored like:

```cpp
DeviceID triggerID("FPGA Trigger", getID().getAddress(), 8, getID().getID());
addEventTarget(triggerID, "trigger");
```

`LocalDevice::partner(const DeviceID&)` now returns a `PartnerDevice` using the stored canonical partner ID when the lookup matches by device key.

This API change still matches the desired user-facing behavior. It may not be part of the minimal core routing fix, but it is likely worth keeping if tests confirm it does not interfere with normal parsing.

## Current Concern

The direct-device partner target bug appears fixed after a clean configure and build.

However, there is evidence that the normal `server1` parsing path is now broken. The most likely cause is the changed meaning of `EventEngineDependencyTree::getBranchToTarget()`, because that function is used both for dependency-tree reachability and runtime event routing. Those two use cases have different ownership semantics:

- Dependency construction may need graph reachability independent of raw `targetServerID`.
- Runtime event routing must respect which device is actually acting as server for a target in the current parse.

## Recommended Revert-To-Minimum Plan

### 1. Add the missing usual-path test first

Before reverting or changing production code, add a failing regression for the normal path:

- Create `server1`, `dev1`, and `dev2`.
- Set `dev1.targetServerID == server1`.
- Set `dev2.targetServerID == server1`.
- Make `dev1` declare `dev2` as an event target.
- Create the shot through `server1`'s scheduler.
- The initial shot should target `dev1` only.
- `dev1` should generate partner events for `dev2`.
- `dev2` should receive and play those partner events.
- The parse must be concrete:
  - No `Abstract Shot` warning.
  - No missing target IDs.
- Play must complete:
  - `dev1.playCount == 1`.
  - `dev2.playCount == 1`.
  - `dev2` must have no initial shot event; its event should come only from `dev1` partner generation.

This should reproduce the suspected regression and prevent future fixes from breaking the common server-owned path.

### 2. Keep the tests for direct-device partner ownership

Keep or rework tests for:

- `dev1` as job owner.
- `dev2.targetServerID == server1`.
- `dev1` declares `dev2` as an event target.
- The shot targets `dev1`.
- `dev1` generates partner events for `dev2`.
- `dev1` must act as server for `dev2` during this parse.

Run this case in at least two forms:

- `dev1.addEventTarget(dev2IDWithServer1)`.
- `dev1.addEventTarget(dev2IDWithoutTargetServer)`.

Both should parse and play concretely.

### 3. Revert or split the broad `getBranchToTarget()` behavior

Preferred minimal rollback:

- Restore `EventEngineDependencyTree::getBranchToTarget()` to target-server-chain behavior.
- If graph reachability independent of `targetServerID` is still needed, add a separate helper such as:
  - `hasGraphPathToTarget(root, target)`
  - `getGraphBranchToTarget(root, target, branch)`

Use the graph-path helper only in dependency parser code where the question is "is this node already present somewhere under the local dependency subtree?".

Keep runtime event routing on the target-server-aware path unless the local engine has explicitly determined that it is acting as server for the branch target.

Alternative guarded fix:

- Keep graph-path `getBranchToTarget()`, but update `LocalEventEngine::addEvent()` and `LocalEventEngine::mergePartnerEvents()` so branch routing only handles a target locally when the branch is owned by the local engine:

```cpp
else if (dependencyTree->getBranchToTarget(localDeviceID, eventTargetID, branchID)
         && isActingServerForDevice(branchID)) {
    ...
}
else if (upstreamPartnerEvents != 0) {
    upstreamPartnerEvents->addEvent(...);
}
```

This may be less clean because `getBranchToTarget()` would still have two meanings. Splitting the helpers is easier to reason about.

### 4. Keep canonical ID lookup, but narrow where it is applied if needed

Likely keep:

- `LocalEventEngineDependencyParser::findCanonicalDeviceID()`.
- Canonicalization in `addDeviceEventTargets()`.
- Canonicalization in `addToTargetsByServer()`.

Review after tests:

- `LocalEventEngine::canonicalizeEventGroupTargets()` may be useful, but it is a broader runtime behavior change. Keep it only if the usual-path and direct-device tests both require or tolerate it.
- `LocalEventEngine::findCanonicalDeviceID()` duplicates parser canonicalization. Consider centralizing later, but do not refactor during the bug fix.

### 5. Keep the LocalDevice API cleanup if it remains harmless

The `LocalDevice::addEventTarget()` empty-server normalization matches the desired API. It can stay if both major paths pass:

- Normal `server1` owner path.
- Direct `dev1` acting-server path.

If it causes ambiguity, defer it to a separate API cleanup commit after the core routing fix is stable.

## Test Plan

### Configure and build

Because stale artifacts caused misleading feedback, always configure before validating this issue:

```powershell
conda run --no-capture-output -n sti3-build cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL -DCMAKE_INSTALL_PYTHONDIR=build/Lib/site-packages -S . -B build
conda run --no-capture-output -n sti3-build cmake --build build --parallel 8
```

### Targeted tests

Run the partner dependency test block after adding the usual-path regression:

```powershell
conda run --no-capture-output -n sti3-build ctest --test-dir build -R "partner|target|dependency" --output-on-failure
```

If the test names do not match a useful regex, list tests first:

```powershell
conda run --no-capture-output -n sti3-build ctest --test-dir build -N
```

Then run the exact test range or names.

### Full validation

Run the full suite before accepting the fix:

```powershell
conda run --no-capture-output -n sti3-build ctest --test-dir build --output-on-failure
```

For manual device validation, confirm that the launched process is loading artifacts from the freshly configured `build` tree or from a freshly installed/deployed copy.

## Resolution

The fix was completed by restoring conservative runtime routing semantics and separating dependency-graph reachability from target-server routing.

Final behavior:

- `EventEngineDependencyTree::getBranchToTarget()` again follows the `targetServerID` chain. This keeps runtime routing aligned with the device that actually owns a target during a normal server-owned parse.
- `EventEngineDependencyTree::getGraphBranchToTarget()` was added for graph reachability checks that intentionally ignore `targetServerID`.
- `LocalEventEngineDependencyParser::addToTargetsByServer()` uses graph reachability only when the root parser is allowed to act as server for its declared partner targets. Recursive dependency parser calls keep the target-server route intact, so the normal `server1` path can distribute partner events generated by downstream devices.
- Canonical `DeviceID` lookup remains in place and continues to use `LocalDeviceCollection::getIDs()` as the ground truth for connected devices.
- `LocalDevice::addEventTarget()` can accept a target ID without `targetServerID`; it normalizes that declaration to the local device as a fallback owner.
- `PartnerDevice::getID()` prefers the connected device's canonical ID when the target is connected.

Validation completed:

```powershell
conda run --no-capture-output -n sti3-build cmake --build build --parallel 8
conda run --no-capture-output -n sti3-build ctest --test-dir build -I 161,169 --output-on-failure
conda run --no-capture-output -n sti3-build ctest --test-dir build --output-on-failure
```

Results:

- Partner dependency block passed: `9/9`.
- Full suite passed: `218/218`.
- The new usual-path regression passes: `server1` as job owner, initial shot targets `dev1`, `dev1` generates partner events for `dev2`, and `dev2.targetServerID == server1`.
- The direct-device case passes: `dev1` as job owner can act as server for connected partner targets even when those targets do not declare `dev1` as their `targetServerID`.
- Nested connected partner-generated targets pass in the current test coverage.

## Acceptance Criteria

- Normal server-owned parse works:
  - `server1` is job owner.
  - Initial shot targets `dev1`.
  - `dev1` generates partner events for `dev2`.
  - `dev2.targetServerID == server1`.
  - `dev2` receives and plays the generated events.
  - No abstract shot or missing target warning.

- Direct-device partner parse works:
  - `dev1` is job owner.
  - `dev2.targetServerID == server1`.
  - `dev1` declares `dev2` as an event target.
  - `dev1` acts as server for `dev2` for this parse.
  - No abstract shot or missing target warning.

- Event targets can be declared without `targetServerID`:
  - `LocalDevice::addEventTarget(DeviceID(name, address, module))` works for connected partners.
  - The local device can still resolve the canonical connected ID through `LocalDeviceCollection`.

- Missing partner targets remain abstract:
  - If the event target is declared but not connected and cannot be found through the collection or server chain, parsing remains abstract and play is blocked.

## Open Questions

- Is it acceptable for a dependency tree to contain both `server1 -> dev2` and `dev1 -> dev2` when `server1` is the job owner, or should partner-target edges be omitted when the declared target server is present and owns the target?
- Should "acting server for partners" be represented in the dependency tree with a synthetic local `targetServerID`, or should it be represented only in `LocalEventEngine::isActingServerForDevice()`?
- Should canonicalization live in a shared helper instead of duplicated in `LocalEventEngineDependencyParser` and `LocalEventEngine`?
