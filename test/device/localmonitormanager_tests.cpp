#include <catch2/catch_test_macros.hpp>

#include "../../src/device/src/LocalDeviceMessageDispatcher.h"
#include "../../src/device/src/LocalDeviceMessageHandler.h"
#include "../../src/device/src/LocalMonitorManager.h"
#include <sti/device/DeviceMessageListenerGroup.h>
#include <sti/device/LocalMonitor.h>

#include "localmonitor_tests_support.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

using local_monitor_test_support::MonitorListenerRecorder;
using local_monitor_test_support::MonitorUpdateRecorder;
using local_monitor_test_support::makeDeviceID;
using STI::Device::AbstractMessageListenerGroup;
using STI::Device::DeviceMessageListener;
using STI::Device::DeviceMessageListenerGroup;
using STI::Device::LocalDeviceMessageDispatcher;
using STI::Device::LocalDeviceMessageHandler;
using STI::Device::LocalMonitor;
using STI::Device::LocalMonitorManager;
using STI::Device::Monitor;
using STI::Device::MonitorStatus;
using STI::Device::MonitorUpdateMessage;

TEST_CASE("LocalMonitorManager: add, query, and control monitors") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalMonitorManager manager(makeDeviceID(), dispatcher);

    auto monitorA = std::make_shared<LocalMonitor>("Group/alpha");
    auto monitorB = std::make_shared<LocalMonitor>("Group/beta");
    manager.addMonitor(monitorA);
    manager.addMonitor(monitorB);

    std::vector<std::string> ids;
    REQUIRE(manager.getIDs(ids));
    CHECK(ids.size() == 2);

    std::shared_ptr<Monitor> fetched;
    REQUIRE(manager.getMonitor("Group/alpha", fetched));
    REQUIRE(fetched != nullptr);
    CHECK(fetched->getID() == "Group/alpha");

    std::vector<std::shared_ptr<Monitor>> monitors;
    REQUIRE(manager.getMonitors(monitors));
    CHECK(monitors.size() == 2);

    CHECK(manager.getStatus("Group/alpha") == MonitorStatus::Active);
    REQUIRE(manager.setValue("Group/alpha", STI::Utils::MixedValue("ready")));
    CHECK(manager.getValue("Group/alpha") == STI::Utils::MixedValue("ready"));

    manager.deactivate("Group/alpha");
    CHECK(manager.getStatus("Group/alpha") == MonitorStatus::Inactive);

    manager.activateAll();
    CHECK(manager.getStatus("Group/alpha") == MonitorStatus::Active);
    CHECK(manager.getStatus("Group/beta") == MonitorStatus::Active);
}

TEST_CASE("LocalMonitorManager: forwards monitor events to listeners and grouped messages") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalMonitorManager manager(makeDeviceID(), dispatcher);

    auto handler = std::make_shared<LocalDeviceMessageHandler>();
    dispatcher->addMessageHandler(makeDeviceID(), handler);

    auto listenerGroup = std::make_shared<DeviceMessageListenerGroup<MonitorUpdateMessage>>();
    std::shared_ptr<AbstractMessageListenerGroup> abstractGroup = listenerGroup;
    auto messageListener = std::make_shared<MonitorUpdateRecorder>();
    auto listenerBase = std::static_pointer_cast<DeviceMessageListener<MonitorUpdateMessage>>(messageListener);
    listenerGroup->addListener(
        STI::Device::DeviceMessageListenerID(MonitorUpdateMessage::getMessageClassType(), "monitor-test"), listenerBase);
    handler->addListenerGroup(MonitorUpdateMessage::getMessageClassType(), abstractGroup);

    auto managerListener = std::make_shared<MonitorListenerRecorder>();
    manager.addListener(managerListener);

    std::vector<std::pair<std::string, STI::Utils::MixedValue>> forwardedValues;
    manager.addValueListener([&](const std::string& id, const STI::Utils::MixedValue& value) {
        forwardedValues.emplace_back(id, value);
    });

    auto monitor = std::make_shared<LocalMonitor>("Status/current");
    manager.addMonitor(monitor);

    monitor->setValue(5);

    REQUIRE(managerListener->waitForUpdates(1, std::chrono::milliseconds(200)));
    REQUIRE(messageListener->waitForMessages(1, std::chrono::milliseconds(1500)));

    CHECK(managerListener->lastID == "Status/current");
    CHECK(managerListener->lastValue.getInt() == 5);

    REQUIRE(forwardedValues.size() == 1);
    CHECK(forwardedValues.front().first == "Status/current");
    CHECK(forwardedValues.front().second.getInt() == 5);

    auto& updates = messageListener->received.back();
    REQUIRE(updates.size() == 1);
    CHECK(updates.at("Status/current").getInt() == 5);
    REQUIRE(messageListener->sources.size() == 1);
    CHECK(messageListener->sources.back() == makeDeviceID());
}

TEST_CASE("LocalMonitorManager: removed monitors do not leak updates after re-adding the same ID") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalMonitorManager manager(makeDeviceID(), dispatcher);

    auto handler = std::make_shared<LocalDeviceMessageHandler>();
    dispatcher->addMessageHandler(makeDeviceID(), handler);

    auto listenerGroup = std::make_shared<DeviceMessageListenerGroup<MonitorUpdateMessage>>();
    std::shared_ptr<AbstractMessageListenerGroup> abstractGroup = listenerGroup;
    auto messageListener = std::make_shared<MonitorUpdateRecorder>();
    auto listenerBase = std::static_pointer_cast<DeviceMessageListener<MonitorUpdateMessage>>(messageListener);
    listenerGroup->addListener(
        STI::Device::DeviceMessageListenerID(MonitorUpdateMessage::getMessageClassType(), "monitor-reregister"), listenerBase);
    handler->addListenerGroup(MonitorUpdateMessage::getMessageClassType(), abstractGroup);

    auto original = std::make_shared<LocalMonitor>("Status/shared");
    manager.addMonitor(original);
    manager.removeMonitor("Status/shared");

    auto replacement = std::make_shared<LocalMonitor>("Status/shared");
    manager.addMonitor(replacement);

    original->setValue(1);
    replacement->setValue(2);

    REQUIRE(messageListener->waitForMessages(1, std::chrono::milliseconds(1500)));
    REQUIRE(messageListener->received.size() == 1);
    CHECK(messageListener->received.front().at("Status/shared").getInt() == 2);
}
