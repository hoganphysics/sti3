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
