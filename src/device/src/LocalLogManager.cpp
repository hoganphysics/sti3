#include "LocalLogManager.h"

#include <sti/device/DeviceID.h>
#include <sti/device/Logger.h>
#include <sti/LocalDevice.h>
#include <sti/utils/utils.h>

#include "LocalAttributeManager.h"
#include "LocalChannelManager.h"
#include "LocalLogRepository.h"
#include "LocalPersistenceManager.h"
#include "LocalTaskManager.h"

#include <sstream>

using STI::Device::DeviceID;
using STI::Device::LocalLogManager;
using STI::Device::LocalLogRepository;
using STI::Device::LocalPersistenceManager;
using STI::Device::LogFile;
using STI::Device::LogFileFilter;
using STI::Device::LogID;
using STI::Device::LogManager;
using STI::Device::LogRecord;
using STI::Device::Logger;
using STI::Utils::TimeStamp;


LocalLogManager::LocalLogManager(
    LocalDevice* localDevice,
    const std::shared_ptr<LocalPersistenceManager>& localPersistenceManager)
: localDevice(localDevice),
  localPersistenceManager(localPersistenceManager),
  localLogRepository(std::make_unique<LocalLogRepository>(localDevice, localPersistenceManager)),
  logWriterMessageGrouper(this)
{
    createLogger("");   // default logger

    logWriterMessageGrouper.setWarmup(1000);      // ms
    logWriterMessageGrouper.setCooldown(1000);    // ms
    logWriterMessageGrouper.start();
}

LocalLogManager::~LocalLogManager()
{
    logWriterMessageGrouper.stop();
}

void LocalLogManager::writeLog(const std::string& logName)
{
    if (localPersistenceManager == nullptr || localLogRepository == nullptr) {
        return;
    }

    std::string logDevicePath;
    TimeStamp timestamp;

    if (!localPersistenceManager->makeLogPath(timestamp, localDevice->getID(), logDevicePath)) {
        return;
    }

    std::shared_ptr<Logger> logger;
    if (!loggers.get(logName, logger) || logger == nullptr) {
        return;
    }

    if (!logger->save(logDevicePath)) {
        return;
    }

    localLogRepository->recordLocalWrite(timestamp, logger->logFilename);
}

bool LocalLogManager::getLogRecord(const std::string& date, LogRecord& record)
{
    if (localLogRepository == nullptr) {
        return false;
    }

    return localLogRepository->getLogRecord(date, record);
}

void LocalLogManager::getLogNames(std::set<std::string>& names)
{
    if (localLogRepository == nullptr) {
        names.clear();
        return;
    }

    localLogRepository->getLogNames(names);
}

int LocalLogManager::getLogCount(const LogFileFilter& filter)
{
    if (localLogRepository == nullptr) {
        return 0;
    }

    return localLogRepository->getLogCount(filter);
}

int LocalLogManager::getLogCount(const DeviceID& deviceID, const LogFileFilter& filter)
{
    if (localDevice == nullptr || deviceID != localDevice->getID()) {
        return 0;
    }

    return getLogCount(filter);
}

std::string LocalLogManager::makeLogFilename(const std::string& logName, int index)
{
    const std::string logbasename = "sti";
    const std::string extension = "log";

    std::stringstream filename;

    filename << logbasename;
    if (!logName.empty()) {
        filename << "_" << logName;
    }
    filename << "_" << STI::Utils::valueToString(index) << "." << extension;

    return filename.str();
}

void LocalLogManager::getLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids)
{
    if (localLogRepository == nullptr) {
        ids.clear();
        return;
    }

    localLogRepository->getLogIDs(filter, ids);
}

void LocalLogManager::getLogIDs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogID>& ids)
{
    if (localDevice == nullptr || deviceID != localDevice->getID()) {
        return;
    }

    getLogIDs(filter, ids);
}

bool LocalLogManager::getLog(const LogID& id, LogFile& logFile)
{
    if (localLogRepository == nullptr) {
        return false;
    }

    return localLogRepository->getLog(id, logFile);
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
    if (localLogRepository == nullptr) {
        return false;
    }

    return localLogRepository->getLogs(filter, files);
}

bool LocalLogManager::getLogs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogFile>& files)
{
    if (localDevice == nullptr || deviceID != localDevice->getID()) {
        return false;
    }

    return getLogs(filter, files);
}

void LocalLogManager::getNetworkLogNames(std::set<std::string>& names)
{
    getLogNames(names);

    std::shared_ptr<STI::Device::DeviceCollection> collection;
    localDevice->getCollection(collection);

    if (collection == nullptr) {
        return;
    }

    std::set<DeviceID> deviceIDs;
    collection->getIDs(deviceIDs);

    for (const auto& deviceID : deviceIDs) {
        if (deviceID == localDevice->getID()) {
            continue;
        }

        std::shared_ptr<STI::Device::Device> device;
        std::shared_ptr<LogManager> logManager;
        if (!collection->get(deviceID, device) || device == nullptr) {
            continue;
        }
        if (!device->getLogManager(logManager) || logManager == nullptr) {
            continue;
        }

        std::set<std::string> remoteNames;
        logManager->getLogNames(remoteNames);
        names.insert(remoteNames.begin(), remoteNames.end());
    }
}

int LocalLogManager::getNetworkLogCount(const LogFileFilter& filter)
{
    int count = getLogCount(filter);

    std::shared_ptr<STI::Device::DeviceCollection> collection;
    localDevice->getCollection(collection);

    if (collection == nullptr) {
        return count;
    }

    std::set<DeviceID> deviceIDs;
    collection->getIDs(deviceIDs);

    for (const auto& deviceID : deviceIDs) {
        if (deviceID == localDevice->getID()) {
            continue;
        }

        std::shared_ptr<STI::Device::Device> device;
        std::shared_ptr<LogManager> logManager;
        if (!collection->get(deviceID, device) || device == nullptr) {
            continue;
        }
        if (!device->getLogManager(logManager) || logManager == nullptr) {
            continue;
        }

        count += logManager->getLogCount(filter);
    }

    return count;
}

void LocalLogManager::getNetworkLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids)
{
    getLogIDs(filter, ids);

    std::shared_ptr<STI::Device::DeviceCollection> collection;
    localDevice->getCollection(collection);

    if (collection == nullptr) {
        return;
    }

    std::set<DeviceID> deviceIDs;
    collection->getIDs(deviceIDs);

    for (const auto& deviceID : deviceIDs) {
        if (deviceID == localDevice->getID()) {
            continue;
        }

        std::shared_ptr<STI::Device::Device> device;
        std::shared_ptr<LogManager> logManager;
        if (!collection->get(deviceID, device) || device == nullptr) {
            continue;
        }
        if (!device->getLogManager(logManager) || logManager == nullptr) {
            continue;
        }

        logManager->getLogIDs(filter, ids);
    }
}

bool LocalLogManager::getNetworkLogs(const LogFileFilter& filter, std::vector<LogFile>& files)
{
    bool success = getLogs(filter, files);

    std::shared_ptr<STI::Device::DeviceCollection> collection;
    localDevice->getCollection(collection);

    if (collection == nullptr) {
        return success;
    }

    std::set<DeviceID> deviceIDs;
    collection->getIDs(deviceIDs);

    for (const auto& deviceID : deviceIDs) {
        if (deviceID == localDevice->getID()) {
            continue;
        }

        std::shared_ptr<STI::Device::Device> device;
        std::shared_ptr<LogManager> logManager;
        if (!collection->get(deviceID, device) || device == nullptr) {
            success = false;
            continue;
        }
        if (!device->getLogManager(logManager) || logManager == nullptr) {
            success = false;
            continue;
        }

        success &= logManager->getLogs(filter, files);
    }

    return success;
}

void LocalLogManager::createLogger(const std::string& name)
{
    if (loggers.contains(name)) return;

    auto logger = std::make_shared<Logger>(name, this);
    loggers.add(name, logger);
}

bool LocalLogManager::getLogger(const std::string& name, std::shared_ptr<Logger>& logger) const
{
    return loggers.get(name, logger) && (logger != 0);
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
