#include <catch2/catch_test_macros.hpp>

#include "../../src/device/src/LocalDeviceMessageDispatcher.h"
#include "../../src/device/src/LocalDeviceMessageHandler.h"
#include <sti/device/DeviceMessageListenerGroup.h>

#include "devicemessage_tests_support.h"

using device_message_test_support::CountingListener;
using device_message_test_support::makeAttributeMessage;
using device_message_test_support::makeDeviceID;
using STI::Device::AttributeUpdateMessage;
using STI::Device::DeviceMessageListenerGroup;
using STI::Device::LocalDeviceMessageDispatcher;
using STI::Device::LocalDeviceMessageHandler;

TEST_CASE("LocalDeviceMessageDispatcher only notifies handlers with listeners") {
    LocalDeviceMessageDispatcher dispatcher;
    auto id = makeDeviceID("local");

    auto handlerWithListener = std::make_shared<LocalDeviceMessageHandler>();
    auto handlerWithoutListener = std::make_shared<LocalDeviceMessageHandler>();
    dispatcher.addMessageHandler(id, handlerWithListener);
    dispatcher.addMessageHandler(makeDeviceID("other"), handlerWithoutListener);

    auto listener = std::make_shared<CountingListener>();
    auto listenerBase =
        std::static_pointer_cast<STI::Device::DeviceMessageListener<AttributeUpdateMessage>>(listener);
    auto group = std::make_shared<DeviceMessageListenerGroup<AttributeUpdateMessage>>();
    group->addListener(STI::Device::DeviceMessageListenerID(AttributeUpdateMessage::getMessageClassType(), "l"),
                       listenerBase);
    std::shared_ptr<STI::Device::AbstractMessageListenerGroup> abstractGroup = group;
    handlerWithListener->addListenerGroup(AttributeUpdateMessage::getMessageClassType(), abstractGroup);

    auto message = makeAttributeMessage(id, "key", "val");
    dispatcher.addMessage(message);

    REQUIRE(listener->waitFor(1, std::chrono::milliseconds(500)));
    CHECK(listener->count == 1);
}
