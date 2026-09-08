#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/Device.h>
#include <sti/device/DeviceMessageReceiver.h>
#include <sti/utils/LocalCollection.h>

#include "../../src/device/src/LocalDeviceMessageDispatcher.h"

#include "devicemessage_tests_support.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

using device_message_test_support::CountingListener;
using device_message_test_support::RefreshCountingListener;
using device_message_test_support::makeAttributeMessage;
using device_message_test_support::makeDeviceID;
using STI::Device::AttributeUpdateMessage;
using STI::Device::Device;
using STI::Device::DeviceID;
using STI::Device::DeviceMessageReceiver;
using STI::Device::LocalDevice;
using STI::Device::LocalDeviceMessageDispatcher;
using STI::Device::RefreshDeviceMessage;
using STI::Utils::LocalCollectionListener;

namespace {

class ReceiverTestDevice : public LocalDevice {
public:
    ReceiverTestDevice(const std::string& name, unsigned short module)
        : LocalDevice(name, "127.0.0.1", module, "receiver-test") {}
};

class CollectionRemovalListener : public LocalCollectionListener<DeviceID> {
public:
    void add(const DeviceID&) override {}

    void remove(const DeviceID&) override {
        {
            std::lock_guard<std::mutex> lock(mutex);
            ++removeCount;
        }
        cv.notify_all();
    }

    void refresh() override {}

    bool waitForRemoval(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return removeCount > 0; });
    }

private:
    std::size_t removeCount{0};
    std::mutex mutex;
    std::condition_variable cv;
};

} // namespace

TEST_CASE("DeviceMessageReceiver wires listeners to local dispatcher") {
    auto localDispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto devices = std::make_shared<STI::Utils::LocalCollection<DeviceID, STI::Device::Device>>();
    DeviceID localID = makeDeviceID("receiver");

    DeviceMessageReceiver receiver(localID, devices, localDispatcher);

    auto listener = std::make_shared<CountingListener>();
    receiver.addListener<AttributeUpdateMessage>(localID, "listener", listener);

    auto message = makeAttributeMessage(localID, "k", "v1");
    localDispatcher->addMessage(message);

    REQUIRE(listener->waitFor(1, std::chrono::milliseconds(500)));
    CHECK(listener->count == 1);

    // Remove and ensure messages no longer delivered.
    receiver.removeListener(localID,
                            STI::Device::DeviceMessageListenerID(AttributeUpdateMessage::getMessageClassType(),
                                                                 "listener"));
    auto message2 = makeAttributeMessage(localID, "k", "v2");
    localDispatcher->addMessage(message2);
    CHECK(listener->waitFor(2, std::chrono::milliseconds(250)) == false);
    CHECK(listener->count == 1);
}

TEST_CASE("DeviceMessageReceiver installs pending remote listeners when source is collected",
          "[devicemessagereceiver][device]") {
    auto localDispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto devices = std::make_shared<STI::Utils::LocalCollection<DeviceID, Device>>();
    DeviceID localID = makeDeviceID("receiver");
    auto remoteDevice = std::make_shared<ReceiverTestDevice>("remote", 9);
    auto remoteID = remoteDevice->getID();
    auto listener = std::make_shared<RefreshCountingListener>();
    auto collectionRemovalListener = std::make_shared<CollectionRemovalListener>();

    {
        DeviceMessageReceiver receiver(localID, devices, localDispatcher);
        // Register this after the receiver so observing the removal also proves
        // that any earlier receiver callback has finished.
        devices->addListener(collectionRemovalListener);
        receiver.addListener<RefreshDeviceMessage>(remoteID, "pending-refresh", listener);

        remoteDevice->sendMessage(std::make_shared<RefreshDeviceMessage>(remoteID));
        CHECK_FALSE(listener->waitFor(1, std::chrono::milliseconds(150)));

        REQUIRE(devices->add(remoteID, remoteDevice));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        remoteDevice->sendMessage(std::make_shared<RefreshDeviceMessage>(remoteID));

        REQUIRE(listener->waitFor(1, std::chrono::milliseconds(500)));
        CHECK(listener->count == 1);
    }

    REQUIRE(devices->remove(remoteID));
    REQUIRE(collectionRemovalListener->waitForRemoval(std::chrono::milliseconds(500)));

    remoteDevice->sendMessage(std::make_shared<RefreshDeviceMessage>(remoteID));
    CHECK_FALSE(listener->waitFor(2, std::chrono::milliseconds(250)));
}

TEST_CASE("DeviceMessageReceiver clearListeners drops handlers") {
    auto localDispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto devices = std::make_shared<STI::Utils::LocalCollection<DeviceID, STI::Device::Device>>();
    DeviceID localID = makeDeviceID("receiver");

    DeviceMessageReceiver receiver(localID, devices, localDispatcher);
    auto listener = std::make_shared<CountingListener>();
    receiver.addListener<AttributeUpdateMessage>(localID, "listener", listener);

    receiver.clearListeners();

    auto message = makeAttributeMessage(localID, "k", "v");
    localDispatcher->addMessage(message);
    CHECK(listener->waitFor(1, std::chrono::milliseconds(250)) == false);
    CHECK(listener->count == 0);
}
