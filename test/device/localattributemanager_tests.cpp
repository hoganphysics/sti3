#include <catch2/catch_test_macros.hpp>

#include "../../src/device/src/LocalAttributeManager.h"
#include "../../src/device/src/LocalDeviceMessageDispatcher.h"
#include "../../src/device/src/LocalDeviceMessageHandler.h"
#include <sti/device/AttributeManager.h>
#include <sti/device/DeviceMessageListenerGroup.h>
#include <sti/device/LocalAttribute.h>

#include "localattribute_tests_support.h"

#include <algorithm>
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

TEST_CASE("LocalAttributeManager: refreshValue syncs member-backed attributes", "[localattributemanager][attribute]") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    int alpha = 1;
    int beta = 10;

    auto attrAlpha = std::make_shared<LocalAttribute>("alpha", std::to_string(alpha));
    auto attrBeta = std::make_shared<LocalAttribute>("beta", std::to_string(beta));
    attrAlpha->setRefresher([&]() { return std::to_string(alpha); });
    attrBeta->setRefresher([&]() { return std::to_string(beta); });

    REQUIRE(manager.addAttribute(attrAlpha));
    REQUIRE(manager.addAttribute(attrBeta));

    alpha = 2;
    beta = 11;

    CHECK(manager.getValue("alpha") == "1");
    CHECK(manager.getValue("beta") == "10");

    CHECK(manager.refreshValue("alpha"));
    CHECK(manager.getValue("alpha") == "2");
    CHECK(manager.getValue("beta") == "10");
    CHECK_FALSE(manager.refreshValue("missing"));

    alpha = 3;
    manager.refreshValues();

    CHECK(manager.getValue("alpha") == "3");
    CHECK(manager.getValue("beta") == "11");
}

TEST_CASE("LocalAttributeManager: refresh groups refresh every member") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    int alphaValue = 1;
    int betaValue = 10;
    int alphaRefreshes = 0;
    int betaRefreshes = 0;
    auto alpha = std::make_shared<LocalAttribute>("alpha", "1");
    auto beta = std::make_shared<LocalAttribute>("beta", "10");
    alpha->setRefresher([&](std::string& value) {
        ++alphaRefreshes;
        value = std::to_string(alphaValue);
        return true;
    });
    beta->setRefresher([&](std::string& value) {
        ++betaRefreshes;
        value = std::to_string(betaValue);
        return true;
    });
    REQUIRE(manager.addAttribute(alpha));
    REQUIRE(manager.addAttribute(beta));
    manager.addAttributeRefreshGroup({"alpha", "beta"});

    alphaValue = 2;
    betaValue = 11;
    REQUIRE(manager.refreshValue("alpha"));

    CHECK(alphaRefreshes == 1);
    CHECK(betaRefreshes == 1);
    CHECK(manager.getValue("alpha") == "2");
    CHECK(manager.getValue("beta") == "11");
}

TEST_CASE("LocalAttributeManager: refresh group cycles and duplicates refresh each attribute once") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    std::map<std::string, int> refreshes;
    for (const auto& key : {"a", "b", "c"}) {
        auto attribute = std::make_shared<LocalAttribute>(key, key);
        attribute->setRefresher([&, key](std::string& value) {
            ++refreshes[key];
            value = key;
            return true;
        });
        REQUIRE(manager.addAttribute(attribute));
    }

    manager.addAttributeRefreshGroup({"a", "b", "a"});
    manager.addAttributeRefreshGroup({"b", "c"});
    manager.addAttributeRefreshGroup({"c", "a"});

    REQUIRE(manager.refreshValue("a"));
    CHECK(refreshes == std::map<std::string, int>{{"a", 1}, {"b", 1}, {"c", 1}});
}

TEST_CASE("LocalAttributeManager: overlapping refresh groups are transitive") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    std::vector<std::string> refreshed;
    for (const auto& key : {"a", "b", "c", "outside"}) {
        auto attribute = std::make_shared<LocalAttribute>(key, key);
        attribute->setRefresher([&, key](std::string& value) {
            refreshed.push_back(key);
            value = key;
            return true;
        });
        REQUIRE(manager.addAttribute(attribute));
    }
    manager.addAttributeRefreshGroup({"a", "b"});
    manager.addAttributeRefreshGroup({"b", "c"});

    REQUIRE(manager.refreshValue("c"));
    CHECK(std::set<std::string>(refreshed.begin(), refreshed.end()) == std::set<std::string>{"a", "b", "c"});
    CHECK(refreshed.size() == 3);
}

TEST_CASE("LocalAttributeManager: grouped refresh emits updates only for changed values") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    std::string alphaValue = "same";
    std::string betaValue = "same";
    auto alpha = std::make_shared<LocalAttribute>("alpha", alphaValue);
    auto beta = std::make_shared<LocalAttribute>("beta", betaValue);
    local_attribute_test_support::RefreshRecorder alphaEvents;
    local_attribute_test_support::RefreshRecorder betaEvents;
    alpha->addRefreshListener(&alphaEvents);
    beta->addRefreshListener(&betaEvents);
    alpha->setRefresher([&]() { return alphaValue; });
    beta->setRefresher([&]() { return betaValue; });
    REQUIRE(manager.addAttribute(alpha));
    REQUIRE(manager.addAttribute(beta));
    manager.addAttributeRefreshGroup({"alpha", "beta"});

    REQUIRE(manager.refreshValue("alpha"));
    CHECK(alphaEvents.count == 0);
    CHECK(betaEvents.count == 0);

    betaValue = "changed";
    REQUIRE(manager.refreshValue("alpha"));
    CHECK(alphaEvents.count == 0);
    CHECK(betaEvents.count == 1);
    CHECK(betaEvents.lastValue == "changed");
}

TEST_CASE("LocalAttributeManager: grouped refresh continues after failures and aggregates success") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    std::vector<std::string> attempts;
    for (const auto& key : {"a", "b", "c"}) {
        auto attribute = std::make_shared<LocalAttribute>(key, key);
        attribute->setRefresher([&, key](std::string& value) {
            attempts.push_back(key);
            value = std::string(key) + "-new";
            return key != std::string("b");
        });
        REQUIRE(manager.addAttribute(attribute));
    }
    manager.addAttributeRefreshGroup({"a", "b", "c"});

    CHECK_FALSE(manager.refreshValue("a"));
    CHECK(std::set<std::string>(attempts.begin(), attempts.end()) == std::set<std::string>{"a", "b", "c"});
    CHECK(attempts.size() == 3);
    CHECK(manager.getValue("a") == "a-new");
    CHECK(manager.getValue("b") == "b");
    CHECK(manager.getValue("c") == "c-new");
}

TEST_CASE("LocalAttributeManager: missing grouped keys fail without stopping known refreshes") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    int refreshes = 0;
    auto attribute = std::make_shared<LocalAttribute>("known", "value");
    attribute->setRefresher([&](std::string& value) {
        ++refreshes;
        value = "value";
        return true;
    });
    REQUIRE(manager.addAttribute(attribute));
    manager.addAttributeRefreshGroup({"known", "missing"});

    CHECK_FALSE(manager.refreshValue("known"));
    CHECK(refreshes == 1);
    CHECK_FALSE(manager.refreshValue("missing"));
    CHECK(refreshes == 2);
    CHECK_FALSE(manager.setValue("missing", "value"));
}

TEST_CASE("LocalAttributeManager: setting a group member runs its setter before one refresh of every member") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    std::string alphaHardware = "old-a";
    std::string betaHardware = "old-b";
    std::vector<std::string> operations;
    auto alpha = std::make_shared<LocalAttribute>("alpha", alphaHardware);
    auto beta = std::make_shared<LocalAttribute>("beta", betaHardware);
    alpha->setSetter([&](const std::string& value) {
        operations.push_back("setter");
        alphaHardware = value;
        betaHardware = "from-setter";
        return true;
    });
    alpha->setRefresher([&](std::string& value) {
        operations.push_back("alpha-refresh");
        value = alphaHardware;
        return true;
    });
    beta->setRefresher([&](std::string& value) {
        operations.push_back("beta-refresh");
        value = betaHardware;
        return true;
    });
    REQUIRE(manager.addAttribute(alpha));
    REQUIRE(manager.addAttribute(beta));
    manager.addAttributeRefreshGroup({"alpha", "beta"});

    REQUIRE(manager.setValue("alpha", "new-a"));
    REQUIRE(operations.front() == "setter");
    CHECK(std::count(operations.begin(), operations.end(), "alpha-refresh") == 1);
    CHECK(std::count(operations.begin(), operations.end(), "beta-refresh") == 1);
    CHECK(manager.getValue("alpha") == "new-a");
    CHECK(manager.getValue("beta") == "from-setter");
}

TEST_CASE("LocalAttributeManager: failed setters still refresh the complete group") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    int alphaRefreshes = 0;
    int betaRefreshes = 0;
    auto alpha = std::make_shared<LocalAttribute>("alpha", "old");
    auto beta = std::make_shared<LocalAttribute>("beta", "old");
    alpha->setSetter([](const std::string&) { return false; });
    alpha->setRefresher([&](std::string& value) {
        ++alphaRefreshes;
        value = "hardware-a";
        return true;
    });
    beta->setRefresher([&](std::string& value) {
        ++betaRefreshes;
        value = "hardware-b";
        return true;
    });
    REQUIRE(manager.addAttribute(alpha));
    REQUIRE(manager.addAttribute(beta));
    manager.addAttributeRefreshGroup({"alpha", "beta"});

    CHECK_FALSE(manager.setValue("alpha", "rejected"));
    CHECK(alphaRefreshes == 1);
    CHECK(betaRefreshes == 1);
    CHECK(manager.getValue("alpha") == "hardware-a");
    CHECK(manager.getValue("beta") == "hardware-b");
}

TEST_CASE("LocalAttributeManager: grouped setters dispatch updates for every changed member") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    auto handler = std::make_shared<LocalDeviceMessageHandler>();
    dispatcher->addMessageHandler(makeDeviceID(), handler);

    auto listenerGroup = std::make_shared<DeviceMessageListenerGroup<AttributeUpdateMessage>>();
    std::shared_ptr<STI::Device::AbstractMessageListenerGroup> abstractGroup = listenerGroup;
    auto listener = std::make_shared<AttributeUpdateRecorder>();
    auto listenerBase = std::static_pointer_cast<STI::Device::DeviceMessageListener<AttributeUpdateMessage>>(listener);
    listenerGroup->addListener(
        STI::Device::DeviceMessageListenerID(AttributeUpdateMessage::getMessageClassType(), "group-test"),
        listenerBase);
    handler->addListenerGroup(AttributeUpdateMessage::getMessageClassType(), abstractGroup);

    std::string alphaValue = "a0";
    std::string betaValue = "b0";
    std::string gammaValue = "c0";
    auto alpha = std::make_shared<LocalAttribute>("alpha", alphaValue);
    auto beta = std::make_shared<LocalAttribute>("beta", betaValue);
    auto gamma = std::make_shared<LocalAttribute>("gamma", gammaValue);
    alpha->setSetter([&](const std::string& value) {
        alphaValue = value;
        betaValue = "b1";
        gammaValue = "c1";
        return true;
    });
    alpha->setRefresher([&]() { return alphaValue; });
    beta->setRefresher([&]() { return betaValue; });
    gamma->setRefresher([&]() { return gammaValue; });
    REQUIRE(manager.addAttribute(alpha));
    REQUIRE(manager.addAttribute(beta));
    REQUIRE(manager.addAttribute(gamma));
    manager.addAttributeRefreshGroup({"alpha", "beta", "gamma"});

    REQUIRE(manager.setValue("alpha", "a1"));
    REQUIRE(listener->waitForMessages(1, std::chrono::milliseconds(1500)));

    const auto& updates = listener->received.back();
    REQUIRE(updates.size() == 3);
    CHECK(updates.at("alpha") == "a1");
    CHECK(updates.at("beta") == "b1");
    CHECK(updates.at("gamma") == "c1");
}

TEST_CASE("LocalAttributeManager: grouped explicit refresh dispatches every changed member") {
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    LocalAttributeManager manager(makeDeviceID(), dispatcher);

    auto handler = std::make_shared<LocalDeviceMessageHandler>();
    dispatcher->addMessageHandler(makeDeviceID(), handler);

    auto listenerGroup = std::make_shared<DeviceMessageListenerGroup<AttributeUpdateMessage>>();
    std::shared_ptr<STI::Device::AbstractMessageListenerGroup> abstractGroup = listenerGroup;
    auto listener = std::make_shared<AttributeUpdateRecorder>();
    auto listenerBase = std::static_pointer_cast<STI::Device::DeviceMessageListener<AttributeUpdateMessage>>(listener);
    listenerGroup->addListener(
        STI::Device::DeviceMessageListenerID(AttributeUpdateMessage::getMessageClassType(), "refresh-group-test"),
        listenerBase);
    handler->addListenerGroup(AttributeUpdateMessage::getMessageClassType(), abstractGroup);

    std::map<std::string, std::string> hardware{
        {"alpha", "a0"},
        {"beta", "b0"},
        {"gamma", "c0"}
    };
    for (const auto& key : {"alpha", "beta", "gamma"}) {
        auto attribute = std::make_shared<LocalAttribute>(key, hardware.at(key));
        attribute->setRefresher([&, key]() { return hardware.at(key); });
        REQUIRE(manager.addAttribute(attribute));
    }
    manager.addAttributeRefreshGroup({"alpha", "beta", "gamma"});

    hardware["alpha"] = "a1";
    hardware["beta"] = "b1";
    hardware["gamma"] = "c1";
    REQUIRE(manager.refreshValue("beta"));
    REQUIRE(listener->waitForMessages(1, std::chrono::milliseconds(1500)));

    const auto& updates = listener->received.back();
    REQUIRE(updates.size() == 3);
    CHECK(updates.at("alpha") == "a1");
    CHECK(updates.at("beta") == "b1");
    CHECK(updates.at("gamma") == "c1");
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
