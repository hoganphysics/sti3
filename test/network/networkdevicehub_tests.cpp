#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/LocalDeviceHub.h>
#include <sti/NetworkDeviceHub.h>
#include <sti/device/DeviceCollection.h>
#include <sti/network/HubID.h>
#include <sti/utils/Configuration.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <thread>

using STI::Device::DeviceCollection;
using STI::Device::DeviceID;
using STI::Device::LocalDevice;
using STI::Network::HubID;
using STI::Network::LocalDeviceHub;
using STI::Network::NetworkDeviceHub;
using STI::Utils::Configuration;

namespace {

class TempRoot {
public:
    explicit TempRoot(const std::string& name)
    {
        static std::atomic<unsigned> counter{0};
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        path = std::filesystem::temp_directory_path()
            / ("sti3_networkdevicehub_" + name + "_" + std::to_string(now) + "_" + std::to_string(counter++));
    }

    ~TempRoot()
    {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }

    std::filesystem::path path;
};

Configuration makeDeviceConfig(const std::filesystem::path& root, const std::string& name)
{
    Configuration config;
    config.set("PersistenceManager", "root path", root.string());
    config.set("PersistenceManager", "device subdirectory", name);
    config.set("EngineManager", "Engine Count", 0);
    return config;
}

Configuration makeHubConfig()
{
    Configuration config;
    config.set("NetworkHub", "NameService", "127.0.0.1:9");
    config.set("NetworkHub", "SelfRebind", "false");
    config.set("NetworkHub", "EnablePrune", "false");
    config.set("omniORB", "clientConnectTimeOutPeriod", "50");
    return config;
}

class RefreshControlledDevice : public LocalDevice {
public:
    RefreshControlledDevice(const std::string& name, unsigned short module, const std::filesystem::path& root)
        : LocalDevice(name, "127.0.0.1", module, "root", makeDeviceConfig(root, name)) {}

    bool refresh() override
    {
        ++refreshCount;
        return alive;
    }

    bool alive = true;
    unsigned refreshCount = 0;
};

bool collectionContains(const std::shared_ptr<RefreshControlledDevice>& device, const DeviceID& id)
{
    std::shared_ptr<DeviceCollection> collection;
    device->getCollection(collection);
    return collection != nullptr && collection->contains(id);
}

} // namespace

TEST_CASE("NetworkDeviceHub topology-change cleanup removes stale remote device refs", "[networkdevicehub][network]")
{
    TempRoot root("topology_cleanup");

    auto localDevice = std::make_shared<RefreshControlledDevice>("LocalDevice", 1, root.path);
    auto staleDevice = std::make_shared<RefreshControlledDevice>("StaleRemoteDevice", 2, root.path);
    auto liveDevice = std::make_shared<RefreshControlledDevice>("LiveRemoteDevice", 3, root.path);

    auto staleHub = std::make_shared<LocalDeviceHub>(HubID("StaleHub", "127.0.0.1", 20));
    auto liveHub = std::make_shared<LocalDeviceHub>(HubID("LiveHub", "127.0.0.1", 30));
    NetworkDeviceHub networkHub(HubID("NetworkHub", "127.0.0.1", 10), makeHubConfig());

    localDevice->addPartner(staleDevice->getID());
    localDevice->addPartner(liveDevice->getID());

    REQUIRE(networkHub.addDevice(localDevice));
    REQUIRE(staleHub->addDevice(staleDevice));
    REQUIRE(networkHub.connect(staleHub));
    REQUIRE(collectionContains(localDevice, staleDevice->getID()));

    const auto staleRefreshCountAfterFirstConnect = staleDevice->refreshCount;
    staleDevice->alive = false;

    REQUIRE(liveHub->addDevice(liveDevice));
    REQUIRE(networkHub.connect(liveHub));

    CHECK_FALSE(collectionContains(localDevice, staleDevice->getID()));
    CHECK(collectionContains(localDevice, liveDevice->getID()));
    CHECK(staleDevice->refreshCount > staleRefreshCountAfterFirstConnect);

    networkHub.shutdown();
}

TEST_CASE("NetworkDeviceHub periodic maintenance does not refresh local devices", "[networkdevicehub][network][periodic]")
{
    TempRoot root("periodic");

    auto device = std::make_shared<RefreshControlledDevice>("PeriodicDevice", 4, root.path);
    NetworkDeviceHub networkHub(HubID("PeriodicHub", "127.0.0.1", 40), makeHubConfig());

    REQUIRE(networkHub.addDevice(device));
    const auto refreshCountAfterAdd = device->refreshCount;

    std::this_thread::sleep_for(std::chrono::milliseconds(6200));

    CHECK(device->refreshCount == refreshCountAfterAdd);

    networkHub.shutdown();
}
