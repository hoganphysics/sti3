#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/LogManager.h>
#include <sti/device/Logger.h>
#include <sti/device/LogFileFilter.h>

#include "LocalLogManager.h"
#include "LocalPersistenceManager.h"
#include "fileholder_tests_support.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>

using STI::Device::LocalDevice;
using STI::Device::LocalLogManager;
using STI::Device::LocalPersistenceManager;
using STI::Device::Logger;
using STI::Device::LogFile;
using STI::Device::LogFileFilter;
using STI::Utils::Configuration;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;
using STI::Utils::TimeStamp;

namespace {

Configuration makePersistenceConfig(const std::filesystem::path& rootPath, const std::string& subdir) {
    Configuration config;
    config.set("PersistenceManager", "root path", rootPath.string());
    config.set("PersistenceManager", "device subdirectory", subdir);
    config.set("EngineManager", "Engine Count", 0);
    return config;
}

std::shared_ptr<LocalPersistenceManager> getPersistence(const std::shared_ptr<LocalDevice>& device) {
    std::shared_ptr<STI::Device::PersistenceManager> pm;
    device->getPersistenceManager(pm);
    return std::dynamic_pointer_cast<LocalPersistenceManager>(pm);
}

std::filesystem::path ensureLogDir(const std::shared_ptr<LocalPersistenceManager>& persistence, const STI::Device::DeviceID& id, const TimeStamp& ts) {
    std::string logDevicePath;
    REQUIRE(persistence->makeLogPath(ts, id, logDevicePath));
    std::filesystem::create_directories(logDevicePath);
    return std::filesystem::path(logDevicePath);
}

bool waitForLogFile(const std::filesystem::path& dir, std::filesystem::path& found, std::chrono::milliseconds timeout) {
    using namespace std::chrono_literals;
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < timeout) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".log") {
                found = entry.path();
                return true;
            }
        }
        std::this_thread::sleep_for(50ms);
    }
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".log") {
            found = entry.path();
            return true;
        }
    }
    return false;
}

class LoggerTestDevice : public LocalDevice {
public:
    explicit LoggerTestDevice(const Configuration& config, const std::string& subdir)
        : LocalDevice("LoggerDevice", "127.0.0.1", 10, "target", config), readValue(42) {}

    bool writeChannel(short, const MixedValue&) override { return true; }

    bool readChannel(short, const MixedValue&, MixedValue& data) override {
        data = readValue;
        return true;
    }

    void setReadValue(const MixedValue& value) { readValue = value; }

private:
    MixedValue readValue;
};

std::string readLogFile(const std::filesystem::path& path) {
    return fileholder_test_support::readFileToString(path);
}

} // namespace

TEST_CASE("Logger streams group messages and persist to log file") {
    using namespace std::chrono_literals;

    fileholder_test_support::TempDir tempDir;
    auto config = makePersistenceConfig(tempDir.path, "logger-stream");
    auto device = std::make_shared<LoggerTestDevice>(config, "logger-stream");

    std::shared_ptr<LocalLogManager> logManager;
    REQUIRE(device->getLogManager(logManager));
    auto persistence = getPersistence(device);
    REQUIRE(persistence);

    TimeStamp now;
    auto logDir = ensureLogDir(persistence, device->getID(), now);

    auto& logger = logManager->log("stream");
    logger << "alpha";
    logger << "beta";

    std::filesystem::path logPath;
    REQUIRE(waitForLogFile(logDir, logPath, 5s));
    auto contents = readLogFile(logPath);
    CHECK(contents.find("alpha") != std::string::npos);
    CHECK(contents.find("beta") != std::string::npos);
}

TEST_CASE("Logger addLogTask registers task and writes output") {
    using namespace std::chrono_literals;

    fileholder_test_support::TempDir tempDir;
    auto config = makePersistenceConfig(tempDir.path, "logger-task");
    auto device = std::make_shared<LoggerTestDevice>(config, "logger-task");

    std::shared_ptr<LocalLogManager> logManager;
    REQUIRE(device->getLogManager(logManager));
    auto persistence = getPersistence(device);
    REQUIRE(persistence);

    TimeStamp now;
    auto logDir = ensureLogDir(persistence, device->getID(), now);

    auto& logger = logManager->log("task");
    logger.addLogTask("00:00:01", []() { return std::string("task-run"); });

    std::shared_ptr<STI::Device::TaskManager> taskManager;
    REQUIRE(device->getTaskManager(taskManager));

    std::set<std::string> ids;
    taskManager->getTaskIDs(ids);
    REQUIRE_FALSE(ids.empty());

    // Execute all registered tasks to ensure log output is generated.
    for (const auto& id : ids) {
        taskManager->runTask(id);
    }

    std::filesystem::path logPath;
    REQUIRE(waitForLogFile(logDir, logPath, 5s));
    auto contents = readLogFile(logPath);
    CHECK(contents.find("task-run") != std::string::npos);
}

TEST_CASE("Logger addReadLogTask logs channel reads with value and channel metadata") {
    using namespace std::chrono_literals;

    fileholder_test_support::TempDir tempDir;
    auto config = makePersistenceConfig(tempDir.path, "logger-read");
    auto device = std::make_shared<LoggerTestDevice>(config, "logger-read");

    std::shared_ptr<LocalLogManager> logManager;
    REQUIRE(device->getLogManager(logManager));
    auto persistence = getPersistence(device);
    REQUIRE(persistence);

    TimeStamp now;
    auto logDir = ensureLogDir(persistence, device->getID(), now);

    auto& channel = device->addInputChannel(1, MixedValueType::Double, "temperature");
    channel.setChannelName("temperature");

    MixedValue readValue;
    readValue.setValue(123.45);
    device->setReadValue(readValue);

    auto& logger = logManager->log("reader");
    logger.addReadLogTask(1, "00:00:01");

    std::shared_ptr<STI::Device::TaskManager> taskManager;
    REQUIRE(device->getTaskManager(taskManager));

    std::set<std::string> ids;
    taskManager->getTaskIDs(ids);
    auto it = std::find_if(ids.begin(), ids.end(), [](const std::string& id) {
        return id.find("Read channel #1") != std::string::npos;
    });
    REQUIRE(it != ids.end());

    taskManager->runTask(*it);

    std::filesystem::path logPath;
    REQUIRE(waitForLogFile(logDir, logPath, 5s));
    auto contents = readLogFile(logPath);
    CHECK(contents.find("|read|") != std::string::npos);
    CHECK(contents.find("temperature") != std::string::npos);
    CHECK(contents.find("123.45") != std::string::npos);
}
