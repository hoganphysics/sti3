#include <catch2/catch_test_macros.hpp>

#include "../../src/device/src/LocalAttributeManager.h"
#include "../../src/device/src/LocalDeviceMessageDispatcher.h"
#include "../../src/device/src/LocalDeviceMessageHandler.h"
#include <sti/device/AttributeManager.h>
#include <sti/device/DeviceMessageListenerGroup.h>
#include <sti/device/LocalAttribute.h>

#include "localattribute_tests_support.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

using local_attribute_test_support::AttributeUpdateRecorder;
using local_attribute_test_support::makeDeviceID;
using STI::Device::Attribute;
using STI::Device::AttributeManager;
using STI::Device::AttributeUpdateMessage;
using STI::Device::DeviceMessageListenerGroup;
using STI::Device::DeviceMessageType;
using STI::Device::LocalAttribute;
using STI::Device::LocalAttributeManager;
using STI::Device::LocalDeviceMessageDispatcher;
using STI::Device::LocalDeviceMessageHandler;

TEST_CASE("LocalAttributeManager: add, get, and aggregate attributes") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    auto attrA = std::make_shared<LocalAttribute>("alpha", "1");
    auto attrB = std::make_shared<LocalAttribute>("beta", "2");
    REQUIRE(manager.addAttribute(attrA));
    REQUIRE(manager.addAttribute(attrB));

    std::shared_ptr<Attribute> fetched;
    REQUIRE(manager.getAttribute("alpha", fetched));
    CHECK(fetched->getValue() == "1");
    CHECK(manager.getValue("beta") == "2");

    std::vector<std::shared_ptr<Attribute>> vectorOut;
    manager.getAttributes(vectorOut);
    CHECK(vectorOut.size() == 2);

    std::map<std::string, std::string> mapOut;
    manager.getAttributes(mapOut);
    CHECK(mapOut["alpha"] == "1");
    CHECK(mapOut["beta"] == "2");
}

TEST_CASE("LocalAttributeManager: setValue updates known attributes only") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    auto attr = std::make_shared<LocalAttribute>("key", "old");
    REQUIRE(manager.addAttribute(attr));

    CHECK(manager.setValue("key", "new"));
    CHECK(manager.getValue("key") == "new");
    CHECK_FALSE(manager.setValue("missing", "value"));
}

TEST_CASE("LocalAttributeManager: profile load/save round trip") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    auto attr1 = std::make_shared<LocalAttribute>("a1", "one");
    auto attr2 = std::make_shared<LocalAttribute>("a2", "two");
    REQUIRE(manager.addAttribute(attr1));
    REQUIRE(manager.addAttribute(attr2));

    auto profile = std::make_shared<STI::Device::Profile>();
    profile->type = STI::Device::ProfileType::Attribute;
    REQUIRE(manager.saveProfile(profile));
    REQUIRE(profile->attributeData.size() == 2);
    CHECK(profile->attributeData["a1"] == "one");
    CHECK(profile->attributeData["a2"] == "two");

    attr1->setValue("changed");
    attr2->setValue("changed");

    REQUIRE(manager.loadProfile(profile));
    CHECK(manager.getValue("a1") == "one");
    CHECK(manager.getValue("a2") == "two");

    // Unknown attribute returns failure.
    auto badProfile = std::make_shared<STI::Device::Profile>();
    badProfile->type = STI::Device::ProfileType::Attribute;
    badProfile->attributeData["unknown"] = "value";
    CHECK_FALSE(manager.loadProfile(badProfile));
}

TEST_CASE("LocalAttributeManager: refresh dispatches AttributeUpdateMessage") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    auto handler = std::make_shared<LocalDeviceMessageHandler>();
    dispatcher->addMessageHandler(makeDeviceID(), handler);

    auto listenerGroup = std::make_shared<DeviceMessageListenerGroup<AttributeUpdateMessage>>();
    std::shared_ptr<STI::Device::AbstractMessageListenerGroup> abstractGroup = listenerGroup;
    auto listener = std::make_shared<AttributeUpdateRecorder>();
    auto listenerBase = std::static_pointer_cast<STI::Device::DeviceMessageListener<AttributeUpdateMessage>>(listener);
    listenerGroup->addListener(
        STI::Device::DeviceMessageListenerID(AttributeUpdateMessage::getMessageClassType(), "test"), listenerBase);
    handler->addListenerGroup(AttributeUpdateMessage::getMessageClassType(), abstractGroup);

    REQUIRE(listenerGroup->size() == 1);

    auto attr = std::make_shared<LocalAttribute>("key", "v0");
    REQUIRE(manager.addAttribute(attr));

    std::set<DeviceMessageType> types;
    handler->getListenerTypes(types);
    REQUIRE(types.count(AttributeUpdateMessage::getMessageClassType()) == 1);
    auto probe = std::make_shared<AttributeUpdateMessage>(makeDeviceID());
    REQUIRE(handler->hasListeners(probe));

    const auto initialCount = listener->received.size();

    auto manual = std::make_shared<AttributeUpdateMessage>(makeDeviceID(), "manual", "value");
    dispatcher->addMessage(manual);
    REQUIRE(listener->waitForMessages(initialCount + 1, std::chrono::milliseconds(1500)));

    REQUIRE(attr->setValue("v1"));
    REQUIRE(listener->waitForMessages(initialCount + 2, std::chrono::milliseconds(1500)));

    auto& attributes = listener->received.back();
    CHECK(attributes.size() == 1);
    CHECK(attributes.at("key") == "v1");
}
