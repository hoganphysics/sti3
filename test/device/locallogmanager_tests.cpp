#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/LogFileFilter.h>
#include <sti/device/LogManager.h>
#include <sti/device/LogRecord.h>

#include "LocalLogManager.h"
#include "LocalPersistenceManager.h"
#include "LogRecordFile.h"
#include "fileholder_tests_support.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>

using STI::Device::DeviceID;
using STI::Device::LocalDevice;
using STI::Device::LocalLogManager;
using STI::Device::LocalPersistenceManager;
using STI::Device::LogFile;
using STI::Device::LogFileFilter;
using STI::Device::LogID;
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

} // namespace

TEST_CASE("LocalLogManager initializes default logger and tracks names") {
    fileholder_test_support::TempDir tempDir;
    auto config = makePersistenceConfig(tempDir.path, "locallogmanager-names");
    auto device = std::make_shared<LocalDevice>("LogDevice", "127.0.0.1", 1, "target", config);

    std::shared_ptr<LocalLogManager> logManager;
    REQUIRE(device->getLogManager(logManager));
    REQUIRE(logManager);

    std::set<std::string> names;
    logManager->getLogNames(names);
    REQUIRE(names.size() == 1);
    CHECK(names.count("") == 1); // default logger

    logManager->createLogger("custom");
    names.clear();
    logManager->getLogNames(names);
    CHECK(names.count("") == 1);
    CHECK(names.count("custom") == 1);
    CHECK(names.size() == 2);

    std::shared_ptr<STI::Device::Logger> logger;
    REQUIRE(logManager->getLogger("custom", logger));
    CHECK(logger->getName() == "custom");
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

    LogFileFilter filter{"custom", dateStr, dateStr, 0, 0};
    CHECK(logManager->getLogCount(filter) == 1);

    std::vector<LogFile> files;
    REQUIRE(logManager->getLogs(filter, files));
    REQUIRE(files.size() == 1);
    CHECK(files.front().fileHolder);
    CHECK(files.front().fileHolder->exists());
}

TEST_CASE("LocalLogManager parses log files for counts and id ranges") {
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

    auto recordPath = std::filesystem::path(logBasePath) / "logRecord.ini";
    STI::Device::LogRecordFile recordFile(recordPath.string());
    recordFile.setLogStatus(device->getID(), "");
    recordFile.setLogStatus(device->getID(), "named");

    std::filesystem::create_directories(logDevicePath);
    const std::vector<std::string> filenames = {"sti_0.log", "sti_named_0.log", "sti_named_2.log", "sti_named_5.log"};
    for (const auto& name : filenames) {
        std::ofstream file(std::filesystem::path(logDevicePath) / name);
        file << name << std::endl;
    }

    auto dateStr = targetDate.date_YYYY_MM_DD("/");
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
}
