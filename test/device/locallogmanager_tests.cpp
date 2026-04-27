#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/Device.h>
#include <sti/device/LogFileFilter.h>
#include <sti/device/LogManager.h>
#include <sti/device/LogRecord.h>
#include <sti/utils/LocalCollection.h>

#include "LocalLogManager.h"
#include "LocalPersistenceManager.h"
#include "LogRecordFile.h"
#include "fileholder_tests_support.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>

using STI::Device::DeviceID;
using STI::Device::Device;
using STI::Device::LocalDevice;
using STI::Device::LocalLogManager;
using STI::Device::LocalPersistenceManager;
using STI::Device::LogFile;
using STI::Device::LogFileFilter;
using STI::Device::LogID;
using STI::Device::LogManager;
using STI::Device::LogRecord;
using STI::Utils::Configuration;
using STI::Utils::TimeStamp;

namespace {

Configuration makePersistenceConfig(const std::filesystem::path& rootPath, const std::string& deviceSubdir) {
    Configuration config;
    config.set("PersistenceManager", "root path", rootPath.string());
    config.set("PersistenceManager", "device subdirectory", deviceSubdir);
    // Keep background engine threads minimal for tests.
    config.set("EngineManager", "Engine Count", 0);
    return config;
}

std::shared_ptr<LocalPersistenceManager> getPersistenceManager(const std::shared_ptr<LocalDevice>& device) {
    std::shared_ptr<STI::Device::PersistenceManager> persistence;
    device->getPersistenceManager(persistence);
    return std::dynamic_pointer_cast<LocalPersistenceManager>(persistence);
}

bool waitForLogFile(const std::filesystem::path& dir, std::filesystem::path& foundPath, std::chrono::milliseconds timeout) {
    using namespace std::chrono_literals;
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < timeout) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                foundPath = entry.path();
                return true;
            }
        }
        std::this_thread::sleep_for(50ms);
    }
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".log") {
            foundPath = entry.path();
            return true;
        }
    }
    return false;
}

class FixedRemoteLogManager : public LogManager {
public:
    explicit FixedRemoteLogManager(const LogID& id) : id_(id) {}

    void getLogNames(std::set<std::string>& names) override {
        names.insert(id_.logName);
    }

    int getLogCount(const LogFileFilter& filter) override {
        return matches(filter) ? 1 : 0;
    }

    int getLogCount(const DeviceID& deviceID, const LogFileFilter& filter) override {
        return (deviceID == id_.deviceID && matches(filter)) ? 1 : 0;
    }

    void getLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids) override {
        if (matches(filter)) {
            ids.push_back(id_);
        }
    }

    void getLogIDs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogID>& ids) override {
        if (deviceID == id_.deviceID && matches(filter)) {
            ids.push_back(id_);
        }
    }

    bool getLog(const LogID& id, LogFile& logFile) override {
        if (id != id_) {
            return false;
        }
        logFile.id = id_;
        logFile.type = LogFile::LogFileType::String;
        logFile.logString = "remote log";
        return true;
    }

    bool getLog(const std::string& name, const std::string& date, unsigned index, LogFile& logFile) override {
        if (name != id_.logName || date != id_.date || index != id_.index) {
            return false;
        }
        return getLog(id_, logFile);
    }

    bool getLogs(const LogFileFilter& filter, std::vector<LogFile>& files) override {
        if (!matches(filter)) {
            return true;
        }
        LogFile file;
        getLog(id_, file);
        files.push_back(file);
        return true;
    }

    bool getLogs(const DeviceID& deviceID, const LogFileFilter& filter, std::vector<LogFile>& files) override {
        if (deviceID != id_.deviceID) {
            return false;
        }
        return getLogs(filter, files);
    }

    bool getLogRecord(const std::string&, LogRecord&) override {
        return false;
    }

    void getNetworkLogNames(std::set<std::string>& names) override {
        getLogNames(names);
    }

    int getNetworkLogCount(const LogFileFilter& filter) override {
        return getLogCount(filter);
    }

    void getNetworkLogIDs(const LogFileFilter& filter, std::vector<LogID>& ids) override {
        getLogIDs(filter, ids);
    }

    bool getNetworkLogs(const LogFileFilter& filter, std::vector<LogFile>& files) override {
        return getLogs(filter, files);
    }

private:
    bool matches(const LogFileFilter& filter) const {
        if (filter.logName != "*" && filter.logName != id_.logName) {
            return false;
        }
        return filter.startDate <= id_.date && id_.date <= filter.endDate;
    }

    LogID id_;
};

class FixedRemoteDevice : public Device {
public:
    FixedRemoteDevice(const DeviceID& id, const std::shared_ptr<LogManager>& logManager)
        : id_(id), logManager_(logManager) {}

    const DeviceID getID() const override { return id_; }
    void kill() override {}

    void getMessageDispatcher(std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher) override { dispatcher.reset(); }
    bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler) override {
        scheduler.reset();
        return false;
    }
    void getChannelManager(std::shared_ptr<STI::Device::ChannelManager>& manager) override { manager.reset(); }
    void getAttributeManager(std::shared_ptr<STI::Device::AttributeManager>& manager) override { manager.reset(); }
    bool getMonitorManager(std::shared_ptr<STI::Device::MonitorManager>& manager) override {
        manager.reset();
        return false;
    }
    bool getPersistenceManager(std::shared_ptr<STI::Device::PersistenceManager>& manager) override {
        manager.reset();
        return false;
    }
    bool getProfileManager(std::shared_ptr<STI::Device::ProfileManager>& manager) override {
        manager.reset();
        return false;
    }
    bool getTaskManager(std::shared_ptr<STI::Device::TaskManager>& manager) override {
        manager.reset();
        return false;
    }
    bool getLogManager(std::shared_ptr<LogManager>& manager) override {
        manager = logManager_;
        return manager != nullptr;
    }

    void attachMessageListenerForwarder(const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>&) override {}
    bool addto(const STI::Network::HubID&) override { return true; }
    void setRemoveCB(const std::function<void(void)>&) override {}

    bool refresh() override { return true; }
    void activate() override {}
    void disable() override {}
    bool write(short, const STI::Utils::MixedValue&) override { return false; }
    bool read(short, STI::Utils::MixedValue&) override { return false; }
    bool read(short, const STI::Utils::MixedValue&, STI::Utils::MixedValue&) override { return false; }
    void stopRW() override {}
    std::string getAttribute(const std::string&) override { return {}; }
    bool setAttribute(const std::string&, const std::string&) override { return false; }
    bool getAttribute(const std::string&, std::shared_ptr<STI::Device::Attribute>& attribute) override {
        attribute.reset();
        return false;
    }
    void getCollection(std::shared_ptr<STI::Utils::Collection<DeviceID, Device>>& collection) override {
        collection.reset();
    }

private:
    DeviceID id_;
    std::shared_ptr<LogManager> logManager_;
};

} // namespace

TEST_CASE("LocalLogManager reports persisted log names instead of live logger names") {
    using namespace std::chrono_literals;

    fileholder_test_support::TempDir tempDir;
    auto config = makePersistenceConfig(tempDir.path, "locallogmanager-names");
    std::string dateStr;

    {
        auto device = std::make_shared<LocalDevice>("LogDevice", "127.0.0.1", 1, "target", config);
        std::shared_ptr<LocalLogManager> logManager;
        REQUIRE(device->getLogManager(logManager));
        REQUIRE(logManager);

        std::set<std::string> names;
        logManager->getLogNames(names);
        CHECK(names.empty());

        logManager->createLogger("custom");

        names.clear();
        logManager->getLogNames(names);
        CHECK(names.empty());

        std::shared_ptr<STI::Device::Logger> logger;
        REQUIRE(logManager->getLogger("custom", logger));
        CHECK(logger->getName() == "custom");

        auto persistence = getPersistenceManager(device);
        REQUIRE(persistence);

        TimeStamp now;
        std::string logDevicePath;
        REQUIRE(persistence->makeLogPath(now, device->getID(), logDevicePath));
        std::filesystem::path logDir(logDevicePath);

        auto& persistedLogger = logManager->log("custom");
        persistedLogger << "persisted entry";

        std::filesystem::path createdLog;
        REQUIRE(waitForLogFile(logDir, createdLog, 5s));

        names.clear();
        logManager->getLogNames(names);
        CHECK(names.count("custom") == 1);
        CHECK(names.size() == 1);

        dateStr = now.date_YYYY_MM_DD("/");
    }

    {
        auto reloadedDevice = std::make_shared<LocalDevice>("LogDevice", "127.0.0.1", 1, "target", config);
        std::shared_ptr<LocalLogManager> reloadedLogManager;
        REQUIRE(reloadedDevice->getLogManager(reloadedLogManager));

        std::set<std::string> names;
        reloadedLogManager->getLogNames(names);
        CHECK(names.count("custom") == 1);

        LogFileFilter filter{"custom", dateStr, dateStr, 0, -1};
        CHECK(reloadedLogManager->getLogCount(filter) == 1);
    }
}

TEST_CASE("LocalLogManager writes logs, records metadata, and returns counts and ids") {
    using namespace std::chrono_literals;

    fileholder_test_support::TempDir tempDir;
    auto config = makePersistenceConfig(tempDir.path, "locallogmanager-logs");
    auto device = std::make_shared<LocalDevice>("LogDevice", "127.0.0.1", 2, "target", config);

    std::shared_ptr<LocalLogManager> logManager;
    REQUIRE(device->getLogManager(logManager));
    auto persistence = getPersistenceManager(device);
    REQUIRE(persistence);

    TimeStamp now;
    std::string logDevicePath;
    REQUIRE(persistence->makeLogPath(now, device->getID(), logDevicePath));
    std::filesystem::path logDir(logDevicePath);

    auto& logger = logManager->log("custom");
    logger << "first entry";

    std::filesystem::path createdLog;
    REQUIRE(waitForLogFile(logDir, createdLog, 5s));
    REQUIRE(std::filesystem::exists(createdLog));

    auto contents = fileholder_test_support::readFileToString(createdLog);
    CHECK(contents.find("first entry") != std::string::npos);

    auto dateStr = now.date_YYYY_MM_DD("/");
    LogRecord record;
    REQUIRE(logManager->getLogRecord(dateStr, record));

    auto recordIt = record.deviceLogRecords.find(device->getID().getID());
    REQUIRE(recordIt != record.deviceLogRecords.end());
    CHECK(recordIt->second.status == STI::Device::LogRecordStatus::LogsPresent);
    CHECK(recordIt->second.logNames.count("custom") == 1);
    REQUIRE(recordIt->second.logs.count("custom") == 1);
    CHECK(recordIt->second.logs.at("custom").files.size() == 1);
    CHECK(recordIt->second.logs.at("custom").totalBytes > 0);
    CHECK(recordIt->second.logs.at("custom").totalLines >= 1);

    std::set<std::string> names;
    logManager->getLogNames(names);
    CHECK(names.count("custom") == 1);

    LogFileFilter filter{"custom", dateStr, dateStr, 0, 0};
    CHECK(logManager->getLogCount(filter) == 1);

    std::vector<LogFile> files;
    REQUIRE(logManager->getLogs(filter, files));
    REQUIRE(files.size() == 1);
    CHECK(files.front().fileHolder);
    CHECK(files.front().fileHolder->exists());
}

TEST_CASE("LocalLogManager rebuilds metadata from existing log files") {
    fileholder_test_support::TempDir tempDir;
    auto config = makePersistenceConfig(tempDir.path, "locallogmanager-counts");
    auto device = std::make_shared<LocalDevice>("LogDevice", "127.0.0.1", 3, "target", config);

    std::shared_ptr<LocalLogManager> logManager;
    REQUIRE(device->getLogManager(logManager));
    auto persistence = getPersistenceManager(device);
    REQUIRE(persistence);

    TimeStamp targetDate(2024, 1, 2, 0, 0, 0, 0, 0, 0);
    std::string logBasePath;
    REQUIRE(persistence->makeLogPath(targetDate, logBasePath));
    std::string logDevicePath;
    REQUIRE(persistence->makeLogPath(targetDate, device->getID(), logDevicePath));

    std::filesystem::create_directories(logDevicePath);
    const std::vector<std::string> filenames = {"sti_0.log", "sti_named_0.log", "sti_named_2.log", "sti_named_5.log"};
    for (const auto& name : filenames) {
        std::ofstream file(std::filesystem::path(logDevicePath) / name);
        file << name << std::endl;
    }

    auto dateStr = targetDate.date_YYYY_MM_DD("/");
    std::set<std::string> names;
    logManager->getLogNames(names);
    CHECK(names.count("") == 1);
    CHECK(names.count("named") == 1);

    LogFileFilter namedFilter{"named", dateStr, dateStr, 0, -1};
    CHECK(logManager->getLogCount(namedFilter) == 3);

    LogFileFilter wildcardFilter{"*", dateStr, dateStr, 0, -1};
    CHECK(logManager->getLogCount(wildcardFilter) == 4);

    std::vector<LogID> ids;
    logManager->getLogIDs(namedFilter, ids);
    REQUIRE(ids.size() == 3);
    CHECK(ids[0].index == 0);
    CHECK(ids[1].index == 2);
    CHECK(ids[2].index == 5);
    CHECK(ids[0].logName == "named");

    std::vector<LogID> lastOnly;
    LogFileFilter lastFilter{"named", dateStr, dateStr, -1, -1};
    logManager->getLogIDs(lastFilter, lastOnly);
    REQUIRE(lastOnly.size() == 1);
    CHECK(lastOnly.front().index == 5);

    LogFile logFile;
    REQUIRE(logManager->getLog(ids[1], logFile));
    CHECK(logFile.fileHolder);
    CHECK(logFile.fileHolder->exists());

    LogRecord record;
    REQUIRE(logManager->getLogRecord(dateStr, record));
    auto recordIt = record.deviceLogRecords.find(device->getID().getID());
    REQUIRE(recordIt != record.deviceLogRecords.end());
    CHECK(recordIt->second.logs.count("named") == 1);
    CHECK(recordIt->second.logs.at("named").files.size() == 3);
}

TEST_CASE("LocalLogManager network methods aggregate connected device logs", "[log] [network]") {
    using LocalCollection = STI::Utils::LocalCollection<DeviceID, Device>;

    fileholder_test_support::TempDir tempDir;
    auto config = makePersistenceConfig(tempDir.path, "locallogmanager-network");
    auto device = std::make_shared<LocalDevice>("LogDevice", "127.0.0.1", 4, "target", config);

    std::shared_ptr<LocalLogManager> logManager;
    REQUIRE(device->getLogManager(logManager));

    TimeStamp now;
    auto dateStr = now.date_YYYY_MM_DD("/");

    auto persistence = getPersistenceManager(device);
    REQUIRE(persistence);
    std::string logDevicePath;
    REQUIRE(persistence->makeLogPath(now, device->getID(), logDevicePath));
    std::filesystem::path logDir(logDevicePath);

    auto& localLogger = logManager->log("local");
    localLogger << "local entry";

    std::filesystem::path createdLog;
    REQUIRE(waitForLogFile(logDir, createdLog, std::chrono::seconds(5)));

    std::shared_ptr<STI::Device::DeviceCollection> collectionBase;
    device->getCollection(collectionBase);
    auto collection = std::dynamic_pointer_cast<LocalCollection>(collectionBase);
    REQUIRE(collection);

    DeviceID remoteID("RemoteDevice", "127.0.0.1", 44, device->getID().getID());
    LogID remoteLogID(remoteID, dateStr, "remote", 0);
    auto remoteLogManager = std::make_shared<FixedRemoteLogManager>(remoteLogID);
    auto remoteDevice = std::make_shared<FixedRemoteDevice>(remoteID, remoteLogManager);
    REQUIRE(collection->add(remoteID, remoteDevice));

    LogFileFilter localFilter{"local", dateStr, dateStr, 0, -1};
    LogFileFilter remoteFilter{"remote", dateStr, dateStr, 0, -1};
    LogFileFilter wildcardFilter{"*", dateStr, dateStr, 0, -1};

    std::set<std::string> names;
    logManager->getNetworkLogNames(names);
    CHECK(names.count("local") == 1);
    CHECK(names.count("remote") == 1);

    CHECK(logManager->getLogCount(remoteID, remoteFilter) == 0);

    CHECK(logManager->getNetworkLogCount(localFilter) >= 1);
    CHECK(logManager->getNetworkLogCount(remoteFilter) == 1);

    std::vector<LogID> ids;
    logManager->getNetworkLogIDs(wildcardFilter, ids);
    CHECK(std::any_of(ids.begin(), ids.end(), [](const LogID& id) { return id.logName == "remote"; }));
    CHECK(std::any_of(ids.begin(), ids.end(), [](const LogID& id) { return id.logName == "local"; }));

    std::vector<LogFile> files;
    REQUIRE(logManager->getNetworkLogs(remoteFilter, files));
    REQUIRE(files.size() == 1);
    CHECK(files.front().type == LogFile::LogFileType::String);
    CHECK(files.front().logString == "remote log");

    REQUIRE(collection->remove(remoteID));
}
