#include <catch2/catch_test_macros.hpp>

#include <sti/device/DeviceID.h>

#include <string>

using STI::Device::DeviceID;

TEST_CASE("DeviceID: stringToDeviceID parses valid input") {
    DeviceID id;

    bool parsed = DeviceID::stringToDeviceID("192.168.0.1/7/DigitalOut", id);
    REQUIRE(parsed);
    CHECK(id.getAddress() == "192.168.0.1");
    CHECK(id.getModule() == 7);
    CHECK(id.getName() == "DigitalOut");
    CHECK(id.getID() == DeviceID::generateID("DigitalOut", "192.168.0.1", 7));
}

TEST_CASE("DeviceID: stringToDeviceID rejects malformed input") {
    DeviceID id;
    const std::string originalID = id.getID();

    bool parsed = DeviceID::stringToDeviceID("missing/module", id);
    REQUIRE_FALSE(parsed);
    CHECK(id.getID() == originalID);
}

TEST_CASE("DeviceID: generateID and generateContext sanitize fields") {
    DeviceID id("Digital/Out", "192.168.0.1", 3);

    CHECK(DeviceID::generateID("Digital/Out", "192.168.0.1", 3) == "192_168_0_1/3/Digital_Out");
    CHECK(DeviceID::generateContext(id) == "192_168_0_1/3/Digital_Out/");
}

TEST_CASE("DeviceID: stores target server id") {
    DeviceID id("ClockGen", "10.0.0.5", 12, "server-A");

    CHECK(id.getTargetServerID() == "server-A");
    CHECK(id.getID() == DeviceID::generateID("ClockGen", "10.0.0.5", 12));
}

TEST_CASE("DeviceID: comparison operators compare IDs") {
    DeviceID idA("Alpha", "10.0.0.1", 1);
    DeviceID idA_same("Alpha", "10.0.0.1", 1);
    DeviceID idB("Beta", "10.0.0.1", 1);

    CHECK(idA == idA_same);
    CHECK(idA != idB);
    CHECK(idA < idB);
    CHECK_FALSE(idB < idA);
}

TEST_CASE("DeviceID: default constructed ID is empty") {
    DeviceID nullId;

    CHECK(nullId.getName().empty());
    CHECK(nullId.getAddress().empty());
    CHECK(nullId.getModule() == 0);
    CHECK(nullId.getTargetServerID().empty());
    CHECK(nullId.getID() == DeviceID::generateID("", "", 0));
}
