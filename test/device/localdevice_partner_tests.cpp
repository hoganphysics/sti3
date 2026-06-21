#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/Device.h>
#include <sti/device/PartnerDeviceInfo.h>
#include <sti/utils/Configuration.h>

#include <algorithm>
#include <vector>

using STI::Device::Device;
using STI::Device::DeviceID;
using STI::Device::LocalDevice;
using STI::Device::PartnerDeviceInfo;
using STI::Utils::Configuration;

namespace {

Configuration makePartnerDeviceConfig()
{
    Configuration config;
    config.set("EngineManager", "Engine Count", 0);
    return config;
}

std::vector<std::string> sortedAliases(std::vector<std::string> aliases)
{
    std::sort(aliases.begin(), aliases.end());
    return aliases;
}

const PartnerDeviceInfo* findPartner(
    const std::vector<PartnerDeviceInfo>& partners,
    const DeviceID& id)
{
    auto it = std::find_if(partners.begin(), partners.end(), [&id](const PartnerDeviceInfo& partner) {
        return partner.deviceID == id;
    });
    return it == partners.end() ? nullptr : &(*it);
}

} // namespace

TEST_CASE("LocalDevice: getPartnerDevices reports declared partners and event targets", "[localdevice][partner]")
{
    LocalDevice device("PartnerDevice", "127.0.0.1", 1, "target", makePartnerDeviceConfig());
    const DeviceID partnerID("Supply", "127.0.0.2", 2, "supply-server");
    const DeviceID eventTargetID("Camera", "127.0.0.3", 3, "camera-server");

    device.addPartner(partnerID, "supply");
    device.addPartner(partnerID, "laser-supply");
    device.addEventTarget(eventTargetID, "camera");

    std::vector<PartnerDeviceInfo> partners;
    const Device& base = device;
    base.getPartnerDevices(partners);

    REQUIRE(partners.size() == 2);

    const auto* partner = findPartner(partners, partnerID);
    REQUIRE(partner != nullptr);
    CHECK(sortedAliases(partner->aliases) == std::vector<std::string>{"laser-supply", "supply"});
    CHECK_FALSE(partner->eventTarget);

    const auto* eventTarget = findPartner(partners, eventTargetID);
    REQUIRE(eventTarget != nullptr);
    CHECK(eventTarget->aliases == std::vector<std::string>{"camera"});
    CHECK(eventTarget->eventTarget);

    device.disable();
}
