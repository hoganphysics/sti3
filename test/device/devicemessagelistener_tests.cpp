#include <catch2/catch_test_macros.hpp>

#include <sti/device/DeviceMessageListenerGroup.h>

#include "devicemessage_tests_support.h"

using device_message_test_support::RefreshCountingListener;
using device_message_test_support::makeAttributeMessage;
using device_message_test_support::makeDeviceID;
using STI::Device::DeviceMessage;
using STI::Device::DeviceMessageListenerGroup;
using STI::Device::RefreshDeviceMessage;

TEST_CASE("DeviceMessageListenerGroup dispatches only matching message types") {
    DeviceMessageListenerGroup<RefreshDeviceMessage> group;
    auto listener = std::make_shared<RefreshCountingListener>();
    auto listenerBase =
        std::static_pointer_cast<STI::Device::DeviceMessageListener<RefreshDeviceMessage>>(listener);
    group.addListener(STI::Device::DeviceMessageListenerID(RefreshDeviceMessage::getMessageClassType(), "one"),
                      listenerBase);
    REQUIRE(group.size() == 1);

    auto refresh = std::make_shared<RefreshDeviceMessage>(makeDeviceID("src"));
    std::shared_ptr<DeviceMessage> refreshBase = refresh;
    group.handleMessage(refreshBase);
    CHECK(listener->count == 1);

    // Different type should be ignored.
    auto attr = makeAttributeMessage(makeDeviceID("src"), "k", "v");
    std::shared_ptr<DeviceMessage> attrBase = attr;
    group.handleMessage(attrBase);
    CHECK(listener->count == 1);
}
