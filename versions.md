# STI3 Versions

This file records the versioning policy for STI3 and a short release history.
The source of truth for the package version used by CMake, conda, and the
library build is `sti3_version.json`.

## Versioning Policy

STI3 uses versions in the form:

```text
3.feature.patch
```

The leading `3` is the STI generation and compatibility line. It is similar to
the "Python 3" line: normal STI3 releases should keep the `3` prefix unless a
future STI4 generation is intentionally created.

Use the second number for user-visible feature releases:

```text
3.1.0
3.2.0
3.3.0
```

Increment the patch number for fixes that do not add a new public capability:

```text
3.1.1
3.1.2
```

Use the conda `build_number` for packaging rebuilds of the same source version.
When the source version changes, reset `build_number` to `0`.

Examples:

* `3.1.0`, build `0`: first package for a new STI3 feature release.
* `3.1.0`, build `1`: rebuilt package for the same source version, such as a
  packaging metadata fix.
* `3.1.1`, build `0`: source change containing bug fixes only.
* `3.2.0`, build `0`: source change adding another user-facing feature.

Each conda package uploaded to the cloud should have a unique version/build
combination. For source-code changes, prefer changing the version number rather
than only incrementing the conda build number.

## Release History

### 3.6.0 - Asynchronous post-shot post-processing

Feature release.  Adds asynchronous post-shot post-processing so a device can
run analysis code after a shot plays without blocking the parsing or playback of
later shots.  Additional 3.6.0 features are recorded alongside this entry.

Features:

* Add a `PostProcessingManager` device subsystem, with a
  `LocalPostProcessingManager` that runs registered analysis targets on a single
  per-device background worker thread (started lazily on the first target).
* Add `LocalDevice::addPostProcessingTarget()` for registering named targets and
  their callbacks, and `Device::getPostProcessingManager()` so local and remote
  device references can reach a device's targets.
* Add the `PostProcessTarget` and `PostProcessRequest` engine types and a
  non-hard-timed post-processing request side-list on `RawEventGroup`, parallel
  to (not part of) the hard-timed event table.
* Resolve post-processing targets during parsing and add them to the shot's
  dependency tree (a second, additive `getDependants` pass over the side-list, on
  the same tree the hard-timed event targets use). A target that cannot be found
  produces a non-fatal "Missing post-processing target" parse warning and is
  skipped, instead of making the shot abstract and blocking playback.
* Store the resolved requests on the job owner's `LocalEventEngine` (not on the
  scheduler) and dispatch them at the end of that engine's play, after the shot
  result is persisted, off the play-critical handshake.
* Route dispatch along the dependency tree (`distributePostProcessing`): requests
  for the owner or its directly-owned devices are delivered immediately; the rest
  are forwarded, as per-branch sublists, to the owned branch that leads to each
  target, repeating at each hop to arbitrary depth. This reaches analysis devices
  nested behind sub-servers, which the original owner-collection-only dispatch
  could not.
* Pull the shot's `ShotResult` on the target's worker thread (resolving the owner
  device's `PersistenceManager`, self or a partner reference) and hand it to the
  callback, whose signature is now `MetaData(const shared_ptr<ShotResult>&, const
  MetaData&)`. Two distinct `Failed` completion messages separate "owner not
  reachable (declare it with `addPartner`)" from "shot result not found on owner".
* Broadcast a `PostProcessingCompleteMessage` with the results, or an error
  message when the routine raises or the result is unavailable, so other devices
  and clients can subscribe to completed analyses.

Network:

* Carry the post-processing request side-list across CORBA in the
  `TRawEventGroup` structure and `Convert_RawEventGroup` conversion.
* Add a `TPostProcessingManager` interface and a
  `TDevice::getPostProcessingManager()` accessor, a `RemotePostProcessingManager`
  proxy, and a `TPostProcessingManager_i` servant.
* Add a `distributePostProcessing` RPC on `TEventEngineScheduler` (with
  `RemoteEventEngineScheduler` marshalling and `TEventEngineScheduler_i` servant)
  so the tree-routed dispatch forwards per-branch request sublists, plus the
  dependency tree, to a branch device's scheduler at each hop.
* Add a `TPostProcessingCompleteMessage` structure and device-message conversion
  so completion messages relay over the network.

Python and examples:

* Add `postTarget()` and `postProcess()` STIPy helpers for declaring
  post-processing requests in timing files, with global, shot, and group forms,
  a dictionary of options, and no time argument.
* Add `LocalDevice.addPostProcessingTarget()` for registering targets backed by a
  Python callable, and `Device.getPostProcessingTargets()` for discovering the
  targets a local or connected device offers. The callable now receives the pulled
  `ShotResult` (`def fit(shot_result, options): ...`) instead of a `ShotID`.
* Wrap `PostProcessTarget` and `PostProcessRequest`, and expose
  `RawEventGroup.postProcessRequests()` for inspecting a shot's requests.
* Update the committed C++ and Python `postProcess` example devices to the pulled
  `ShotResult` callback, dropping the manual `getPersistenceManager()`/
  `getShotResult()` lookup (and noting `addPartner(owner)` for cross-device
  analysis).

Fixes:

* Fix the templated `MetaData::addMetaData<T>` overload to store the converted
  value instead of reinterpreting the original argument, avoiding a spurious
  "Unsupported type" message and incorrect storage for non-`MixedValue` values.

Documentation:

* Document the post-processing manager and target registration in the device
  library guide, and the `postTarget()`/`postProcess()` and discovery API in the
  STIPy guide.

Tests:

* Add C++ coverage for `PostProcessTarget`, `LocalPostProcessingManager` success,
  exception, and the two abort paths (owner unreachable / shot result not found,
  each driven through a backing `PersistenceManager` that returns a `ShotResult`),
  and event-engine integration covering a resolvable target, a two-level hierarchy
  where the analysis device is reachable only via a sub-server, the dependency
  tree extended with post-process-only nodes, a missing target warning that still
  plays, and a regression that a missing hard-timed device still hard-errors.
* Add network conversion round-trip coverage for the post-processing request
  side-list and the completion message.

### 3.5.5 - File-backed makeshot performance

Patch release for a performance regression in file-backed STIPy shot creation.

Fixes:

* Avoid scanning every loaded Python module after every import during
  file-backed `stipy.makeshot(...)` and `server.makeshot(...)` execution.
  Import cleanup now removes stale timing modules at context entry and checks
  only modules added during the shot at context exit.
* Cache protected import roots within each file-backed shot isolation context so
  module path classification avoids repeated environment-root resolution.
* Build STIPy stack traces by walking Python frames directly instead of using
  `inspect.stack()` and `inspect.getframeinfo()` for every variable, event, and
  measurement call in a timing file.
* Avoid resolved path classification for protected Python environment modules
  during file-backed import cleanup, keeping `makeshot()` latency stable in
  Jupyter kernels with many loaded modules.

Tests:

* Add regression coverage that verifies file-backed import tracking does not
  scale as a full `sys.modules` scan per import.
* Add regression coverage that fails if file-backed `makeshot()` returns to the
  expensive `inspect.stack()` path.
* Add regression coverage for protected-module classification without resolved
  path scans.

### 3.5.4 - STIPy makeshot fixes

Patch release for deterministic file-backed shot generation and clearer STIPy
variable declaration errors.

Fixes:

* Isolate imports for file-backed `stipy.makeshot(...)` and
  `server.makeshot(...)` calls so helper timing modules are re-executed for each
  shot instead of being reused from `sys.modules`.
* Pick up edited helper timing files on the next file-backed `makeshot()` call
  without restarting Python.
* Execute submitted timing files under private module names and remove timing
  modules from `sys.modules` after shot creation so timing-file globals do not
  leak into later shots or the caller's namespace.
* Raise a Python `ValueError` when `setvar(...)` declares the same variable name
  more than once in a shot, instead of silently keeping the first value.
* Preserve the intended `makeshot(..., vars={...})` override behavior for the
  first `setvar(...)` declaration of an overridden variable.

Python API:

* Add the optional `import_roots` keyword for file-backed local and server
  `makeshot()` calls so helper timing modules outside the submitted file's
  directory can be refreshed with the shot.

Documentation:

* Document file-backed `makeshot()` import isolation and `import_roots` usage.
* Add design notes for the STIPy file-backed import isolation behavior.

Tests:

* Add STIPy integration coverage for repeated file-backed shots, edited helper
  modules, stale pre-imported helpers, timing-module cleanup, private main-file
  globals, extra import roots, and server-backed file shots.
* Add STIPy regression coverage for duplicate `setvar(...)` declarations and
  preserved variable override behavior.

### 3.5.3 - Python image helpers and Windows build fixes

Patch release for constructing, inspecting, and round-tripping STIPy images from
Python, and for restoring the Windows build after recent scheduler and network
conversion changes.

Python and examples:

* Export `STI_Image` from `stipy` star imports as the preferred Python image
  alias, while avoiding a collision with Pillow's `Image` symbol.
* Add `STI_Image.from_file()`, `STI_Image.from_path()`, and
  `STI_Image.from_pil()` helpers for constructing STIPy images from Python files
  and Pillow images.
* Add `STI_Image.to_pil()` for converting readable STIPy image payloads back to
  Pillow images when Pillow is installed.
* Expose Python image metadata helpers and preserve source, storage, format, and
  encoding metadata on constructed images.
* Extend the Python `readWrite` example and notebooks with STIPy image
  construction and round-trip examples.

Fixes:

* Avoid Windows `min`/`max` macro expansion when clamping CORBA sequence
  lengths in network conversion.
* Keep event-engine implementation headers out of `LocalEventEngineScheduler.h`
  so `STIServer` does not require private Boost.Graph include paths.
* Make `LocalEventEngineFactory.h` include the concrete engine type it
  constructs instead of relying on scheduler-header transitive includes.
* Preserve arbitrary Python bytes exactly when creating `BinaryData` from
  `bytes`, including payloads with embedded null bytes.
* Initialize default image dimensions consistently and preserve the complete
  `FileID` when constructing images from file holders.

Tests:

* Add STIPy integration coverage for `STI_Image` exports, file-backed image
  construction, file-holder-backed image IDs, Pillow construction, and Pillow
  round-trip conversion.

### 3.5.2 - Add-node network refresh

Patch release for pruning stale device references opportunistically when new
devices join a hub.

Fixes:

* Refresh the LocalHub network after a successful `addNode()` distribution so
  dead device references are removed without requiring a manual refresh call or
  a periodic device-ping task.

Tests:

* Add LocalDeviceHub coverage for add-node-triggered network refresh and stale
  reference cleanup across connected hubs.

### 3.5.1 - File-backed payload materialization

Patch release for materializing lazy file-backed payloads from Python clients
and avoiding process-working-directory files for generated read measurements.

Features:

* Expose `PersistenceManager.getFileServer()` through STIPy so Python clients
  can transfer `FileID`-backed payloads through the device file server.
* Add virtual file transfer support to the public file-server path, including
  Python access to virtual file holders and virtual file servers.
* Add `PersistenceManager.getBasePath()` and `getTemporaryPath()` to the C++,
  CORBA, remote persistence, and STIPy APIs.
* Back `getTemporaryPath()` with the transient repository cache directory so
  device read methods can create temporary file-backed measurements without
  polluting the device base path.
* Extend the Python `readWrite` example with file-backed image, plain file,
  plain binary, and virtual-file measurement channels.
* Update the lazy image notebook with file-backed image and virtual file
  transfer examples.

Fixes:

* Materialize FileID-backed images through the exposed file server instead of
  requiring inline image data or a cached `FileHolder`.
* Preserve lazy `BinaryData` stream ownership during network conversion so
  remote binary reads do not crash the server process.
* Return failure from unsupported remote `FileServer.addFile()` calls and make
  file-server add semantics explicit through the abstract interface.

Tests:

* Add C++ coverage for local and virtual file-server registration, transfer,
  image file-server writes, and measurement file handling.
* Add STIPy integration coverage for file-backed image reads, virtual file
  transfer, temporary path access, remote persistence paths, and binary channel
  reads through an STIServer proxy.
* Add network conversion regression coverage for lazy binary and image payload
  ownership.

### 3.5.0 - Lazy channel measurement payloads

Feature release for live channel measurement state, lazy transfer of heavy
channel payloads, Python image access, and scheduler/network reliability
improvements.

Features:

* Add channel `lastMeasurement` state to the C++ channel API, device update
  messages, remote channel snapshots, remote channel caches, and STIPy channel
  wrappers.
* Send heavy `BinaryData` and binary-backed `Image` channel measurements as
  lazy stream-backed references in channel update messages and channel
  snapshots.
* Preserve `BinaryData` metadata for stream-backed payloads, including element
  length, total byte count, and word size, before the bytes are materialized.
* Add public `BinaryData` stream-query and materialization APIs so clients can
  inspect lightweight metadata and explicitly pull binary bytes when needed.
* Return heavy `BinaryData` and binary-backed `Image` read results as lazy
  references so clients can inspect size metadata before pulling bytes.
* Pull lazy binary and image measurements during local shot-result collection so
  completed `ShotResult` measurements reference server-local archived data.
* Add `Normal` and `Interleaved` sequence scheduling modes, including the
  `EventScheduler` `Sequence Mode` server configuration option.

Python and examples:

* Expose explicit STIPy accessors for binary and image mixed values, including
  `getBinary()`, `getImage()`, `BinaryData.pull()`, `BinaryData.getBytes()`,
  and save helpers.
* Return `BinaryData` and `Image` wrapper objects from direct STIPy reads so
  Python clients can inspect lazy metadata before pulling payload bytes.
* Support constructing STIPy `BinaryData` from Python bytes and `Image` from
  bytes, `BinaryData`, `FileHolder`, or `FileID`.
* Add `FileHolder.writeBytes()` for Python device code that creates file-backed
  measurements.
* Extend the C++ and Python `readWrite` examples with 10 by 10 random image
  input channels backed by `BinaryData` and `FileHolder`.
* Add a Python notebook for reading the new image channels and materializing
  lazy image data on demand.

Fixes:

* Fix binary stream conversion so successful eager stream transfer reports
  success and preserves `wordsize`.
* Keep rapid log entries on separate output lines.
* Resynchronize hub nodes when network devices reconnect.
* Remove the unreleased public `ChannelState.h` header in favor of the channel
  API and network-owned conversion details.

Documentation:

* Document lazy binary and image payload behavior for C++ and Python device
  clients.
* Add planning notes for channel `lastMeasurement` state and heavy channel
  measurement streaming.

Tests:

* Add focused C++ coverage for binary stream conversion, lazy binary metadata,
  lazy image writes, channel update and snapshot conversion, remote channel
  cache behavior, and shot-result archival of lazy payloads.
* Add STIPy runtime coverage for explicitly pulling lazy binary channel
  measurements.
* Add scheduler tests for sequence scheduling modes and logger tests for rapid
  log-entry formatting.
* Add partner distribution reconnect integration coverage and mark the covered
  reconnect cases as passing.

### 3.4.2 - Configured device metadata

Patch release for allowing device metadata to be configured without recompiling
device code.

Features:

* Add an optional `Metadata` configuration section for `LocalDevice`.
* Apply configured `Color`, `Description`, and `Help` entries through the
  existing `LocalDevice` metadata helper methods.
* Store arbitrary key-value pairs from the `Metadata` section as device
  metadata.
* Support nested metadata sections when constructing a device from a sectioned
  config, such as `DeviceName.Metadata`.

Tests:

* Add regression coverage for direct and sectioned device metadata config.

### 3.4.1 - Result job ownership

Patch release for recording which device acted as the server for parse and play
jobs without opening a new feature-release line.

Features:

* Add `jobOwner` to `ParseResult` and `ShotResult` so saved and returned
  results identify the `DeviceID` that owned the parse or play job.
* Populate result job ownership from the `EventEngineJob` running in the local
  event engine.
* Transport result job ownership through the CORBA `TParseResult` and
  `TShotResult` API structures.
* Expose `ParseResult.jobOwner` and `ShotResult.jobOwner` through STIPy.
* Include shot job ownership in legacy experiment XML output and reload it when
  reading legacy XML.

Compatibility:

* Keep older serialized parse and shot result XML readable when the `jobOwner`
  field is absent.

Tests:

* Add regression coverage for scheduler-generated result ownership,
  serialized-result round trips, backward-compatible result loading, and network
  conversion round trips.

### 3.4.0 - Device metadata support

Feature release for attaching metadata directly to STI devices.

Features:

* Add `Device::getMetaData()` and keyed `Device::getMetaData()` accessors for
  device-level metadata.
* Store device metadata on `LocalDevice` using the existing `MetaData` utility,
  including helper methods for color, description, and help text.
* Transport device metadata through the CORBA device interface so remote devices
  can expose the same metadata as local devices.
* Expose device metadata access and local-device metadata registration through
  STIPy.

Tests:

* Add regression coverage for `LocalDevice` metadata storage and retrieval.
* Update test device stubs for the expanded `Device` API.

### 3.3.0 - Robust event-engine playback

Feature release for making event-engine playback recover cleanly when required
devices disappear or stop responding after parse.

Features:

* Add configurable playback wait settings in the `EngineManager` config section:
  `PlayReady Timeout ms`, `Trigger Timeout ms`, and `PlayComplete Grace ms`.
* Apply configured playback wait values to local event engines created by the
  scheduler, including engines recreated after a factory replacement.

Fixes:

* Validate owned target engines immediately before play so parsed shots are
  canceled if a required device is missing or no longer parsed.
* Bound server waits for owned-device `PlayReady`, trigger arming, and
  `PlayComplete` messages so playback jobs do not hang indefinitely.
* Report timeout failures as stable play errors with the affected device IDs and
  observed engine states.
* Stop the owned-device subtree when a child times out during playback.

Tests:

* Add regression coverage for missing parsed targets, missing `PlayReady`,
  missing trigger arming, missing `PlayComplete`, and bounded master-trigger
  arm waits.

### 3.2.0 - Event engine last-result access

Feature release for retrieving recent event-engine results and tightening local
hub distribution behavior.

Features:

* Add `EventEngineScheduler::getLastParseResult()` and
  `EventEngineScheduler::getLastShotResult()` so callers can retrieve the most
  recent parse or shot result for a specific engine ID.
* Implement last-result lookup for local event engines and schedulers, including
  access to the newest buffered shot result.
* Expose last-result lookup through the CORBA scheduler interface and remote
  scheduler implementation.
* Add STIPy bindings for the new scheduler last-result APIs.

Fixes:

* Avoid duplicate local collection add events when `LocalHub::addNode()` forwards
  a newly added node to connected hubs.

Tests:

* Add regression coverage for scheduler last-result lookup and duplicate
  `LocalDeviceHub` add-event prevention.

### 3.1.4 - Partner dependency routing fix

Patch release for LocalEventEngine partner-event dependency routing.

Fixes:

* Resolve partner event targets by canonical device identity so partner devices
  can be found even when the raw event target omits `targetServerID` or names a
  different server.
* Allow a device acting as a job owner to route events to its declared partner
  devices without requiring those partners to declare the device as their target
  server.
* Preserve the normal server-owned parse/play path where a shared server owns
  both the source device and partner target devices.
* Add regression coverage for missing partners, direct device-as-server partner
  routing, nested partner routing, and normal server-owned partner routing.

### 3.1.2 - STIPy makeshot API alignment

Patch release for STIPy shot-construction API cleanup.

Fixes:

* Add a package-level `stipy.makeshot()` wrapper that supports the same source
  forms as `STIPyServer.makeshot()`: empty shots, callables, and Python timing
  files.
* Support variable overrides for both callable and filename-based shot creation
  in the global and server-based `makeshot` paths.
* Remove the ambiguous global string-name overload from the low-level pybind
  `makeshot` API so strings consistently mean Python timing filenames in the
  public package-level wrapper.
* Document global and server-based `makeshot` usage, including `vars` and
  `shot_type`.

### 3.1.1 - Shot result and measurement fixes

Patch release for fixes and integration follow-up after the initial
VersionManager support.

Fixes:

* Clear placeholder build strings from CMake-generated version metadata.
* Cancel stopped event-engine play jobs cleanly and persist shot status through
  local, network, XML, and Python result paths.
* Include the date when printing `ShotID` values.
* Transfer file and binary measurement attachments through result collection,
  persistence, XML output, and Python bindings.

Version reporting:

* Register STI library version information in the TestDevice and
  EventCheckingDevice examples.
* Collect per-device version information in `ShotResult` records and legacy XML
  output.

### 3.1.0 - VersionManager support

Planned first release under this versioning policy.

This release was followed by `3.1.1`, which completes the version-reporting
integration and fixes shot-result persistence details without introducing a new
feature-release line.

Features:

* Add a single version source in `sti3_version.json` for CMake, conda, setup.py,
  and the STI library build.
* Add `VersionInfo` and `VersionManager` to the C++ device API.
* Add `Device::getVersionManager()` so local and remote device references can
  report version information.
* Add `LocalDevice::addVersionInfo()` so device implementations can register
  driver-, hardware-, or firmware-specific version records.
* Add network support for querying version information from remote devices.
* Add Python bindings for `VersionInfo`, `VersionManager`, and device version
  queries.
* Add `stipy.__version__`, `stipy.version()`, and `stipy.printVersion()`.
* Add tests and documentation for version reporting.

### 3.0.1 - Historical STI3 baseline

Pre-policy STI3 package version used during recent development. Detailed
release notes were not maintained for this period, so treat this as the
baseline before formal version tracking began.
