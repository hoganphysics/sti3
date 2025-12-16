#include <catch2/catch_test_macros.hpp>

#include <sti/device/DeviceMessageRelayer.h>
#include <sti/device/DeviceMessageReceiver.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/Device.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/DeviceMessageHandler.h>
#include <sti/device/DeviceID.h>
#include <sti/utils/LocalCollection.h>
#include <sti/network/HubID.h>

#include "../../src/device/src/LocalDeviceMessageDispatcher.h"
#include "devicemessage_tests_support.h"

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

using device_message_test_support::makeAttributeMessage;
using device_message_test_support::makeDeviceID;
using STI::Device::AttributeUpdateMessage;
using STI::Device::Device;
using STI::Device::DeviceID;
using STI::Device::DeviceMessage;
using STI::Device::DeviceMessageDispatcher;
using STI::Device::DeviceMessageHandler;
using STI::Device::DeviceMessageReceiver;
using STI::Device::DeviceMessageRelayer;
using STI::Device::DeviceMessageListenerForwarder;
using STI::Device::RefreshDeviceMessage;
using STI::Device::DeviceTrace;
using STI::Device::LocalDeviceMessageDispatcher;
using STI::Device::ChannelManager;
using STI::Device::AttributeManager;
using STI::Device::PersistenceManager;
using STI::Device::ProfileManager;
using STI::Device::TaskManager;
using STI::Device::LogManager;
using STI::Device::Attribute;

namespace {

class RecordingDispatcher : public DeviceMessageDispatcher {
public:
    void addMessageHandler(const DeviceID&, const std::shared_ptr<DeviceMessageHandler>&) override {}
    void removeMessageHandler(const DeviceID&) override {}
    bool makeMessageHandler(std::shared_ptr<DeviceMessageHandler>&) override { return false; }

    void addMessage(const std::shared_ptr<DeviceMessage>& mess) override {
        {
            std::lock_guard<std::mutex> lock(mutex);
            ++count;
            lastMessage = mess;
        }
        cv.notify_all();
    }

    void clearMessages() override {
        std::lock_guard<std::mutex> lock(mutex);
        count = 0;
        lastMessage.reset();
    }

    bool waitForCount(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return count >= expected; });
    }

    std::size_t count{0};
    std::shared_ptr<DeviceMessage> lastMessage;

private:
    std::mutex mutex;
    std::condition_variable cv;
};

class DummyDevice : public Device {
public:
    DummyDevice(const DeviceID& id, const std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
        : id(id), dispatcher(dispatcher) {}

    const DeviceID getID() const override { return id; }
    void kill() override {}

    void getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& out) override { out = dispatcher; }
    bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>&) override { return false; }
    void getChannelManager(std::shared_ptr<ChannelManager>& manager) override { manager.reset(); }
    void getAttributeManager(std::shared_ptr<AttributeManager>& manager) override { manager.reset(); }
    bool getPersistenceManager(std::shared_ptr<PersistenceManager>& manager) override {
        manager.reset();
        return false;
    }
    bool getProfileManager(std::shared_ptr<ProfileManager>& manager) override {
        manager.reset();
        return false;
    }
    bool getTaskManager(std::shared_ptr<TaskManager>& manager) override {
        manager.reset();
        return false;
    }
    bool getLogManager(std::shared_ptr<LogManager>& manager) override {
        manager.reset();
        return false;
    }

    void attachMessageListenerForwarder(const std::shared_ptr<DeviceMessageListenerForwarder>&) override {}

    bool addto(const STI::Network::HubID&) override { return true; }
    void setRemoveCB(const std::function<void(void)>&) override {}

    bool refresh() override { return true; }
    void activate() override {}
    void disable() override {}

    bool write(short, const STI::Utils::MixedValue&) override { return false; }
    bool read(short, STI::Utils::MixedValue&) override { return false; }
    bool read(short, const STI::Utils::MixedValue&, STI::Utils::MixedValue&) override { return false; }
    void stopRW() override {}
    std::string getAttribute(const std::string&) override { return std::string(); }
    bool setAttribute(const std::string&, const std::string&) override { return false; }
    bool getAttribute(const std::string&, std::shared_ptr<Attribute>& attribute) override {
        attribute.reset();
        return false;
    }

    void getCollection(std::shared_ptr<STI::Utils::Collection<DeviceID, Device>>& collection) override {
        collection.reset();
    }

private:
    DeviceID id;
    std::shared_ptr<DeviceMessageDispatcher> dispatcher;
};

} // namespace

TEST_CASE("DeviceMessageRelayer relays messages and appends trace", "[messagerelayer][device]") {
    auto dispatcher = std::make_shared<RecordingDispatcher>();
    auto relayID = makeDeviceID("relay");
    DeviceMessageRelayer<AttributeUpdateMessage> relayer(relayID, dispatcher);

    auto message = makeAttributeMessage(makeDeviceID("source"), "key", "value");
    relayer.handleMessage(message);

    REQUIRE(dispatcher->waitForCount(1, std::chrono::milliseconds(200)));
    CHECK(dispatcher->lastMessage == message);
    CHECK(message->getDeviceTrace().includesID(relayID));
}

TEST_CASE("DeviceMessageRelayer ignores null or looping messages", "[messagerelayer][device]") {
    auto dispatcher = std::make_shared<RecordingDispatcher>();
    auto relayID = makeDeviceID("relay");
    DeviceMessageRelayer<AttributeUpdateMessage> relayer(relayID, dispatcher);

    relayer.handleMessage(std::shared_ptr<AttributeUpdateMessage>());
    CHECK(dispatcher->count == 0);

    DeviceTrace trace(makeDeviceID("remote"));
    trace.addID(relayID);
    auto loopingMessage = std::make_shared<AttributeUpdateMessage>(trace);
    relayer.handleMessage(loopingMessage);
    CHECK(dispatcher->count == 0);
}

TEST_CASE("DeviceMessageRelayer applies filters per message type", "[messagerelayer][device]") {
    auto dispatcher = std::make_shared<RecordingDispatcher>();
    auto relayID = makeDeviceID("relay");
    DeviceMessageRelayer<AttributeUpdateMessage, RefreshDeviceMessage> relayer(relayID, dispatcher);

    std::size_t attrFilters = 0;
    std::size_t refreshFilters = 0;

    relayer.addFilter<AttributeUpdateMessage>(
        [&](const std::shared_ptr<AttributeUpdateMessage>& mess) {
            ++attrFilters;
            return mess->attributes.count("block") == 0;
        });
    relayer.addFilter<RefreshDeviceMessage>(
        [&](const std::shared_ptr<RefreshDeviceMessage>&) {
            ++refreshFilters;
            return false;
        });

    auto allowed = makeAttributeMessage(makeDeviceID("remote"), "ok", "yes");
    relayer.DeviceMessageRelayer<AttributeUpdateMessage>::handleMessage(allowed);
    auto blocked = makeAttributeMessage(makeDeviceID("remote"), "block", "no");
    relayer.DeviceMessageRelayer<AttributeUpdateMessage>::handleMessage(blocked);

    auto refresh = std::make_shared<RefreshDeviceMessage>(DeviceTrace(makeDeviceID("remote")));
    relayer.DeviceMessageRelayer<RefreshDeviceMessage>::handleMessage(refresh);

    REQUIRE(dispatcher->waitForCount(1, std::chrono::milliseconds(200)));
    CHECK(attrFilters == 2);
    CHECK(refreshFilters == 1);
    CHECK(dispatcher->lastMessage == allowed);
}

TEST_CASE("DeviceMessageRelayer hooks into DeviceMessageReceiver", "[messagerelayer][device]") {
    auto relayDispatcher = std::make_shared<RecordingDispatcher>();
    auto localDispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto remoteDispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto relayID = makeDeviceID("local");
    auto remoteID = makeDeviceID("remote");

    auto collection = std::make_shared<STI::Utils::LocalCollection<DeviceID, Device>>();
    auto receiver = std::make_shared<DeviceMessageReceiver>(relayID, collection, localDispatcher);

    auto remoteDevice = std::make_shared<DummyDevice>(remoteID, remoteDispatcher);
    collection->add(remoteID, remoteDevice);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto relayer = std::make_shared<DeviceMessageRelayer<AttributeUpdateMessage>>(relayID, relayDispatcher);
    DeviceMessageRelayer<AttributeUpdateMessage>::addAllListeners(receiver, remoteID, relayer);

    auto message = makeAttributeMessage(remoteID, "k", "v");
    remoteDispatcher->addMessage(message);

    REQUIRE(relayDispatcher->waitForCount(1, std::chrono::milliseconds(500)));

    auto previousCount = relayDispatcher->count;
    DeviceMessageRelayer<AttributeUpdateMessage>::removeAllListeners(receiver, remoteID, relayer);

    auto second = makeAttributeMessage(remoteID, "k2", "v2");
    remoteDispatcher->addMessage(second);
    CHECK_FALSE(relayDispatcher->waitForCount(previousCount + 1, std::chrono::milliseconds(200)));
    CHECK(relayDispatcher->count == previousCount);
}
