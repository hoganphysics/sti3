#include <catch2/catch_test_macros.hpp>

#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessage.h>
#include <sti/utils/MixedValue.h>

using STI::Device::ChannelUpdateMessage;
using STI::Device::DeviceID;
using STI::Utils::MixedValue;

TEST_CASE("ChannelUpdateMessage: grouped value updates merge measurement values", "[devicemessage][channel]")
{
    DeviceID source("MessageDevice", "127.0.0.1", 1);
    ChannelUpdateMessage first(source, 1, MixedValue(10));
    first.measurementValues[2] = MixedValue(20.5);

    auto second = ChannelUpdateMessage::makeMeasurementMessage(source, 1, MixedValue(30.5));
    second->channelValues[2] = MixedValue(40);

    REQUIRE(first.appendMessage(*second));

    REQUIRE(first.channelValues.size() == 2);
    CHECK(first.channelValues.at(1) == MixedValue(10));
    CHECK(first.channelValues.at(2) == MixedValue(40));

    REQUIRE(first.measurementValues.size() == 2);
    CHECK(first.measurementValues.at(1) == MixedValue(30.5));
    CHECK(first.measurementValues.at(2) == MixedValue(20.5));
    CHECK(first.groupable());
}
