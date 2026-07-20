#include <catch2/catch_test_macros.hpp>

#include <sti/LocalDevice.h>
#include <sti/device/Device.h>
#include <sti/device/LocalAttribute.h>
#include <sti/utils/Configuration.h>
#include <sti/utils/MixedValue.h>

#include <string>

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

Configuration makeConfiguredMetadataDeviceConfig()
{
    auto config = makeMetadataDeviceConfig();
    config.set("Device Name", "ConfiguredMetadataDevice");
    config.set("IP Address", "127.0.0.1");
    config.set("Module", 2);
    config.set("Target Server", "target");
    config.set("Metadata", "Color", "green");
    config.set("Metadata", "Description", "Configured device description.");
    config.set("Metadata", "Help", "Configured device help.");
    config.set("Metadata", "Owner", "Timing Lab");
    return config;
}

Configuration makeSectionedMetadataDeviceConfig()
{
    Configuration config;
    config.set("Laser", "Device Name", "SectionedMetadataDevice");
    config.set("Laser", "IP Address", "127.0.0.1");
    config.set("Laser", "Module", 3);
    config.set("Laser", "Target Server", "target");
    config.set("Laser.EngineManager", "Engine Count", 0);
    config.set("Laser.Metadata", "Color", "orange");
    config.set("Laser.Metadata", "Location", "Optics Table");
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

TEST_CASE("LocalDevice: constructor applies Metadata configuration section", "[localdevice][metadata]")
{
    LocalDevice device(makeConfiguredMetadataDeviceConfig());
    const Device& base = device;

    CHECK(base.getMetaData("color").getString() == "green");
    CHECK(base.getMetaData("description").getString() == "Configured device description.");
    CHECK(base.getMetaData("help").getString() == "Configured device help.");
    CHECK(base.getMetaData("Owner").getString() == "Timing Lab");

    device.disable();
}

TEST_CASE("LocalDevice: section constructor applies nested Metadata configuration", "[localdevice][metadata]")
{
    LocalDevice device(makeSectionedMetadataDeviceConfig(), "Laser");
    const Device& base = device;

    CHECK(base.getMetaData("color").getString() == "orange");
    CHECK(base.getMetaData("Location").getString() == "Optics Table");

    device.disable();
}

TEST_CASE("LocalDevice: refreshAttribute syncs member-backed attributes", "[localdevice][attribute]")
{
    LocalDevice device("AttributeRefreshDevice", "127.0.0.1", 1, "target", makeMetadataDeviceConfig());

    int alpha = 1;
    int beta = 10;

    device.addAttribute("alpha", std::to_string(alpha))
        .setRefresher([&]() { return std::to_string(alpha); });
    device.addAttribute("beta", std::to_string(beta))
        .setRefresher([&]() { return std::to_string(beta); });

    alpha = 2;
    beta = 11;

    CHECK(device.getAttribute("alpha") == "1");
    CHECK(device.getAttribute("beta") == "10");

    CHECK(device.refreshAttribute("alpha"));
    CHECK(device.getAttribute("alpha") == "2");
    CHECK(device.getAttribute("beta") == "10");
    CHECK_FALSE(device.refreshAttribute("missing"));

    device.addAttributeRefreshGroup({"alpha", "beta"});
    alpha = 3;
    beta = 12;
    CHECK(device.refreshAttribute("alpha"));
    CHECK(device.getAttribute("alpha") == "3");
    CHECK(device.getAttribute("beta") == "12");

    alpha = 4;
    device.refreshAttributes();

    CHECK(device.getAttribute("alpha") == "4");
    CHECK(device.getAttribute("beta") == "12");

    device.disable();
}
