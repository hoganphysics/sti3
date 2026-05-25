#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDeviceHub.h>
#include <sti/device/Device.h>
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/utils/LocalCollection.h>
#include <sti/utils/MetaData.h>
#include <sti/utils/MixedValue.h>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>

using STI::Device::Attribute;
using STI::Device::AttributeManager;
using STI::Device::ChannelManager;
using STI::Device::Device;
using STI::Device::DeviceID;
using STI::Device::DeviceMessageDispatcher;
using STI::Device::DeviceMessageListenerForwarder;
using STI::Device::LogManager;
using STI::Device::MonitorManager;
using STI::Device::PersistenceManager;
using STI::Device::ProfileManager;
using STI::Device::TaskManager;
using STI::Network::Hub;
using STI::Network::HubID;
using STI::Network::LocalDeviceHub;
using STI::Utils::Collection;
using STI::Utils::LocalCollection;
using STI::Utils::LocalCollectionListener;
using STI::Utils::MixedValue;

namespace {

DeviceID makeDeviceID(const std::string& name, unsigned short module)
{
    return DeviceID(name, "127.0.0.1", module, "hub-test");
}

class HubTestDevice : public Device {
public:
    explicit HubTestDevice(const DeviceID& id)
        : id(id), collection(std::make_shared<LocalCollection<DeviceID, Device>>()) {}

    const DeviceID getID() const override { return id; }
    void kill() override { alive = false; }
    const MixedValue& getMetaData() const override { return metaData.getMetaData(); }
    MixedValue getMetaData(const std::string& key) const override { return metaData.getMetaData(key); }

    void getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher) override { dispatcher.reset(); }
    bool getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler) override
    {
        scheduler.reset();
        return false;
    }
    void getChannelManager(std::shared_ptr<ChannelManager>& manager) override { manager.reset(); }
    void getAttributeManager(std::shared_ptr<AttributeManager>& manager) override { manager.reset(); }
    bool getMonitorManager(std::shared_ptr<MonitorManager>& manager) override
    {
        manager.reset();
        return false;
    }
    bool getPersistenceManager(std::shared_ptr<PersistenceManager>& manager) override
    {
        manager.reset();
        return false;
    }
    bool getProfileManager(std::shared_ptr<ProfileManager>& manager) override
    {
        manager.reset();
        return false;
    }
    bool getTaskManager(std::shared_ptr<TaskManager>& manager) override
    {
        manager.reset();
        return false;
    }
    bool getLogManager(std::shared_ptr<LogManager>& manager) override
    {
        manager.reset();
        return false;
    }

    void attachMessageListenerForwarder(const std::shared_ptr<DeviceMessageListenerForwarder>&) override {}

    bool write(short, const MixedValue&) override { return false; }
    bool read(short, MixedValue&) override { return false; }
    bool read(short, const MixedValue&, MixedValue&) override { return false; }
    void stopRW() override {}
    std::string getAttribute(const std::string&) override { return {}; }
    bool setAttribute(const std::string&, const std::string&) override { return false; }
    bool getAttribute(const std::string&, std::shared_ptr<Attribute>& attribute) override
    {
        attribute.reset();
        return false;
    }

    bool refresh() override
    {
        ++refreshCount;
        return alive;
    }
    void activate() override
    {
        active = true;
        ++activateCount;
    }
    void disable() override
    {
        active = false;
        alive = false;
        ++disableCount;
    }
    bool addto(const HubID&) override { return true; }
    void setRemoveCB(const std::function<void(void)>& remover) override { removeCB = remover; }

    void getCollection(std::shared_ptr<Collection<DeviceID, Device>>& out) override { out = collection; }

    bool collectionContains(const DeviceID& otherID) const { return collection->contains(otherID); }
    void addCollectionListener(const std::shared_ptr<LocalCollectionListener<DeviceID>>& listener)
    {
        collection->addListener(listener);
    }

    bool alive = true;
    bool active = false;
    unsigned activateCount = 0;
    unsigned disableCount = 0;
    unsigned refreshCount = 0;
    std::function<void(void)> removeCB;

private:
    DeviceID id;
    std::shared_ptr<LocalCollection<DeviceID, Device>> collection;
    STI::Utils::MetaData metaData;
};

class RecordingCollectionListener : public LocalCollectionListener<DeviceID> {
public:
    void add(const DeviceID&) override
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            ++addCount_;
        }
        cv.notify_all();
    }

    void remove(const DeviceID&) override {}
    void refresh() override {}

    bool waitForAddCount(unsigned expected, std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return addCount_ >= expected; });
    }

    unsigned addCount() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return addCount_;
    }

private:
    mutable std::mutex mutex;
    std::condition_variable cv;
    unsigned addCount_ = 0;
};

} // namespace

TEST_CASE("LocalDeviceHub addNode forwards to connected hubs without duplicate local add events", "[localdevicehub][network]")
{
    auto hub = std::make_shared<LocalDeviceHub>(HubID("Hub", "127.0.0.1", 1));

    auto dev1 = std::make_shared<HubTestDevice>(makeDeviceID("dev1", 1));
    auto dev2 = std::make_shared<HubTestDevice>(makeDeviceID("dev2", 2));

    auto dev1Listener = std::make_shared<RecordingCollectionListener>();
    auto dev2Listener = std::make_shared<RecordingCollectionListener>();
    dev1->addCollectionListener(dev1Listener);
    dev2->addCollectionListener(dev2Listener);

    REQUIRE(hub->addNode(dev1->getID(), dev1));
    REQUIRE(hub->addNode(dev2->getID(), dev2));

    REQUIRE(dev1Listener->waitForAddCount(1, std::chrono::milliseconds(500)));
    REQUIRE(dev2Listener->waitForAddCount(1, std::chrono::milliseconds(500)));

    CHECK_FALSE(dev1Listener->waitForAddCount(2, std::chrono::milliseconds(100)));
    CHECK_FALSE(dev2Listener->waitForAddCount(2, std::chrono::milliseconds(100)));
    CHECK(dev1Listener->addCount() == 1);
    CHECK(dev2Listener->addCount() == 1);
}

TEST_CASE("LocalDeviceHub distributes devices across connected hubs", "[localdevicehub][network]")
{
    auto hub1 = std::make_shared<LocalDeviceHub>(HubID("Hub1", "127.0.0.1", 1));
    auto hub2 = std::make_shared<LocalDeviceHub>(HubID("Hub2", "127.0.0.1", 2));

    auto dev1 = std::make_shared<HubTestDevice>(makeDeviceID("dev1", 1));
    auto dev2 = std::make_shared<HubTestDevice>(makeDeviceID("dev2", 2));
    auto dev3 = std::make_shared<HubTestDevice>(makeDeviceID("dev3", 3));
    auto dev4 = std::make_shared<HubTestDevice>(makeDeviceID("dev4", 4));

    REQUIRE(hub1->addNode(dev1->getID(), dev1));
    REQUIRE(hub1->addNode(dev2->getID(), dev2));
    REQUIRE(hub2->addNode(dev3->getID(), dev3));

    REQUIRE(Hub<DeviceID, Device>::connect(hub1, hub2));
    CHECK(hub1->isConnectedTo(hub2->getID()));
    CHECK(hub2->isConnectedTo(hub1->getID()));

    CHECK(dev1->collectionContains(dev2->getID()));
    CHECK(dev1->collectionContains(dev3->getID()));
    CHECK_FALSE(dev1->collectionContains(dev1->getID()));
    CHECK(dev3->collectionContains(dev1->getID()));
    CHECK(dev3->collectionContains(dev2->getID()));

    REQUIRE(hub2->addNode(dev4->getID(), dev4));
    CHECK(dev1->collectionContains(dev4->getID()));
    CHECK(dev4->collectionContains(dev1->getID()));
    CHECK(dev4->collectionContains(dev2->getID()));
    CHECK(dev4->collectionContains(dev3->getID()));

    CHECK(dev1->active);
    CHECK(dev2->active);
    CHECK(dev3->active);
    CHECK(dev4->active);
    CHECK(dev4->activateCount == 1);
}

TEST_CASE("LocalDeviceHub propagates node removal across connected hubs", "[localdevicehub][network]")
{
    auto hub1 = std::make_shared<LocalDeviceHub>(HubID("Hub1", "127.0.0.1", 1));
    auto hub2 = std::make_shared<LocalDeviceHub>(HubID("Hub2", "127.0.0.1", 2));

    auto dev1 = std::make_shared<HubTestDevice>(makeDeviceID("dev1", 1));
    auto dev2 = std::make_shared<HubTestDevice>(makeDeviceID("dev2", 2));
    auto dev3 = std::make_shared<HubTestDevice>(makeDeviceID("dev3", 3));

    REQUIRE(hub1->addNode(dev1->getID(), dev1));
    REQUIRE(hub2->addNode(dev2->getID(), dev2));
    REQUIRE(Hub<DeviceID, Device>::connect(hub1, hub2));
    REQUIRE(hub2->addNode(dev3->getID(), dev3));

    CHECK(dev1->collectionContains(dev3->getID()));
    CHECK(dev2->collectionContains(dev3->getID()));

    REQUIRE(hub2->removeNode(dev3->getID()));
    CHECK_FALSE(hub1->hasNodeID(dev3->getID()));
    CHECK_FALSE(hub2->hasNodeID(dev3->getID()));
    CHECK_FALSE(dev1->collectionContains(dev3->getID()));
    CHECK_FALSE(dev2->collectionContains(dev3->getID()));
    CHECK_FALSE(dev3->active);
    CHECK(dev3->disableCount >= 1);
}

TEST_CASE("LocalDeviceHub refresh removes dead references from collected devices", "[localdevicehub][network]")
{
    auto hub1 = std::make_shared<LocalDeviceHub>(HubID("Hub1", "127.0.0.1", 1));
    auto hub2 = std::make_shared<LocalDeviceHub>(HubID("Hub2", "127.0.0.1", 2));

    auto dev1 = std::make_shared<HubTestDevice>(makeDeviceID("dev1", 1));
    auto dev2 = std::make_shared<HubTestDevice>(makeDeviceID("dev2", 2));

    REQUIRE(hub1->addNode(dev1->getID(), dev1));
    REQUIRE(hub2->addNode(dev2->getID(), dev2));
    REQUIRE(Hub<DeviceID, Device>::connect(hub1, hub2));

    REQUIRE(dev1->collectionContains(dev2->getID()));

    dev2->alive = false;

    REQUIRE(hub1->refresh());

    CHECK_FALSE(dev1->collectionContains(dev2->getID()));
    CHECK_FALSE(hub2->hasNodeID(dev2->getID()));
    CHECK_FALSE(dev2->active);
    CHECK(dev2->disableCount >= 1);
}
