
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
    lastLogRecordFile->setLogStatus(localDevice->getID(), LogRecordStatus::LogsPresent);
    
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


void LocalLogManager::getLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids)
{
}

void LocalLogManager::getLogIDs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogID>& ids)
{
}


bool LocalLogManager::getLog(const LogID& id, LogFile& logFile)
{
    return false;
}

bool LocalLogManager::getLog(const std::string& name, const std::string& date, LogFile& logFile)
{
    return false;
}


bool LocalLogManager::getLogs(const LogFileFilter& filter, std::vector<LogFile>& files)
{
    return false;
}

bool LocalLogManager::getLogs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogFile>& files)
{
    return false;
}


void LocalLogManager::createLogger(const std::string& name)
{
    if (loggers.contains(name)) return;

    auto logger = std::make_shared<Logger>(name, this);
    loggers.add(name, logger);
}

void LocalLogManager::save()
{
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



