# Network NameService maintenance refactor

Date: 2026-06-25

## Background

STI3 hubs register `TDeviceHub.Object` references in the omniORB NameService so
that network connections can recover after a process restart. The current design
also tries to keep the NameService tree clean by pruning bindings that appear to
be dead during normal hub discovery.

This can remove live hub bindings after a transient failure. A client-side
timeout while probing the server hub can cause that client to unbind the server's
hub object from the shared NameService. Once this happens, other devices can no
longer resolve the server hub and reconnect, even though the server process may
still be running.

The cleanup problem is real: stale references can accumulate over weeks when
devices crash or are killed without clean shutdown. STI1 had startup slowdowns
because dead leaves in the NameService tree had to time out one at a time. The
refactor must solve both problems:

- Normal discovery must not be able to globally delete live hubs.
- Stale entries must still be cleaned so server startup and reconnect remain
  bounded over long deployments.

## Current Behavior

Relevant code:

- `NetworkDeviceHub::run()` registers the local hub once with
  `registerHubContext()`.
- `NetworkDeviceHub` starts a 5-second `IntervalTask` that calls
  `refreshHubConnections()` and `connectToTargetHubs()`.
- `refreshHubConnections()` calls
  `ORBManager::getAllLiveObjectContexts(hubContextPath, hubObjectName, liveHubs)`.
- `findHub()` also calls
  `ORBManager::getAllLiveObjectContexts(stiContext, hubObjectName, liveHubs)`.
- `ORBManager::getAllLiveObjectContexts()` constructs a `COSBindingNode` and
  immediately calls `node.prune()`.
- `COSBindingNode::walkBranches()` calls `_non_existent()` on bindings and marks
  leaves dead when it catches `TRANSIENT`, `COMM_FAILURE`, or `TIMEOUT`.
- `COSBindingNode::prune()` calls `context->unbind(...)` for branches it marked
  dead.

This means discovery and cleanup are currently the same operation. Any STI3
process that can access the NameService can remove bindings while attempting to
discover live hubs.

## STI3 Naming Semantics

For a hub `H`, the primary binding is:

```text
STI/H/TDeviceHub.Object
```

This is owned by `H`. It is the public location used to resolve `H` by HubID.

When another hub `P` connects to `H`, `P` also registers its own hub object under
`H`'s context:

```text
STI/H/P/TDeviceHub.Object
```

This child entry is not just an optional hint. It is the persistent reverse
discovery record that lets `H` rediscover `P` after `H` restarts. This is
especially important for the server hub: devices know their target server, so
device restart is naturally robust, but the server does not start with a complete
list of devices. The server's incoming subtree records the hubs that previously
connected to it.

The usual network shape is asymmetric in practice:

- Many device hubs connect to the server hub.
- A normal device hub usually connects only to one server hub.
- Partner device references are generally distributed through device
  collections, not through additional hub target lists.

Therefore stale child bindings are most expensive for the server hub, because
server restart recovery must scan a much larger incoming subtree.

## Design Decisions

### 1. Separate discovery from pruning

Normal discovery must be read-only. It may enumerate candidate hub paths and may
ignore failed candidates locally, but it must not call `unbind()` as a side
effect.

`ORBManager::getAllLiveObjectContexts()` should either become read-only or be
replaced by a clearly named read-only API, for example:

```cpp
void getObjectContexts(const std::string& baseContext,
                       const std::string& objectName,
                       std::vector<std::string>& objContexts);
```

Pruning should move behind a separate API with explicit policy and scope, for
example:

```cpp
void pruneObjectContexts(const std::string& baseContext,
                         const std::string& objectName,
                         const PrunePolicy& policy);
```

Read-only discovery and pruning should run on different schedules and with
different failure thresholds.

### 2. Do not probe liveness while walking the tree

Tree enumeration should avoid `_non_existent()` during the NameService walk.
Use the naming binding type to traverse contexts and collect object leaves named
`TDeviceHub.Object`. Stale object leaves should be cheap candidate strings, not
startup-time timeout traps.

Liveness should be checked later, when attempting to resolve and connect a
candidate hub. That check is still necessary, but it should happen under a
bounded reconnect policy rather than during raw tree traversal.

### 3. Periodically self-rebind every hub

Each live `NetworkDeviceHub` should periodically reassert its own bindings:

- Its primary root binding:
  `STI/<thisHub>/TDeviceHub.Object`
- Its binding under every target hub context.
- Its binding under every currently connected peer hub context.

`NetworkDeviceHub::registerHubContext()` already contains the basic logic for
root and target-hub binding. The refactor should extend this idea so the hub can
periodically rebind using the connected HubIDs already maintained by
`LocalHub`.

This self-rebind is important even if pruning is scoped to each hub's local
context. If hub `A` accidentally prunes `STI/A/B/TDeviceHub.Object`, hub `B`
should repair that registration without requiring a restart.

Self-rebind should be idempotent. `bindObjectReference()` already uses `rebind()`
when an object is already present, so the operation is suitable for periodic
anti-entropy.

Recommended initial interval: 30-60 seconds.

The self-rebind operation does not require a separate task for performance
reasons; periodic `rebind()` should be cheap. A separate logical interval is
recommended because self-rebind has different policy from reconnect and pruning:

- self-rebind should default on
- normal reconnect should be read-only
- pruning should default off and use conservative thresholds

This can still use the existing `TaskScheduler`. Adding a second `IntervalTask`
to that scheduler should not require another OS thread.

### 4. Pruning should be opt-in, conservative, and scoped

Pruning should not be part of normal client/device discovery. It should be an
explicit maintenance task controlled by configuration.

Recommended initial scope:

```text
OwnIncomingSubtree
```

For hub `H`, this means pruning only under:

```text
STI/H/*
```

and not pruning another hub's primary binding at:

```text
STI/P/TDeviceHub.Object
```

This preserves a symmetric design: any hub can opt in to maintaining its own
incoming subtree. In the common lab topology, only the server usually needs this
because the server owns the large incoming subtree.

Recommended config shape:

```ini
[NetworkHub]
SelfRebind = true
SelfRebindIntervalSeconds = 30

EnablePrune = false
PruneScope = OwnIncomingSubtree
PruneIntervalSeconds = 300
PruneFailureThreshold = 3
PruneSuspectSeconds = 600
```

`SelfRebind` should default to true for all hubs. `EnablePrune` should default
to false for all hubs. For the server config, set `EnablePrune = true`. Leave
pruning disabled for ordinary devices unless a topology needs it.

### 5. Use suspect state before unbinding

A single failed probe is not enough evidence to delete a binding.

The pruning task should maintain suspect state keyed by the peer HubID and the
full binding path. STI3 binds `TDeviceHub.Object` references under HubID-derived
paths, so the HubID is the logical object identity used by the library.

Before unbinding, the task should resolve the current path again and attempt to
verify/reconnect to the associated `TDeviceHub.Object`. A successful reconnect
or liveness check clears suspect state. This avoids pruning a binding that has
already recovered or been refreshed by periodic self-rebind.

Recommended policy:

- First failure marks a HubID/path as suspect.
- Additional failures must occur across a real elapsed time window.
- A successful probe clears suspect state.
- Only after both `PruneFailureThreshold` and `PruneSuspectSeconds` are
  satisfied may the binding be unbound.

Example initial values:

- `PruneFailureThreshold = 3`
- `PruneSuspectSeconds = 600`
- `PruneIntervalSeconds = 300`

These values intentionally make cleanup slow. Stale names are less dangerous
than deleting live names.

### 6. Server restart recovery should be bounded

The server still needs to reconnect to the hubs recorded under its own context
before it is fully operational. The refactor should not remove this recovery
behavior.

However, server startup should not perform an unbounded serial timeout over all
stale leaves. The recovery flow should be:

1. Register/rebind the server's own hub context.
2. Enumerate candidate children under the server's own context using read-only
   tree traversal.
3. Attempt to reconnect candidates with bounded per-candidate timeout and a
   total recovery policy.
4. Optionally continue reconnect attempts after the ORB has started serving.
5. Let the pruning task clean stale entries later.

The current `NetworkDeviceHub::run()` registers the hub context, attempts
reconnects, starts the ORB with `orbmanager->run()`, and then blocks with
`orbmanager->block()` if requested. Since `orbmanager->run()` is non-blocking,
server recovery may be able to start serving without extra threads by switching
the order:

1. Register/rebind the local hub context.
2. Start the ORB.
3. Reconnect target and incoming child hubs on the main thread.
4. Block the main thread with `orbmanager->block()`.

This ordering should be verified during implementation. The goal is to let ORB
managed threads handle incoming work while the main thread continues bounded
network recovery.

### 7. Clean shutdown should preserve recovery state

Clean shutdown needs to preserve automatic reconnect.

For hub `H`, shutdown should not destroy the `STI/H` context or its child list,
because that incoming subtree is how `H` recovers previous connections after it
restarts. It may make sense to remove `H`'s own `TDeviceHub.Object` binding and
to remove `H`'s self-registration entries under currently connected peer
contexts, since those self bindings will be rebound on startup and by periodic
self-rebind.

This area needs careful implementation review before adding
`unregisterHubContext()` behavior.

## Proposed Implementation Phases

### Phase 1: Stop destructive discovery

- Split read-only tree enumeration from pruning.
- Ensure `findHub()`, `refreshHubConnections()`, `connectToTargetHubs()`, and
  `printNetwork()` do not prune as a side effect.
- Preserve existing reconnect behavior as much as possible, but allow stale
  candidates to fail locally without deleting NameService entries.

### Phase 2: Add periodic self-rebind

- Add periodic self-rebind to `NetworkDeviceHub`, preferably as a separate
  `IntervalTask` on the existing scheduler.
- Rebind:
  - root context for this hub
  - configured target hub contexts
  - currently connected peer hub contexts
- Keep this independent from normal refresh/reconnect.
- Add configuration for interval and enable/disable.

### Phase 3: Add explicit pruning task

- Add pruning configuration and default it off.
- Implement `OwnIncomingSubtree` pruning.
- Add suspect tracking with repeated failures and elapsed-time threshold.
- Verify the suspect HubID/path remains unreachable before unbinding.
- Enable this in the server config after testing.

### Phase 4: Improve recovery diagnostics and bounds

- Add logging around:
  - number of candidate child hub bindings found
  - number successfully reconnected
  - number skipped or timed out
  - current prune suspect counts
- Add a configurable startup recovery deadline or diagnostic warning if recovery
  is still incomplete.

## Testing Plan

Add integration coverage with a spawned omniNames instance where possible.

Important cases:

1. **Read-only discovery does not unbind**
   - Create a stale/dead binding.
   - Run normal hub discovery from another process.
   - Verify the binding is still present in the NameService.

2. **Self-rebind repairs accidental deletion**
   - Start a hub and verify its root binding exists.
   - Delete or overwrite the binding through a test helper.
   - Wait for the self-rebind interval.
   - Verify the binding resolves again without restarting the owner hub.

3. **Server incoming subtree recovery**
   - Start server and multiple devices.
   - Stop and restart the server while devices remain alive.
   - Verify the server enumerates its incoming subtree and reconnects live device
     hubs.

4. **Pruning is conservative**
   - Create a stale child binding under the server context.
   - Verify it is not removed after one failed probe.
   - Verify it is removed only after the configured failure count and elapsed
     suspect window.

5. **Recovered binding is not pruned**
   - Mark a HubID/path suspect.
   - Let the owner self-rebind or otherwise make the hub reachable again.
   - Verify successful liveness clears suspect state and pruning does not remove
     the recovered binding.

## Resolved Decisions

- `SelfRebind` should default to true for all hubs.
- `EnablePrune` should default to false for all hubs.
- The server example config should opt in with `EnablePrune = true`.
- Server reconnect/recovery should be structured so the ORB can start serving
  before the main thread blocks, likely by moving `orbmanager->run()` before
  `connectToTargetHubs()` and incoming child reconnects.
- Prune suspect state should be keyed by HubID/path, with reconnect/liveness
  attempts used to verify that the HubID is still unreachable before unbinding.

## Open Questions

- What exact shutdown bindings should `unregisterHubContext()` remove while
  preserving the incoming child list needed for server restart recovery?
- Should server startup wait for an initial bounded recovery pass before
  accepting normal sequence work, or should the server expose a degraded/recovery
  state while reconnect continues?
- What logging and user-facing diagnostics should report incomplete reconnect
  recovery or prune suspect state?
