#include "JLogManager.h"

#include <sti/device/LogID.h>
#include <sti/device/LogRecord.h>
#include <sti/device/LogFile.h>
#include <sti/device/LogFileFilter.h>


using STI::Device::JLogManager;
using STI::Device::LogID;
using STI::Device::LogFileFilter;
using STI::Device::LogFile;
using STI::Device::LogRecord;
using STI::Device::DeviceID;


JLogManager::JLogManager(const std::shared_ptr<STI::Device::LogManager>& manager)
: logManager(manager)
{
}

JLogManager::~JLogManager()
{
}


std::set<std::string> JLogManager::getLogNames()
{
    std::set<std::string> names;
    if (logManager != 0) {
        logManager->getLogNames(names);
    }
    return names;
}


int JLogManager::getLogCount(const LogFileFilter& filter)
{
    if (logManager != 0) {
        return logManager->getLogCount(filter);
    }
    return 0;
}

int JLogManager::getLogCount(const DeviceID& deviceID, const LogFileFilter& filter)
{
    if (logManager != 0) {
        return logManager->getLogCount(deviceID, filter);
    }
    return 0;
}


std::vector<LogID> JLogManager::getLogIDs(const LogFileFilter& filter)
{
    std::vector<LogID> logIDs;
    if (logManager != 0) {
        logManager->getLogIDs(filter, logIDs);
    }
    return logIDs;
}

std::vector<LogID> JLogManager::getLogIDs(const DeviceID& deviceID, const LogFileFilter& filter)
{
    std::vector<LogID> logIDs;
    if (logManager != 0) {
        logManager->getLogIDs(deviceID, filter, logIDs);
    }
    return logIDs;
}


LogFile JLogManager::getLog(const LogID& id)
{
    LogFile file;
    if (logManager != 0) {
        logManager->getLog(id, file);
    }
    return file;
}

LogFile JLogManager::getLog(const std::string& name, const std::string& date, unsigned index)
{
    LogFile file;
    if (logManager != 0) {
        logManager->getLog(name, date, index, file);
    }
    return file;
}


std::vector<LogFile> JLogManager::getLogs(const LogFileFilter& filter)
{
    std::vector<LogFile> files;
    if (logManager != 0) {
        logManager->getLogs(filter, files);
    }
    return files;
}

std::vector<LogFile> JLogManager::getLogs(const DeviceID& deviceID, const LogFileFilter& filter)
{
    std::vector<LogFile> files;
    if (logManager != 0) {
        logManager->getLogs(deviceID, filter, files);
    }
    return files;
}


LogRecord JLogManager::getLogRecord(const std::string& date)
{
    LogRecord record;
    if (logManager != 0) {
        logManager->getLogRecord(date, record);
    }
    return record;
}
