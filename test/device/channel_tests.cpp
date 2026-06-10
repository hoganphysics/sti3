#include <catch2/catch_test_macros.hpp>

#include <sti/device/Channel.h>
#include <sti/utils/MixedValue.h>
#include <string>

#include "channel_tests_support.h"

using STI::Device::Channel;
using STI::Device::ChannelType;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

TEST_CASE("Channel: typeToString handles known and unknown types") {
    CHECK(Channel::typeToString(ChannelType::Output) == "Output");
    CHECK(Channel::typeToString(ChannelType::Input) == "Input");
    CHECK(Channel::typeToString(static_cast<ChannelType>(999)) == "Unknown");
}

TEST_CASE("Channel: basic getters, setters, and metadata") {
    TestChannel channel(7, ChannelType::Input, MixedValueType::String, MixedValueType::Binary, "initial");

    CHECK(channel.getChannelNumber() == 7);
    CHECK(channel.getType() == ChannelType::Input);
    CHECK(channel.getInputType() == MixedValueType::String);
    CHECK(channel.getOutputType() == MixedValueType::Binary);
    CHECK(channel.getChannelName() == "initial");

    channel.setChannelName("renamed");
    CHECK(channel.getChannelName() == "renamed");

    MixedValue last("payload");
    channel.saveLastValue(last);
    CHECK(channel.getLastValue() == MixedValue("payload"));

    MixedValue measurement(5.5);
    channel.saveLastMeasurement(measurement);
    CHECK(channel.getLastMeasurement() == MixedValue(5.5));

    // Metadata retrieval falls back gracefully when the key is missing.
    auto missing = channel.getMetaData("not_set");
    CHECK(missing.getType() == MixedValueType::Empty);

    channel.addMeta("units", MixedValue("V"));
    CHECK(channel.getMetaData("units").getString() == "V");
    CHECK(channel.getMetaData().getType() == MixedValueType::Vector);
}
