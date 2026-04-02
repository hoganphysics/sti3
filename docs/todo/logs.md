## sti log infrastructure

* The sti library supports logging by devices. There are serveral related classes:
  * LogFile.h
  * LogFileFilter.h
  * Logger.h
  * LogID.h
  * LogManager.h and LocalLogManager.h
  * LogRecord.h
* The logging part of the library has two goals:
  * Easily allow devices to write to local log files using a simple, flexible API
  * Allow for log access over the sti network:
    * The sti server should be able to pull local logs from other devices to help with archieving
    * The python wrapper library should allow for easy opening of logs of remote devices for inspection
    * The front end (Jupyter extension) should allow inspection of availability logs on devices and the opening of log files as Jupyterlab documents.

### Upgrading the device library for better log file management

* The logging features already support robust options to allow devices to write to local log files. Let's assume that the generation of log files is already mostly working.
* The main area where more work is need is in getting access to these logs over the sti network.
* The LogFile class is particularly underdeveloped. There some a partially implemented idea of LogFileType that would allow string vs file log types. String logs would be sent over the network as simple raw data, while file type logs would be transfered using a more sophistocated file transfer that could support lazy loading, for example. But I don't think this concept has ever been realized in the library.
* There are also some commented notes in LogFile that suggest an alternative design:
  * In this approach, all LogFiles are essentially files, with a FileID and a STI::Utils::FileHolder.
  * This eliminates the need of a string type, which means there is no need to specify the LogFileType at all and it could be dropped.
  * The STI::Utils::FileHolder concept is already well developed elsewhere in the library, and allows for byte-level transfer of files over the sti network using the existing corba rpc library. This is used to transfer certain MixedValue types, for example.
  * For remote log files, this design would allow a natural place to store a reference to a RemoteFileServer to allow the transfer of files from a remote device to the calling device. The calling device could be the stipy python api or the Jupyterlab extension python server. The FileServer concept is already used by the ResultsCollector system (for example LocalResultsCollector) to pull remote MixedValue objects that include FileHolders over the network during event playback.
* The goal is to be able to connect to a device over stipy in python, access the remote device's LogManager, and inspect the available recent log records on the device. 
* It should be possible to "open" a LogFile in python. This should involve transfering some fraction of the file (lazy loading) or the full file as needed. The connection to the remote FileHolder must be maintained as long as the python user has a handle to the remote LogFile so more data can be pulled (in the case of lazy loading).  This assumes that log files can be very long, so that the default behavior should only pull the most recent part of the file. For short log files, it could make sense to just pull the full log (which was the orginal idea behind the string LogFileType), but this could also be implemented using the FileHolder concept.
* In addition to the design of LogFile, part of the upgrade should address the python wrapper api. We want a simple text-based log browser to allow paging through of logs as they are lazy loading. The main use for this would be in Jupyterlab notebooks and possibly the python shell if possible. I'm imaging something as simple as `less` in linux, or the equivalent. There may be existing file preview libraries we can use here, in which case we should be mindful about the design of the LogFile system to support the use with these text previewers.
* The test case for this work will be primarily a jupyterlab notebook.  We want to generate some logs on a device, then connect to the device in jupyterlab, access it's LogManager, and open some LogFiles to inspect.

### Design snapshot before implementation

This section records the current design decisions for the log system upgrade before code changes begin.

#### Public API vs internal local repository

* `LocalLogManager` remains the public-facing API for logging and log access.
* A new internal class, tentatively `LocalLogRepository`, should own all local filesystem and local metadata access:
  * open local log files
  * update log metadata after successful writes
  * read daily `LogRecordFile` data
  * read and update a device-level `LogCatalogFile`
  * answer local queries for names, counts, IDs, and `LogFile` handles without RPC
* `LocalLogRepository` is internal only and should not be exposed to users.
* `RemoteLogManager` remains the RPC proxy that delegates to the remote device's local manager.

#### Query scope

* The existing `LogManager` methods should remain in the API, but their meaning should become local-only:
  * `getLogNames`
  * `getLogCount`
  * `getLogIDs`
  * `getLog`
  * `getLogs`
  * `getLogRecord`
* `LocalLogManager` should also add explicit network-scoped methods such as:
  * `getNetworkLogNames`
  * `getNetworkLogCount`
  * `getNetworkLogIDs`
  * `getNetworkLogs`
  * possibly `getNetworkLogRecord`
* The network-scoped methods should aggregate results by delegating to devices currently present in the local device's collection.
* Initial behavior should be direct collection traversal, not arbitrary recursive graph traversal.

#### Local metadata ownership

* The in-memory `loggers` map should continue to represent live writer objects only.
* The in-memory `loggers` map should not be used as the source of truth for browsing saved logs.
* Persistent log metadata should become the authoritative source for log name discovery and local log enumeration.
* Daily log availability should continue to live in a date-specific `LogRecordFile`.
* A device-level `LogCatalogFile` should live in the device's `logs/` directory, outside the date tree.
* Example layout:

```text
logs/
  logCatalog.ini
  2023/10/31/
    logRecord.ini
    <device-id>/
      sti_0.log
      sti_named_0.log
```

#### Role of `LogRecordStatus`

* `LogRecordStatus` should describe log availability/query state for a device on a given day.
* It should not be used to represent central repo collection state.
* If the server later needs to track whether remote logs have been imported or archived, that should live in a separate collector-side record.
* For a purely local source-device `LogRecordFile`, the common states will likely be `LogsPresent` and `NoLogs`.
* `Unqueried` and `Error` remain useful for aggregate network snapshots such as a future `getNetworkLogRecord(date)`.

#### `LogFile` direction

* All logs should be treated as file-backed logs.
* The old `String` vs `FileHolder` split should be removed from the design.
* The remote-open path should rely on file metadata plus the existing `FileHolder` / `FileServer` transfer system.
* Small-log eager transfer can still be an optimization, but it should not require a separate logical log type.

#### Metadata update strategy

* Metadata should be updated after a successful local file write, not before.
* The write path should update:
  1. the local log file itself
  2. the day's `LogRecordFile`
  3. the device-level `LogCatalogFile`
* `LogRecordFile` should be treated as the authoritative day-level record.
* `LogCatalogFile` should be treated as a rebuildable device-level summary/cache derived from the daily records.

#### Why `LogRecord` needs richer metadata

The current `LogRecord` only tracks a set of log names per device/day. That is enough to answer "does a log name exist on this day?", but it is not enough to:

* resolve `LogID` values without rescanning the directory
* know how many rolled files exist for a log
* know the size of a log before opening it remotely
* support lazy loading of the last N lines without rescanning every file
* support UI display of file counts, byte counts, line counts, and recent update information

For that reason, `LogRecord` should be upgraded from:

* `deviceID -> { status, set<logName> }`

to:

* `deviceID -> { status, map<logName, LogNameRecord> }`

#### Proposed structs

These are sketches only. They are intended to capture the shape of the data before implementation.

```cpp
struct LogFileRecord
{
    LogID id;                          // device/date/logName/index
    STI::Utils::FileID fileID;         // direct handle for local/remote open

    uint64_t bytes = 0;                // file size in bytes
    uint64_t lineCount = 0;            // supports tail/lazy browsing by lines

    STI::Utils::TimeStamp firstEntryTime;
    STI::Utils::TimeStamp lastEntryTime;
};


struct LogNameRecord
{
    std::string logName;

    // Keyed by rolling log file index so LogIDs can be resolved without
    // rescanning the directory.
    std::map<unsigned, LogFileRecord> files;

    uint64_t totalBytes = 0;
    uint64_t totalLines = 0;

    unsigned nextIndex = 0;            // next file index to allocate when rolling
    STI::Utils::TimeStamp lastUpdate;
};


struct DeviceLogRecord
{
    std::string deviceID;
    LogRecordStatus status;

    // Replaces the old std::set<std::string> logNames
    std::map<std::string, LogNameRecord> logs;
};


class LogRecord
{
public:
    STI::Utils::TimeStamp timeStamp;   // the day represented by this record

    // Normally local-source records contain one device entry, but a future
    // aggregate network snapshot API can still use this same shape.
    std::map<std::string, DeviceLogRecord> deviceLogRecords;
};
```

#### Proposed device-level catalog

The device-level catalog is not the authoritative day-level record. Its purpose is to make persistent log name discovery and high-level browsing fast and stable.

```cpp
struct LogCatalogEntry
{
    std::string logName;

    STI::Utils::TimeStamp firstDay;
    STI::Utils::TimeStamp lastDay;

    uint64_t dayCount = 0;
    uint64_t fileCount = 0;
    uint64_t totalBytes = 0;
    uint64_t totalLines = 0;
};


class LogCatalog
{
public:
    std::string deviceID;
    STI::Utils::TimeStamp lastUpdate;

    std::map<std::string, LogCatalogEntry> logs;
};
```

#### How these records should be used

* `getLogNames()` should use `LogCatalogFile` as the fast local source of persistent names.
* If `LogCatalogFile` is missing or stale, it should be rebuilt from the daily `LogRecordFile`s.
* `getLogCount()` and `getLogIDs()` should use the daily `LogRecordFile` metadata instead of the in-memory `loggers` map.
* `LogNameRecord.files` should let the system resolve available file indices and file sizes without rescanning the filesystem in normal operation.
* `LogFileRecord.lineCount` should support default remote-open behavior such as "open the last part of the log" and later paging backward.
* A future remote-open implementation should be able to construct a remote `LogFile` directly from stored metadata plus a remote file transfer handle.

#### Non-goals for this phase

* Do not persist central archive/import status into the source device's local `LogRecord`.
* Do not expose `LocalLogRepository` directly to user code.
* Do not require recursive network crawling for the first pass of network log aggregation.

## Network / IDL Implementation Snapshot

This section is the implementation snapshot for the next pass: updating the abstract log API, the IDLs, and the network library so the new device-side log metadata and explicit network aggregation API work over RPC.

### 1. Public and abstract API changes

The network pass should start by reconciling the public abstract interfaces with the device-side behavior that now exists.

#### `include/sti/device/LogManager.h`

This abstract interface should be updated so the explicit network aggregation API is available through any `LogManager`, including `RemoteLogManager`.

Required additions:

* `getNetworkLogNames`
* `getNetworkLogCount`
* `getNetworkLogIDs`
* `getNetworkLogs`

Optional for a later pass:

* `getNetworkLogRecord`

Why this is deferred in the first pass:

* The first pass only needs enough RPC surface to discover logs and open them remotely:
  * names
  * counts
  * IDs
  * `LogFile` access
* `getNetworkLogRecord(date)` is not a direct source-device record. It would be a synthetic merged snapshot assembled from multiple devices.
* That merged snapshot forces extra decisions that the first pass does not need:
  * direct-collection vs recursive scope
  * how to represent timeout / missing-device / partial-query cases
  * whether a device with no logs should appear explicitly as `NoLogs` or be omitted
  * whether the aggregate record is transient only or ever persisted
  * what deterministic merge / ordering rules should be used
* Deferring it keeps the first pass focused on the remote-open path without prematurely locking down aggregate-record semantics.

Advice for a future pass:

* Treat `getNetworkLogRecord(date)` as a transient query result, not as a persisted source-device record.
* Keep `LogRecordStatus::Unqueried` and `LogRecordStatus::Error` primarily for this aggregated snapshot case.
* Decide the query scope explicitly before implementation:
  * direct collection only
  * recursive crawl
* Decide whether partial failures should be visible in the returned record or surfaced separately.
* Define deterministic merge behavior so repeated calls produce stable output ordering.
* Do not let `getNetworkLogRecord(date)` change the meaning of the local `getLogRecord(date)` API.

Important semantic rule:

* Keep the existing methods local-only:
  * `getLogNames`
  * `getLogCount`
  * `getLogIDs`
  * `getLogs`
* Keep the existing `deviceID` overloads local-only as well. They should not silently become network aggregation calls.

Why this matters:

* `LocalLogManager` now has concrete `getNetwork...` methods, but `RemoteLogManager` only exposes the base `LogManager` interface.
* If the abstract base is not updated, callers that receive a remote `shared_ptr<LogManager>` from a server device will have no RPC path to the new aggregation behavior.

#### `include/sti/device/LogRecord.h`

This has already been updated locally. The network layer must now mirror the new data model:

* `LogFileRecord`
* `LogNameRecord`
* `DeviceLogRecord.logs`
* `LogRecord.deviceLogRecords`

`logNames` should remain as a compatibility cache for now, but `logs` should be treated as the authoritative field.

#### `include/sti/device/LogFile.h`

This still uses the old transport shape:

* `FileID`
* `FileHolder`
* `String`

For the network pass there are two viable approaches:

* Minimal pass:
  Keep `LogFile` mostly as-is and rely on `LogRecord` metadata plus `FileID` for remote lazy access.
* Preferred long-term pass:
  Redesign `LogFile` as an explicit remote-open handle.

For the next implementation pass, the minimal option is enough as long as:

* `LogRecord` carries `fileID`, `bytes`, and `lineCount`
* the file transfer RPC layer can actually use that metadata for partial reads

#### `include/sti/utils/FileHolder.h` and `include/sti/utils/FileServer.h`

These are not log-specific classes, but they are part of the log remote-open path and need to stay aligned with the IDL.

Required checks:

* `FileHolder::getFileSize()` exists in C++, but the CORBA `TFileHolder` interface does not currently expose it.
* `FileServer::transferFilePartial()` already exists in C++, but the remote implementations are still stubs.

### 2. IDL changes

#### `src/network/idl/logsNet.idl`

This file should be updated to mirror the richer `LogRecord` structures.

Recommended wire types:

```idl
struct TLogFileRecord
{
    TLogID id;
    TFileID fileID;

    unsigned long long bytes;
    unsigned long long lineCount;

    TTimeStamp firstEntryTime;
    TTimeStamp lastEntryTime;
};
typedef sequence<TLogFileRecord> TLogFileRecordSeq;


struct TLogNameRecord
{
    string logName;
    TLogFileRecordSeq files;

    unsigned long long totalBytes;
    unsigned long long totalLines;

    unsigned long nextIndex;
    TTimeStamp lastUpdate;
};
typedef sequence<TLogNameRecord> TLogNameRecordSeq;


struct TDeviceLogRecord
{
    string deviceID;
    TLogRecordStatus status;

    // Keep temporarily for staged compatibility if desired.
    TStringSeq logNames;

    TLogNameRecordSeq logs;
};
typedef sequence<TDeviceLogRecord> TDeviceLogRecordSeq;
```

`TLogRecord` should continue to hold:

* `TTimeStamp timeStamp`
* `TDeviceLogRecordSeq deviceLogRecords`

Do not add a network `TLogCatalog` in this pass.

Reason:

* `LogCatalog` is a rebuildable local cache, not the authoritative per-day source of truth.
* Remote callers can get persistent names through `getLogNames()` / `getNetworkLogNames()` without exposing the catalog itself.

Type rules:

* Use 64-bit IDL integers for byte counts and line counts.
* Keep `TLogFileFilter.startIndex` and `endIndex` signed, because negative indices are used.
* Consider switching `TLogID.index` from `long` to `unsigned long` to match the C++ model more closely.

#### `src/network/idl/deviceNet.idl`

Update `TLogManager` to match the abstract C++ interface.

Required additions:

```idl
void getNetworkLogNames(out TStringSeq names);
long getNetworkLogCount(in TLogFileFilter filter);
void getNetworkLogIDs(in TLogFileFilter filter, out TLogIDSeq ids);
boolean getNetworkLogs(in TLogFileFilter filter, out TLogFileSeq files);
```

Optional later:

```idl
boolean getNetworkLogRecord(in string date, out TLogRecord record);
```

Keep the current local-only methods:

* `getLogNames`
* `getLogCount(in TDeviceID deviceID, ...)`
* `getLogIDs`
* `getDeviceLogIDs`
* `getLog`
* `getLogs`
* `getDeviceLogs`
* `getLogRecord`

Also update the file interfaces:

* Add `getFileSize()` to `TFileHolder`
* Keep `transferFilePartial()` on `TFileServer`, but define its semantics clearly for logs

Recommended partial-transfer semantics for logs:

* `offset` is line-based, not byte-based
* `lines` is the maximum number of lines to transfer
* negative `offset` counts backward from the end of the file

If that overloading feels too awkward for a generic file service, the alternative is to add a direct log/text RPC that returns a string slice instead of using a destination `TFileHolder`.

### 3. Network conversion layer

#### `src/network/src/convert/Convert_Log.h`
#### `src/network/src/convert/Convert_Log.cpp`

Add conversions for:

* `TLogFileRecord <-> LogFileRecord`
* `TLogNameRecord <-> LogNameRecord`
* updated `TDeviceLogRecord <-> DeviceLogRecord`
* updated `TLogRecord <-> LogRecord`

Conversion rules:

* `logs` is authoritative
* `logNames` is compatibility-only
* when encoding, populate `logNames` from `logs`
* when decoding, build the `logs` map first, then call the equivalent of `syncLogNamesFromLogs()` if needed

Because CORBA IDL does not have maps:

* `DeviceLogRecord.logs` must travel as a sequence
* `LogNameRecord.files` must travel as a sequence

Canonicalization rule:

* use `TLogNameRecord.logName` as the map key when reconstructing `DeviceLogRecord.logs`
* use `TLogFileRecord.id.index` as the map key when reconstructing `LogNameRecord.files`

Do not rely on sequence order as the semantic key.

### 4. Remote proxy and servant changes

#### `src/network/src/RemoteLogManager.h`
#### `src/network/src/RemoteLogManager.cpp`

Update `RemoteLogManager` to implement the new abstract methods:

* `getNetworkLogNames`
* `getNetworkLogCount`
* `getNetworkLogIDs`
* `getNetworkLogs`

Keep the existing methods consistent with the local-only semantics in `LogManager.h`.

#### `src/network/src/TLogManager_i.h`
#### `src/network/src/TLogManager_i.cpp`

Update the servant to expose the new IDL methods and forward them to the underlying `LogManager`.

Important rule:

* do not re-use the old `deviceID` methods to implement the new network methods
* forward each new RPC to the explicit `getNetwork...` method on the wrapped manager

### 5. File transfer classes needed for remote log open

These are not purely "logging" classes, but remote lazy log access will not work correctly unless they are updated in the same pass.

#### `src/network/src/TFileHolder_i.h`
#### `src/network/src/TFileHolder_i.cpp`

Add support for:

* `getFileSize()`

#### `src/network/src/RemoteFileHolder.cpp`

Fix:

* `RemoteFileHolder::getFileSize()` currently returns `0`

It should call the new `TFileHolder::getFileSize()` RPC.

#### `src/network/src/TFileServer_i.cpp`
#### `src/network/src/RemoteFileServer.cpp`
#### `src/device/src/LocalFileServer.cpp`

Implement:

* `transferFilePartial()`

Current status:

* `LocalFileServer::transferFilePartial()` is stubbed
* `TFileServer_i::transferFilePartial()` is stubbed
* `RemoteFileServer::transferFilePartial()` is stubbed

If the final partial-read path uses file servers, these must all be completed together.

#### `src/network/src/TFileHolderRefInterface.h`
#### `src/network/src/NetworkFileHolder.h`
#### `src/network/src/NetworkFileHolder.cpp`

These need to be considered when deciding how partial log transfer is consumed by remote clients.

Current subtle issue:

* the generic transfer API expects the destination to expose a `TFileHolder` reference
* that works for `NetworkFileHolder` / `RemoteFileHolder`
* it does not automatically help Python clients that only hold plain wrapped `FileHolder` objects

### 6. Persistence / file server access

If the implementation uses `LogRecord.LogFileRecord.fileID` as the source of truth for remote open, the originating device's `FileServer` is the natural transport path.

Relevant existing classes:

* `include/sti/device/PersistenceManager.h`
* `src/network/src/RemotePersistenceManager.cpp`
* `src/network/src/TPersistenceManager_i.cpp`

These do not necessarily need new logic, but they are part of the remote-open path because `PersistenceManager` already exposes `FileServer`.

Important rule:

* When a server aggregates logs from many devices, partial file reads must use the originating device's file server, not the aggregator's local file server.

### 7. Generated files

After editing the IDLs:

* regenerate `src/network/src/generated/deviceNet.h`
* regenerate `src/network/src/generated/deviceNet.cpp`
* regenerate any other generated network files produced from the same IDLs

Do not hand-edit generated files.

### 8. Subtle issues to keep in mind

#### Local-only vs network aggregation semantics

The current device-side design is now explicit:

* old methods are local-only
* new `getNetwork...` methods aggregate across the device collection

The network API should preserve that distinction exactly.

#### Stable ordering of aggregated results

`getNetworkLogIDs()` and `getNetworkLogs()` should return results in a deterministic order after merging responses from multiple devices.

Do not let final ordering depend on:

* device collection iteration order
* CORBA sequence arrival order

Choose and document a stable merged ordering before implementation.

#### Source identity must be preserved

For aggregated server-level queries:

* `LogID.deviceID` must remain the originating device
* `FileID.origin` and `FileID.persistenceLocation` must remain the originating device

Do not rewrite these to the aggregator/server device.

#### `logs` vs `logNames`

During the transition:

* encode both if helpful
* treat `logs` as authoritative
* never derive `logs` from `logNames` alone on the new code path unless you are explicitly handling an old record from an older peer

#### Partial transfer semantics must be explicit

The current generic `transferFilePartial(source, destination, offset, lines)` signature is too ambiguous unless the semantics are written down.

Before implementing it, define:

* whether `offset` is from the beginning or end
* whether negative values are allowed
* whether the API is text-only or also valid for binary files
* what happens if the requested range exceeds the available line count

#### Python / wrapper integration

Even after the IDL and network classes are updated, `stipy` may still need a follow-up pass.

Reason:

* a Python caller may want a direct "open last N lines" operation
* the current generic destination-holder transfer model is awkward from Python unless a servant-backed file holder is exposed there

That means the implementation should either:

* expose the necessary network file-holder type to wrappers
* or add a simpler log/text RPC that avoids requiring a destination holder on the client side

#### Mixed-version deployments

Once the IDLs change, old and new network peers will not be safely wire-compatible.

Assume a coordinated upgrade:

* update IDLs
* regenerate stubs/skeletons
* rebuild both client and server network code together
