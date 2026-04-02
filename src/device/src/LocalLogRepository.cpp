#include "LocalLogRepository.h"

#include <sti/LocalDevice.h>
#include <sti/utils/FileHolder.h>
#include <sti/utils/utils.h>

#include "LocalPersistenceManager.h"
#include "LogCatalogFile.h"
#include "LogRecordFile.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>

namespace fs = std::filesystem;

using STI::Device::DeviceLogRecord;
using STI::Device::LocalDevice;
using STI::Device::LocalLogRepository;
using STI::Device::LocalPersistenceManager;
using STI::Device::LogCatalog;
using STI::Device::LogCatalogEntry;
using STI::Device::LogCatalogFile;
using STI::Device::LogFile;
using STI::Device::LogFileFilter;
using STI::Device::LogFileRecord;
using STI::Device::LogID;
using STI::Device::LogNameRecord;
using STI::Device::LogRecord;
using STI::Device::LogRecordFile;
using STI::Device::LogRecordStatus;
using STI::Utils::FileHolder;
using STI::Utils::TimeStamp;

namespace {

TimeStamp dayStamp(const TimeStamp& timestamp)
{
    return TimeStamp(timestamp.year(), timestamp.month(), timestamp.day(), 0, 0, 0, 0, 0, 0);
}

bool hasLogFiles(const fs::path& path)
{
    if (!fs::exists(path) || !fs::is_directory(path)) {
        return false;
    }

    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".log") {
            return true;
        }
    }

    return false;
}

bool parseLogFilenameStem(const std::string& filenameStem, std::string& logName, unsigned& index)
{
    const std::string separator = "_";

    auto startPos = filenameStem.find_first_of(separator);
    auto endPos = filenameStem.find_last_of(separator);

    if (startPos == std::string::npos || endPos == std::string::npos) {
        return false;
    }

    if (startPos == endPos) {
        logName = "";
    }
    else {
        logName = filenameStem.substr(startPos + 1, endPos - startPos - 1);
    }

    return STI::Utils::stringToValue(filenameStem.substr(endPos + 1), index);
}

bool parseLogFilename(const fs::path& filePath, std::string& logName, unsigned& index)
{
    if (!filePath.has_extension() || filePath.extension() != ".log") {
        return false;
    }

    return parseLogFilenameStem(filePath.stem().string(), logName, index);
}

bool tryParseDateFromPath(const fs::path& path, TimeStamp& timestamp)
{
    std::vector<std::string> components;
    for (const auto& part : path) {
        components.push_back(part.string());
    }

    for (std::size_t i = 0; i + 3 < components.size(); ++i) {
        if (components[i] != "logs") {
            continue;
        }

        int year = 0;
        int month = 0;
        int day = 0;

        if (!STI::Utils::stringToValue(components[i + 1], year)) continue;
        if (!STI::Utils::stringToValue(components[i + 2], month)) continue;
        if (!STI::Utils::stringToValue(components[i + 3], day)) continue;

        timestamp = TimeStamp(year, month, day, 0, 0, 0, 0, 0, 0);
        return true;
    }

    return false;
}

bool tryParseLogLineTimestamp(const std::string& line, TimeStamp& timestamp)
{
    if (line.empty() || line.front() != '<') {
        return false;
    }

    auto endPos = line.find('>');
    if (endPos == std::string::npos || endPos <= 1) {
        return false;
    }

    auto header = line.substr(1, endPos - 1);
    auto pipePos = header.find('|');
    if (pipePos != std::string::npos) {
        header = header.substr(0, pipePos);
    }

    auto spacePos = header.find(' ');
    if (spacePos == std::string::npos) {
        return false;
    }

    auto date = header.substr(0, spacePos);
    auto time = header.substr(spacePos + 1);

    if (date.empty() || time.empty()) {
        return false;
    }

    timestamp = TimeStamp::fromString(date + "|" + time);
    return true;
}

std::vector<unsigned> matchingIndices(const LogNameRecord& nameRecord, int startIndex, int endIndex)
{
    std::vector<unsigned> indices;
    indices.reserve(nameRecord.files.size());

    for (const auto& entry : nameRecord.files) {
        indices.push_back(entry.first);
    }

    const int len = static_cast<int>(indices.size());
    if (len == 0) {
        return {};
    }

    int iStart = startIndex;
    int iEnd = endIndex;

    if (iStart < 0) iStart = len + iStart;
    if (iEnd < 0) iEnd = len + iEnd;

    iStart = std::clamp(iStart, 0, len - 1);
    iEnd = std::clamp(iEnd, 0, len - 1);

    if (iStart > iEnd) {
        std::swap(iStart, iEnd);
    }

    return std::vector<unsigned>(indices.begin() + iStart, indices.begin() + iEnd + 1);
}

} // namespace

LocalLogRepository::LocalLogRepository(
    LocalDevice* localDevice,
    const std::shared_ptr<LocalPersistenceManager>& localPersistenceManager)
: localDevice(localDevice),
  localPersistenceManager(localPersistenceManager),
  lastRecordDate(1970, 1, 1, 0, 0, 0, 0, 0, 0)
{
}

LocalLogRepository::~LocalLogRepository()
{
}

bool LocalLogRepository::getDeviceLogPath(const TimeStamp& timestamp, std::string& logDevicePath) const
{
    if (localDevice == nullptr || localPersistenceManager == nullptr) {
        return false;
    }

    return localPersistenceManager->getLogBasePath(timestamp, localDevice->getID(), logDevicePath);
}

bool LocalLogRepository::getLogRootPath(std::string& logRootPath) const
{
    if (localPersistenceManager == nullptr) {
        return false;
    }

    fs::path root(localPersistenceManager->getBasePath());
    root /= "logs";
    logRootPath = root.string();
    return true;
}

bool LocalLogRepository::buildLogFileRecord(
    const TimeStamp& timestamp,
    const std::string& logName,
    unsigned index,
    const std::string& logFilename,
    LogFileRecord& fileRecord) const
{
    fs::path path(logFilename);
    if (!fs::exists(path) || !fs::is_regular_file(path)) {
        return false;
    }

    LogID id;
    id.date = timestamp.date_YYYY_MM_DD("/");
    id.deviceID = localDevice->getID();
    id.logName = logName;
    id.index = index;

    fileRecord.id = id;
    fileRecord.bytes = fs::file_size(path);
    fileRecord.lineCount = 0;
    fileRecord.firstEntryTime = dayStamp(timestamp);
    fileRecord.lastEntryTime = dayStamp(timestamp);

    auto fileHolder = localDevice->makeFileHolder(path.parent_path().string(), path.filename().string());
    if (fileHolder != nullptr) {
        fileRecord.fileID = fileHolder->getID();
    }

    std::ifstream stream(path);
    if (!stream.is_open()) {
        return true;
    }

    bool haveFirstEntry = false;
    std::string line;

    while (std::getline(stream, line)) {
        ++fileRecord.lineCount;

        TimeStamp entryTime;
        if (!tryParseLogLineTimestamp(line, entryTime)) {
            continue;
        }

        if (!haveFirstEntry) {
            fileRecord.firstEntryTime = entryTime;
            haveFirstEntry = true;
        }

        fileRecord.lastEntryTime = entryTime;
    }

    return true;
}

bool LocalLogRepository::buildDeviceLogRecord(const TimeStamp& timestamp, DeviceLogRecord& deviceRecord) const
{
    deviceRecord.deviceID = localDevice->getID().getID();
    deviceRecord.status = LogRecordStatus::NoLogs;
    deviceRecord.logNames.clear();
    deviceRecord.logs.clear();

    std::string logDevicePath;
    if (!getDeviceLogPath(timestamp, logDevicePath)) {
        return false;
    }

    fs::path devicePath(logDevicePath);
    if (!fs::exists(devicePath) || !fs::is_directory(devicePath)) {
        return false;
    }

    for (const auto& entry : fs::directory_iterator(devicePath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        std::string logName;
        unsigned index = 0;
        if (!parseLogFilename(entry.path(), logName, index)) {
            continue;
        }

        LogFileRecord fileRecord;
        if (!buildLogFileRecord(timestamp, logName, index, entry.path().string(), fileRecord)) {
            continue;
        }

        auto& logRecord = deviceRecord.logs[logName];
        logRecord.logName = logName;
        logRecord.files[index] = fileRecord;
    }

    if (deviceRecord.logs.empty()) {
        return false;
    }

    deviceRecord.status = LogRecordStatus::LogsPresent;
    deviceRecord.syncLogNamesFromLogs();
    return true;
}

bool LocalLogRepository::shouldRebuildRecord(const TimeStamp& timestamp, const LogRecordFile& recordFile) const
{
    const auto& record = recordFile.getRecord();

    std::string logDevicePath;
    const bool pathAvailable = getDeviceLogPath(timestamp, logDevicePath);
    const bool logsExist = pathAvailable && hasLogFiles(fs::path(logDevicePath));

    if (!recordFile.exists()) {
        return logsExist;
    }

    if (!record.timeStamp.isSameDate(timestamp)) {
        return true;
    }

    const auto recordIt = record.deviceLogRecords.find(localDevice->getID().getID());
    if (recordIt == record.deviceLogRecords.end()) {
        return logsExist;
    }

    if (logsExist && recordIt->second.status != LogRecordStatus::LogsPresent) {
        return true;
    }

    if (logsExist && recordIt->second.logs.empty()) {
        return true;
    }

    return false;
}

bool LocalLogRepository::rebuildDailyRecord(const TimeStamp& timestamp, LogRecordFile& recordFile)
{
    auto rebuiltRecord = recordFile.getRecord();
    rebuiltRecord.timeStamp = dayStamp(timestamp);

    DeviceLogRecord deviceRecord;
    const bool hasLogs = buildDeviceLogRecord(timestamp, deviceRecord);

    if (!hasLogs) {
        deviceRecord.deviceID = localDevice->getID().getID();
        deviceRecord.status = LogRecordStatus::NoLogs;
        deviceRecord.logNames.clear();
        deviceRecord.logs.clear();
    }

    rebuiltRecord.deviceLogRecords[localDevice->getID().getID()] = deviceRecord;
    recordFile.getRecord() = rebuiltRecord;
    recordFile.save();

    return recordFile.exists();
}

bool LocalLogRepository::ensureRecordReady(const TimeStamp& timestamp, LogRecordFile& recordFile)
{
    if (!recordFile.exists() && !shouldRebuildRecord(timestamp, recordFile)) {
        return false;
    }

    if (shouldRebuildRecord(timestamp, recordFile)) {
        return rebuildDailyRecord(timestamp, recordFile);
    }

    return recordFile.exists();
}

bool LocalLogRepository::getLogRecordFile(const TimeStamp& timestamp, std::shared_ptr<LogRecordFile>& recordFile, bool autocreate)
{
    if (lastLogRecordFile != nullptr && lastRecordDate.isSameDate(timestamp)) {
        recordFile = lastLogRecordFile;
        return autocreate ? true : ensureRecordReady(timestamp, *recordFile);
    }

    std::string logBasePath;
    if (autocreate) {
        if (!localPersistenceManager->makeLogPath(timestamp, logBasePath)) {
            return false;
        }
    }
    else if (!localPersistenceManager->getLogBasePath(timestamp, logBasePath)) {
        return false;
    }

    fs::path recordPath(logBasePath);
    recordPath /= "logRecord.ini";

    recordFile = std::make_shared<LogRecordFile>(recordPath.string());
    if (recordFile == nullptr) {
        return false;
    }

    if (autocreate && !recordFile->exists()) {
        recordFile->getRecord().timeStamp = dayStamp(timestamp);
        recordFile->save();
    }

    const bool ready = ensureRecordReady(timestamp, *recordFile);
    if (ready || autocreate) {
        lastLogRecordFile = recordFile;
        lastRecordDate = dayStamp(timestamp);
    }

    return ready || (autocreate && recordFile->exists());
}

bool LocalLogRepository::getLogCatalogFile(std::shared_ptr<LogCatalogFile>& catalogFile)
{
    if (cachedCatalogFile != nullptr) {
        catalogFile = cachedCatalogFile;
        return true;
    }

    std::string logRootPath;
    if (!getLogRootPath(logRootPath)) {
        return false;
    }

    fs::path catalogPath(logRootPath);
    catalogPath /= "logCatalog.ini";

    catalogFile = std::make_shared<LogCatalogFile>(catalogPath.string());
    if (catalogFile == nullptr) {
        return false;
    }

    cachedCatalogFile = catalogFile;
    return true;
}

bool LocalLogRepository::catalogNeedsRebuild(const LogCatalogFile& catalogFile) const
{
    if (!catalogFile.exists()) {
        return true;
    }

    std::string logRootPath;
    if (!getLogRootPath(logRootPath)) {
        return false;
    }
    if (!fs::exists(logRootPath)) {
        return false;
    }

    fs::path catalogPath(logRootPath);
    catalogPath /= "logCatalog.ini";

    if (!fs::exists(catalogPath)) {
        return true;
    }

    const auto catalogTime = fs::last_write_time(catalogPath);

    for (const auto& entry : fs::recursive_directory_iterator(logRootPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto filename = entry.path().filename().string();
        const bool isMetadata = filename == "logRecord.ini" || filename == "logCatalog.ini";
        const bool isLogFile = entry.path().extension() == ".log";
        if (!isMetadata && !isLogFile) {
            continue;
        }

        if (entry.path() == catalogPath) {
            continue;
        }

        if (fs::last_write_time(entry.path()) > catalogTime) {
            return true;
        }
    }

    return false;
}

bool LocalLogRepository::rebuildCatalog(LogCatalogFile& catalogFile)
{
    LogCatalog catalog;
    catalog.deviceID = localDevice->getID().getID();
    catalog.lastUpdate = TimeStamp();

    std::string logRootPath;
    if (!getLogRootPath(logRootPath)) {
        return false;
    }

    if (!fs::exists(logRootPath)) {
        catalogFile.getCatalog() = catalog;
        catalogFile.save();
        return true;
    }

    std::set<std::string> dateKeys;

    for (const auto& entry : fs::recursive_directory_iterator(logRootPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto filename = entry.path().filename().string();
        if (filename != "logRecord.ini" && entry.path().extension() != ".log") {
            continue;
        }

        TimeStamp timestamp;
        if (!tryParseDateFromPath(entry.path(), timestamp)) {
            continue;
        }

        dateKeys.insert(timestamp.date_YYYY_MM_DD("/"));
    }

    for (const auto& dateKey : dateKeys) {
        auto timestamp = TimeStamp::fromString(dateKey);
        std::shared_ptr<LogRecordFile> recordFile;
        if (!getLogRecordFile(timestamp, recordFile, false) || recordFile == nullptr) {
            continue;
        }

        const auto& record = recordFile->getRecord();
        const auto recordIt = record.deviceLogRecords.find(localDevice->getID().getID());
        if (recordIt == record.deviceLogRecords.end()) {
            continue;
        }

        if (recordIt->second.status != LogRecordStatus::LogsPresent) {
            continue;
        }

        for (const auto& entry : recordIt->second.logs) {
            auto& catalogEntry = catalog.logs[entry.first];
            catalogEntry.logName = entry.first;

            if (catalogEntry.dayCount == 0 || timestamp < catalogEntry.firstDay) {
                catalogEntry.firstDay = dayStamp(timestamp);
            }
            if (catalogEntry.dayCount == 0 || catalogEntry.lastDay < timestamp) {
                catalogEntry.lastDay = dayStamp(timestamp);
            }

            ++catalogEntry.dayCount;
            catalogEntry.fileCount += entry.second.files.size();
            catalogEntry.totalBytes += entry.second.totalBytes;
            catalogEntry.totalLines += entry.second.totalLines;
        }
    }

    catalogFile.getCatalog() = catalog;
    catalogFile.save();
    return true;
}

void LocalLogRepository::getLogNames(std::set<std::string>& names)
{
    names.clear();

    std::shared_ptr<LogCatalogFile> catalogFile;
    if (!getLogCatalogFile(catalogFile) || catalogFile == nullptr) {
        return;
    }

    if (catalogNeedsRebuild(*catalogFile)) {
        rebuildCatalog(*catalogFile);
    }

    LogCatalog catalog;
    if (!catalogFile->copyCatalog(catalog)) {
        return;
    }

    for (const auto& entry : catalog.logs) {
        names.insert(entry.first);
    }
}

std::size_t LocalLogRepository::countMatchingFiles(const LogNameRecord& nameRecord, int startIndex, int endIndex) const
{
    return matchingIndices(nameRecord, startIndex, endIndex).size();
}

void LocalLogRepository::appendMatchingIDs(const LogNameRecord& nameRecord, int startIndex, int endIndex, std::vector<LogID>& ids) const
{
    for (auto index : matchingIndices(nameRecord, startIndex, endIndex)) {
        const auto it = nameRecord.files.find(index);
        if (it != nameRecord.files.end()) {
            ids.push_back(it->second.id);
        }
    }
}

int LocalLogRepository::getLogCount(const LogFileFilter& filter)
{
    auto date = TimeStamp::fromString(filter.startDate);
    auto endDate = TimeStamp::fromString(filter.endDate);

    if (endDate < date) {
        return 0;
    }

    int count = 0;

    do {
        std::shared_ptr<LogRecordFile> recordFile;
        if (getLogRecordFile(date, recordFile, false) && recordFile != nullptr) {
            const auto& record = recordFile->getRecord();
            const auto recordIt = record.deviceLogRecords.find(localDevice->getID().getID());
            if (recordIt != record.deviceLogRecords.end() && recordIt->second.status == LogRecordStatus::LogsPresent) {
                if (filter.logName == "*") {
                    for (const auto& entry : recordIt->second.logs) {
                        count += static_cast<int>(countMatchingFiles(entry.second, filter.startIndex, filter.endIndex));
                    }
                }
                else {
                    const auto logIt = recordIt->second.logs.find(filter.logName);
                    if (logIt != recordIt->second.logs.end()) {
                        count += static_cast<int>(countMatchingFiles(logIt->second, filter.startIndex, filter.endIndex));
                    }
                }
            }
        }

        date.add_day();
    }
    while (date <= endDate);

    return count;
}

void LocalLogRepository::getLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids)
{
    auto date = TimeStamp::fromString(filter.startDate);
    auto endDate = TimeStamp::fromString(filter.endDate);

    if (endDate < date) {
        return;
    }

    do {
        std::shared_ptr<LogRecordFile> recordFile;
        if (getLogRecordFile(date, recordFile, false) && recordFile != nullptr) {
            const auto& record = recordFile->getRecord();
            const auto recordIt = record.deviceLogRecords.find(localDevice->getID().getID());
            if (recordIt != record.deviceLogRecords.end() && recordIt->second.status == LogRecordStatus::LogsPresent) {
                if (filter.logName == "*") {
                    for (const auto& entry : recordIt->second.logs) {
                        appendMatchingIDs(entry.second, filter.startIndex, filter.endIndex, ids);
                    }
                }
                else {
                    const auto logIt = recordIt->second.logs.find(filter.logName);
                    if (logIt != recordIt->second.logs.end()) {
                        appendMatchingIDs(logIt->second, filter.startIndex, filter.endIndex, ids);
                    }
                }
            }
        }

        date.add_day();
    }
    while (date <= endDate);
}

bool LocalLogRepository::getLog(const LogID& id, LogFile& logFile)
{
    if (localDevice == nullptr || id.deviceID != localDevice->getID()) {
        return false;
    }

    std::shared_ptr<LogRecordFile> recordFile;
    auto timestamp = TimeStamp::fromString(id.date);
    if (!getLogRecordFile(timestamp, recordFile, false) || recordFile == nullptr) {
        return false;
    }

    const auto& record = recordFile->getRecord();
    const auto recordIt = record.deviceLogRecords.find(localDevice->getID().getID());
    if (recordIt == record.deviceLogRecords.end()) {
        return false;
    }

    const auto logIt = recordIt->second.logs.find(id.logName);
    if (logIt == recordIt->second.logs.end()) {
        return false;
    }

    const auto fileIt = logIt->second.files.find(id.index);
    if (fileIt == logIt->second.files.end()) {
        return false;
    }

    const auto& fileRecord = fileIt->second;

    logFile.id = id;
    logFile.type = LogFile::LogFileType::FileHolder;
    logFile.fileID = fileRecord.fileID;
    logFile.fileHolder = localDevice->makeFileHolder(fileRecord.fileID.path, fileRecord.fileID.filename);
    logFile.logString.clear();

    return logFile.fileHolder != nullptr && logFile.fileHolder->exists();
}

bool LocalLogRepository::getLogs(const LogFileFilter& filter, std::vector<LogFile>& files)
{
    std::vector<LogID> ids;
    getLogIDs(filter, ids);

    bool success = true;
    for (const auto& id : ids) {
        LogFile logFile;
        if (getLog(id, logFile)) {
            files.push_back(logFile);
        }
        else {
            success = false;
        }
    }

    return success;
}

bool LocalLogRepository::getLogRecord(const std::string& date, LogRecord& record)
{
    std::shared_ptr<LogRecordFile> recordFile;
    auto timestamp = TimeStamp::fromString(date);

    if (getLogRecordFile(timestamp, recordFile, false) && recordFile != nullptr) {
        return recordFile->copyRecord(record);
    }

    return false;
}

bool LocalLogRepository::recordLocalWrite(const TimeStamp& timestamp, const std::string& logFilename)
{
    std::shared_ptr<LogRecordFile> recordFile;
    if (!getLogRecordFile(timestamp, recordFile, true) || recordFile == nullptr) {
        return false;
    }

    std::string parsedName;
    unsigned index = 0;
    if (!parseLogFilename(fs::path(logFilename), parsedName, index)) {
        return false;
    }

    LogFileRecord fileRecord;
    if (!buildLogFileRecord(timestamp, parsedName, index, logFilename, fileRecord)) {
        return false;
    }

    auto& record = recordFile->getRecord();
    record.timeStamp = dayStamp(timestamp);

    auto& deviceRecord = record.deviceLogRecords[localDevice->getID().getID()];
    deviceRecord.deviceID = localDevice->getID().getID();
    deviceRecord.status = LogRecordStatus::LogsPresent;

    const std::string& storedLogName = parsedName;

    auto& nameRecord = deviceRecord.logs[storedLogName];
    nameRecord.logName = storedLogName;
    nameRecord.files[index] = fileRecord;
    nameRecord.recalculateTotals();
    nameRecord.lastUpdate = TimeStamp();

    deviceRecord.syncLogNamesFromLogs();
    recordFile->save();

    std::shared_ptr<LogCatalogFile> catalogFile;
    if (getLogCatalogFile(catalogFile) && catalogFile != nullptr) {
        rebuildCatalog(*catalogFile);
    }

    return true;
}
