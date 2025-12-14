#include <catch2/catch_test_macros.hpp>

#include "../../src/device/src/LocalDeviceMessageHandler.h"
#include <sti/device/DeviceMessageListenerGroup.h>

#include "devicemessage_tests_support.h"

using device_message_test_support::CountingListener;
using device_message_test_support::makeAttributeMessage;
using device_message_test_support::makeDeviceID;
using STI::Device::AttributeUpdateMessage;
using STI::Device::DeviceMessageListenerGroup;
using STI::Device::LocalDeviceMessageHandler;

TEST_CASE("LocalDeviceMessageHandler queues and dispatches to listener groups") {
    LocalDeviceMessageHandler handler;

    auto listener = std::make_shared<CountingListener>();
    auto listenerBase =
        std::static_pointer_cast<STI::Device::DeviceMessageListener<AttributeUpdateMessage>>(listener);
    auto group = std::make_shared<DeviceMessageListenerGroup<AttributeUpdateMessage>>();
    group->addListener(STI::Device::DeviceMessageListenerID(AttributeUpdateMessage::getMessageClassType(), "l"),
                       listenerBase);
    std::shared_ptr<STI::Device::AbstractMessageListenerGroup> abstractGroup = group;
    handler.addListenerGroup(AttributeUpdateMessage::getMessageClassType(), abstractGroup);

    auto message = makeAttributeMessage(makeDeviceID("src"), "key", "val");
    REQUIRE(handler.hasListeners(message));

    handler.addMessage(message);
    REQUIRE(listener->waitFor(1, std::chrono::milliseconds(500)));
    REQUIRE(listener->count == 1);
    CHECK(listener->seen.front().at("key") == "val");

    // Non-listened type ignored.
    auto refresh = std::make_shared<STI::Device::RefreshDeviceMessage>(makeDeviceID("src"));
    CHECK(handler.hasListeners(refresh) == false);
}
