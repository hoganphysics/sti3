#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/AutoMonitor.h>
#include <sti/device/MonitorManager.h>
#include <sti/utils/Task.h>

#include "../../src/device/src/LocalTaskManager.h"
#include "task_tests_support.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <set>
#include <string>
#include <thread>

using STI::Device::AutoMonitor;
using STI::Device::LocalDevice;
using STI::Device::LocalTaskManager;
using STI::Device::Monitor;
using STI::Device::MonitorManager;
using STI::Utils::Configuration;
using STI::Utils::MixedValue;
using STI::Utils::TaskStatus;

namespace {

Configuration makeAutoMonitorConfig() {
    Configuration config;
    config.set("EngineManager", "Engine Count", 0);
    return config;
}

class AutoMonitorTestDevice : public LocalDevice {
public:
    AutoMonitorTestDevice()
        : LocalDevice("AutoMonitorDevice", "127.0.0.1", 1, "target", makeAutoMonitorConfig()) {}
};

} // namespace

TEST_CASE("AutoMonitor: interval task updates value and follows monitor activation") {
    auto taskManager = std::make_shared<LocalTaskManager>();

    int counter = 0;
    auto monitor = AutoMonitor::create(
        "Status/direct",
        1.0,
        [&]() { return MixedValue(++counter); },
        taskManager);

    std::set<std::string> taskIDs;
    taskManager->getTaskIDs(taskIDs);
    REQUIRE(taskIDs.size() == 1);

    const auto taskID = *taskIDs.begin();
    CHECK(taskManager->getTaskStatus(taskID) == TaskStatus::Active);

    taskManager->runTask(taskID);
    CHECK(monitor->getValue() == MixedValue(1));

    monitor->deactivate();
    CHECK(taskManager->getTaskStatus(taskID) == TaskStatus::Inactive);

    taskManager->runTask(taskID);
    CHECK(monitor->getValue() == MixedValue(1));

    monitor->activate();
    CHECK(taskManager->getTaskStatus(taskID) == TaskStatus::Active);

    taskManager->runTask(taskID);
    CHECK(monitor->getValue() == MixedValue(2));
}

TEST_CASE("AutoMonitor: reactivation resumes scheduled updates") {
    using namespace std::chrono_literals;

    auto taskManager = std::make_shared<LocalTaskManager>();
    std::atomic<int> counter{0};

    auto monitor = AutoMonitor::create(
        "Status/reactivate",
        0.1,
        [&]() { return MixedValue(counter.fetch_add(1) + 1); },
        taskManager);

    const auto taskID = std::string("Monitor:Status/reactivate:AutoUpdate");

    REQUIRE(task_test_support::waitForAtomicCount(counter, 1, 300ms));
    const auto valueBeforeDeactivate = monitor->getValue();

    monitor->deactivate();
    CHECK(taskManager->getTaskStatus(taskID) == TaskStatus::Inactive);

    const auto updatesWhileActive = counter.load();
    std::this_thread::sleep_for(200ms);
    CHECK(counter.load() == updatesWhileActive);
    CHECK(monitor->getValue() == valueBeforeDeactivate);

    monitor->activate();
    CHECK(taskManager->getTaskStatus(taskID) == TaskStatus::Active);

    REQUIRE(task_test_support::waitForAtomicCount(counter, updatesWhileActive + 1, 300ms));
    CHECK(monitor->getValue() != valueBeforeDeactivate);
}

TEST_CASE("AutoMonitor: destruction removes the associated task from the task manager") {
    auto taskManager = std::make_shared<LocalTaskManager>();
    std::string taskID;

    {
        auto monitor = AutoMonitor::create(
            "Status/cleanup",
            1.0,
            []() { return MixedValue("done"); },
            taskManager);

        std::set<std::string> taskIDs;
        taskManager->getTaskIDs(taskIDs);
        REQUIRE(taskIDs.size() == 1);
        taskID = *taskIDs.begin();
    }

    CHECK(taskManager->getTaskStatus(taskID) == TaskStatus::Missing);
}

TEST_CASE("LocalDevice: addAutoMonitor wires the monitor and task managers transparently") {
    auto device = std::make_shared<AutoMonitorTestDevice>();

    auto& monitor = device->addAutoMonitor(
        "Status/device",
        1.0,
        []() { return MixedValue(7); });

    std::shared_ptr<STI::Device::TaskManager> taskManager;
    REQUIRE(device->getTaskManager(taskManager));

    std::set<std::string> taskIDs;
    taskManager->getTaskIDs(taskIDs);
    REQUIRE(taskIDs.size() == 1);

    taskManager->runTask(*taskIDs.begin());
    CHECK(monitor.getValue() == MixedValue(7));

    std::shared_ptr<MonitorManager> monitorManager;
    REQUIRE(device->getMonitorManager(monitorManager));

    std::shared_ptr<Monitor> fetched;
    REQUIRE(monitorManager->getMonitor("Status/device", fetched));
    REQUIRE(fetched != nullptr);
    CHECK(fetched->getValue() == MixedValue(7));
}
