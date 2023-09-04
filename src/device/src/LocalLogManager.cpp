
#include "LocalLogManager.h"

#include <sti/device/Logger.h>

#include "LocalTaskManager.h"
#include "LocalChannelManager.h"
#include "LocalAttributeManager.h"
#include "LocalPersistenceManager.h"
#include "LogRecordFile.h"
#include <sti/LocalDevice.h>
#include <sti/device/DeviceID.h>
// #include <sti/utils/ConfigFile.h>

#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

using STI::Device::LocalLogManager;
using STI::Device::Logger;
using STI::Device::LogID;
using STI::Device::LogFile;
using STI::Device::LogFileFilter;
using STI::Device::LogRecord;
using STI::Device::DeviceID;
using STI::Device::LocalPersistenceManager;
using STI::Utils::TimeStamp;


LocalLogManager::LocalLogManager(LocalDevice* localDevice, const std::shared_ptr<LocalPersistenceManager>& localPersistenceManager)
: localDevice(localDevice), localPersistenceManager(localPersistenceManager), logWriterMessageGrouper(this)
{
    createLogger("");   //default log

    logWriterMessageGrouper.setWarmup(1000);      //ms
    logWriterMessageGrouper.setCooldown(1000);    //ms

    logWriterMessageGrouper.start();
}

LocalLogManager::~LocalLogManager()
{
}

void LocalLogManager::writeLog(const std::string& logName)
{
    std::string logBasePath;
    std::string logDevicePath;
    STI::Utils::TimeStamp timestamp;

    if (!localPersistenceManager->makeLogPath(timestamp, logBasePath)) return;

    //modify log record
    std::shared_ptr<LogRecordFile> recordFile;
    if (!getLogRecordFile(timestamp, recordFile, true)) return;
    lastLogRecordFile = recordFile;
    lastLogRecordFile->setLogStatus(localDevice->getID(), logName);
    
    if (!localPersistenceManager->makeLogPath(timestamp, localDevice->getID(), logDevicePath)) return;

    std::shared_ptr<Logger> logger;

    if (loggers.get(logName, logger) && logger != 0) {
        logger->save(logDevicePath);
    }
}


bool LocalLogManager::getLogRecordFile(const STI::Utils::TimeStamp& timestamp, std::shared_ptr<LogRecordFile>& recordFile, bool autocreate)
{
    if (lastLogRecordFile != 0 && lastLogRecordFile->getRecord().timeStamp.isSameDate(timestamp)) {
        recordFile = lastLogRecordFile;
        return recordFile != 0 && recordFile->exists();
    }

    std::string recordFileName = "logRecord.ini";
    std::string logBasePath;
    
    if (!localPersistenceManager->getLogBasePath(timestamp, logBasePath)) return false;

    fs::path logRecordPath = logBasePath;
    logRecordPath /= recordFileName;

    recordFile = std::make_shared<LogRecordFile>(logRecordPath.string());
    if (recordFile == 0) return false;

    if (autocreate && !recordFile->exists()) {
        recordFile->save();     //create file
    }

    return recordFile->exists();
    

    // return recordFile != 0;

    // if (!std::filesystem::exists(logRecordPath)) return false;

    // LogRecordFile recordFile(logRecordPath.string());

    // if (recordFile.exists()) {
    //     return recordFile.getRecord(record);
    // }


    // ConfigFile recordFile(logRecordPath.filename());

    // if (recordFile.isParsed()) {
    //     LogRecord record;

    //     std::string ts;
    //     if (recordFile.getParameter(logName, "TimeStamp", ts)) {
    //         record.timeStamp = STI::Utils::TimeStamp::fromString(ts);
    //     }
    //     auto ids = recordFile.getList(logName, "DeviceIDs");
    //     for (auto& id : ids) {
    //         record.loggedIDs.push_back(id);
    //     }
    // }

}

bool LocalLogManager::getLogRecord(const std::string& date, LogRecord& record)
{
    std::shared_ptr<LogRecordFile> recordFile;
    auto timestamp = TimeStamp::fromString(date);
    
    if (getLogRecordFile(timestamp, recordFile)) {
        return recordFile->copyRecord(record);
    }
    return false;
}


void LocalLogManager::getLogNames(std::set<std::string>& names)
{
    loggers.getKeys(names);
}

int LocalLogManager::getLogCount(const LogFileFilter& filter)
{
    return getLogCount(localDevice->getID(), filter);
}

int LocalLogManager::getLogCount(const DeviceID& deviceID, const LogFileFilter& filter)
{
    auto date = TimeStamp::fromString(filter.startDate);
    auto endDate = TimeStamp::fromString(filter.endDate);

    int count = 0;
    
    std::map<std::string, int> counts;
    do {
        getLogCounts(date, deviceID, counts);
        date.add_day();
    } while (date <= endDate);

    if (filter.logName == "*") {
        //match any
        int total = 0;
        for (auto& m : counts) {
            total += m.second;
        }
        return total;
    }

    if (counts.find(filter.logName) != counts.end()) {
        return counts[filter.logName];
    }

    return 0;
}

bool LocalLogManager::getLogCounts(const STI::Utils::TimeStamp& date, const DeviceID& deviceID, std::map<std::string, int>& counts)
{
    std::shared_ptr<LogRecordFile> recordFile;
    
    if (!getLogRecordFile(date, recordFile, false)) {
        //no log record found on this day
        return false;
    }

    auto& deviceLogRecords = recordFile->getRecord().deviceLogRecords;

    auto it = deviceLogRecords.find(deviceID.getID());
    if (it == deviceLogRecords.end()) return false;     //no log records for this device on this day

    if (it->second.status != LogRecordStatus::LogsPresent) return false;

    // //found log record
    // if (it->second.status == LogRecordStatus::LogsPresent) {
    //     auto logit = it->second.logNames.find(logName);

    //     if (logit == it->second.logNames.end()) return 0;   //did not find logName
    // }
    // else {
    //     //LogRecordStatus indicates logs not present
    //     return 0;
    // }


    std::string logBasePath;
    std::string logDevicePath;

    if (!localPersistenceManager->makeLogPath(date, logBasePath)) return false;
    if (!localPersistenceManager->makeLogPath(date, deviceID.getID(), logDevicePath)) return false;

    //count all log files in path, grouping by logName
    fs::path searchPath = logDevicePath;
    for(auto& p : fs::directory_iterator(searchPath)) {
        //check that the file has extension .log
        if (p.path().has_extension() && p.path().extension().string() == ".log") {
            auto filename = p.path().stem().string();    //filename, no path, no extension
            
            std::string logName;
            int index;
            if (getLogName(filename, logName, index)) {

                if (counts.find(logName) == counts.end()) {
                    //not found
                    counts[logName] = 0;
                }
                counts[logName]++;
            }
        }
    }
    return true;
}

bool LocalLogManager::getLogName(const std::string& filenameStem, std::string& logName, int& index) const
{
    std::string separator = "_";    //sti_logName_# or just sti_#

    auto startPos = filenameStem.find_first_of(separator);
    auto endPos = filenameStem.find_last_of(separator);

    if (startPos != std::string::npos && endPos != std::string::npos) {
        if (startPos == endPos) {
            logName = "";      //default log, no name
        }
        else {
            logName = filenameStem.substr(startPos + 1, endPos - startPos - 1);
        }
        STI::Utils::stringToValue(filenameStem.substr(endPos + 1), index);
        return true;
    }
    return false;
}

std::string LocalLogManager::makeLogFilename(const std::string& logName, int index)
{
    std::string logbasename = "sti";
    std::string extension = "log";
    std::string separator = "_";    //sti_logName_index.log or just sti_index.log

    std::stringstream filename;

    filename << logbasename;
    if (logName != "") {
        filename << "_" << logName;
    }
    filename << "_" << STI::Utils::valueToString(index) << "." << extension;

    return filename.str();
}

void LocalLogManager::getLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids)
{
    return getLogIDs(localDevice->getID(), filter, ids);
}

void LocalLogManager::getLogIDs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogID>& ids)
{
    auto date = TimeStamp::fromString(filter.startDate);
    auto endDate = TimeStamp::fromString(filter.endDate);

    std::set<std::string> logNames;
    if (filter.logName == "*") {
        //match all logNames
        loggers.getKeys(logNames);
    }
    else {
        logNames.insert(filter.logName);
    }

    do {
        for (auto& logName : logNames) {
            getLogIDs(date, deviceID, logName, filter.startIndex, filter.endIndex, ids);
        }
        date.add_day();
    } while (date <= endDate);
    // } while (!date.isSameDate(endDate));
}

void LocalLogManager::getLogIDs(const STI::Utils::TimeStamp& date, const DeviceID& deviceID, const std::string& logName, int startIndex, int endIndex, std::vector<LogID>& ids)
{
    // ids.clear();

    std::shared_ptr<LogRecordFile> recordFile;
    
    if (!getLogRecordFile(date, recordFile, false)) {
        //no log record found on this day
        return;
    }

    auto& deviceLogRecords = recordFile->getRecord().deviceLogRecords;

    auto it = deviceLogRecords.find(deviceID.getID());
    if (it == deviceLogRecords.end()) return;     //no log records for this device on this day

    if (it->second.status != LogRecordStatus::LogsPresent) return;

    std::string logBasePath;
    std::string logDevicePath;

    if (!localPersistenceManager->makeLogPath(date, logBasePath)) return;
    if (!localPersistenceManager->makeLogPath(date, deviceID.getID(), logDevicePath)) return;

    std::vector<int> indices;

    //search directory, collecting all files in range
    fs::path searchPath = logDevicePath;
    for(auto& p : fs::directory_iterator(searchPath)) {
        //ensure that the file has extension .log
        if (!( p.path().has_extension() && p.path().extension().string() == ".log" )) continue;
        
        auto filename = p.path().stem().string();    //filename, no path, no extension
        std::string name;
        int index;

        if (getLogName(filename, name, index) && name == logName) {
            //logName match found
            indices.push_back(index);
        }
    }
    std::sort(indices.begin(), indices.end());
    auto len = indices.size();
    int iStart = startIndex;
    int iEnd = endIndex;

    if (startIndex < 0) {
        iStart = len + startIndex;
    }
    if (iStart < 0) iStart = 0;
    if (iStart >= len) iStart = len - 1;

    if (endIndex < 0) {
        iEnd = len + endIndex;
    }
    if (iEnd < 0) iEnd = 0;
    if (iEnd >= len) iEnd = len - 1;

    if (iStart > iEnd) {
        std::swap(iStart, iEnd);
    }

    for (unsigned i = iStart; i <= iEnd; ++i) {
    // for (auto it = indices.begin() + iStart; it != indices.begin() + iEnd; ++it) {
        
        LogID logID;
        logID.date = date.date_YYYY_MM_DD("/");
        logID.deviceID = deviceID;
        logID.logName = logName;
        // logID.index = *it;
        logID.index = indices.at(i);

        ids.push_back(logID);
    }
}


bool LocalLogManager::getLog(const LogID& id, LogFile& logFile)
{
    auto date = TimeStamp::fromString(id.date);

    std::string logBasePath;
    std::string logDevicePath;

    if (!localPersistenceManager->makeLogPath(date, logBasePath)) return false;
    if (!localPersistenceManager->makeLogPath(date, localDevice->getID().getID(), logDevicePath)) return false;

    std::string logFilename = makeLogFilename(id.logName, id.index);

    fs::path logPath = logDevicePath;
    logPath /= logFilename;

    logFile.id = id;
    logFile.type = LogFile::LogFileType::FileHolder;
    logFile.fileHolder = localDevice->makeFileHolder(logPath.string());
    
    return logFile.fileHolder != 0 && logFile.fileHolder->exists();
}

bool LocalLogManager::getLog(const std::string& name, const std::string& date, unsigned index, LogFile& logFile)
{
    LogID logID;
    logID.date = date;
    logID.deviceID = localDevice->getID();
    logID.index = index;
    logID.logName = name;

    return getLog(logID, logFile);
}


bool LocalLogManager::getLogs(const LogFileFilter& filter, std::vector<LogFile>& files)
{
    std::vector<LogID> logIDs;
    getLogIDs(filter, logIDs);

    bool success = true;

    for (auto& id : logIDs) {
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

bool LocalLogManager::getLogs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogFile>& files)
{
    if (deviceID == localDevice->getID()) {
        return getLogs(filter, files);
    }

    std::shared_ptr<STI::Device::DeviceCollection> collection;
    localDevice->getCollection(collection);

    if (collection == 0) return false;

    std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<STI::Device::LogManager> logManager;
    if (collection->get(deviceID, device) && device != 0 && device->getLogManager(logManager)) {
        return logManager->getLogs(filter, files);
    }

    return false;
}


void LocalLogManager::createLogger(const std::string& name)
{
    if (loggers.contains(name)) return;

    auto logger = std::make_shared<Logger>(name, this);
    loggers.add(name, logger);
}


Logger& LocalLogManager::log()
{
    return log("");
}

Logger& LocalLogManager::log(const std::string& name)
{
    if (!loggers.contains(name)) {
        createLogger(name);
    }

    std::shared_ptr<Logger> logger;
    loggers.get(name, logger);
    return *logger;
}

