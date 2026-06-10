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
