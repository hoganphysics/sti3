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
