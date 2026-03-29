#include <catch2/catch_test_macros.hpp>

#include <sti/device/LocalMonitor.h>

#include "localmonitor_tests_support.h"

#include <chrono>
#include <memory>

using local_monitor_test_support::MonitorListenerRecorder;
using STI::Device::LocalMonitor;
using STI::Device::MonitorStatus;

TEST_CASE("LocalMonitor: stores metadata and pushes active updates") {
    LocalMonitor monitor("Status/temperature");
    auto listener = std::make_shared<MonitorListenerRecorder>();
    monitor.addListener(listener);

    CHECK(monitor.getID() == "Status/temperature");
    CHECK(monitor.getGroup() == "Status");
    CHECK(monitor.getStatus() == MonitorStatus::Active);

    monitor.addMetaData("units", "C");
    CHECK(monitor.getMetaData("units").getString() == "C");

    monitor.setValue(12);
    REQUIRE(listener->waitForUpdates(1, std::chrono::milliseconds(200)));
    CHECK(listener->lastID == "Status/temperature");
    CHECK(listener->lastValue.getInt() == 12);
    CHECK(monitor.getValue() == STI::Utils::MixedValue(12));
}

TEST_CASE("LocalMonitor: inactive monitors retain value without notifying listeners") {
    LocalMonitor monitor("Scope/trace");
    auto listener = std::make_shared<MonitorListenerRecorder>();
    monitor.addListener(listener);

    monitor.deactivate();
    REQUIRE(listener->waitForDeactivations(1, std::chrono::milliseconds(200)));
    CHECK(monitor.getStatus() == MonitorStatus::Inactive);

    monitor.setValue(7);
    CHECK(listener->getUpdateCount() == 0);
    CHECK(monitor.getValue() == STI::Utils::MixedValue(7));

    monitor.activate();
    REQUIRE(listener->waitForActivations(1, std::chrono::milliseconds(200)));
    monitor.setValue(8);
    REQUIRE(listener->waitForUpdates(1, std::chrono::milliseconds(200)));
    CHECK(listener->lastValue.getInt() == 8);
}
