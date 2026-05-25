#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/Device.h>
#include <sti/utils/Configuration.h>
#include <sti/utils/MixedValue.h>

using STI::Device::Device;
using STI::Device::LocalDevice;
using STI::Utils::Configuration;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

namespace {

Configuration makeMetadataDeviceConfig()
{
    Configuration config;
    config.set("EngineManager", "Engine Count", 0);
    return config;
}

} // namespace

TEST_CASE("LocalDevice: metadata helpers store and retrieve values", "[localdevice][metadata]")
{
    LocalDevice device("MetadataDevice", "127.0.0.1", 1, "target", makeMetadataDeviceConfig());

    device.addMetaData("color", MixedValue("blue"));
    device.setDescription("Device-level help and description.");
    MixedValue keywordMetaData;
    keywordMetaData.addValue("camera");
    keywordMetaData.addValue("primary");
    device.addMetaData("keywords", keywordMetaData);

    const Device& base = device;

    CHECK(base.getMetaData().getType() == MixedValueType::Vector);
    CHECK(base.getMetaData("color").getString() == "blue");
    CHECK(base.getMetaData("description").getString() == "Device-level help and description.");

    const auto keywords = base.getMetaData("keywords");
    REQUIRE(keywords.getType() == MixedValueType::Vector);
    REQUIRE(keywords.getVector().size() == 2);
    CHECK(keywords.getVector().at(0).getString() == "camera");
    CHECK(keywords.getVector().at(1).getString() == "primary");

    CHECK(base.getMetaData("missing").getType() == MixedValueType::Empty);

    device.disable();
}
