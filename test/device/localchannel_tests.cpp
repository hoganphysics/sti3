#include <catch2/catch_test_macros.hpp>

#include <sti/device/LocalChannel.h>
#include <sti/utils/MixedValue.h>

#include <string>
#include <vector>

#include "localchannel_tests_support.h"

using local_channel_test_support::ChannelRefreshRecorder;
using STI::Device::ChannelType;
using STI::Device::LocalChannel;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

TEST_CASE("LocalChannel: constructors set defaults") {
    LocalChannel defaultChannel;
    CHECK(defaultChannel.getChannelNumber() == 0);
    CHECK(defaultChannel.getType() == ChannelType::Output);
    CHECK(defaultChannel.getInputType() == MixedValueType::Empty);
    CHECK(defaultChannel.getOutputType() == MixedValueType::Empty);
    CHECK(defaultChannel.getChannelName().empty());

    LocalChannel channel(3, ChannelType::Input, MixedValueType::String, MixedValueType::Double, "named");
    CHECK(channel.getChannelNumber() == 3);
    CHECK(channel.getType() == ChannelType::Input);
    CHECK(channel.getInputType() == MixedValueType::String);
    CHECK(channel.getOutputType() == MixedValueType::Double);
    CHECK(channel.getChannelName() == "named");
}

TEST_CASE("LocalChannel: refresh listeners receive value and name updates") {
    LocalChannel channel(9, ChannelType::Output, MixedValueType::Int, MixedValueType::Int, "channel-9");
    ChannelRefreshRecorder recorderA;
    ChannelRefreshRecorder recorderB;
    channel.addRefreshListener(&recorderA);
    channel.addRefreshListener(&recorderB);

    MixedValue value(42);
    channel.saveLastValue(value);

    REQUIRE(recorderA.refreshEvents.size() == 1);
    REQUIRE(recorderB.refreshEvents.size() == 1);
    CHECK(recorderA.refreshEvents.front().first == 9);
    CHECK(recorderA.refreshEvents.front().second == MixedValue(42));

    MixedValue measurement(7);
    channel.saveLastMeasurement(measurement);

    REQUIRE(recorderA.measurementEvents.size() == 1);
    REQUIRE(recorderB.measurementEvents.size() == 1);
    CHECK(recorderA.measurementEvents.front().first == 9);
    CHECK(recorderA.measurementEvents.front().second == MixedValue(7));
    CHECK(channel.getLastMeasurement() == MixedValue(7));

    channel.setChannelName("updated");
    REQUIRE(recorderA.nameEvents.size() == 1);
    REQUIRE(recorderB.nameEvents.size() == 1);
    CHECK(recorderA.nameEvents.front().second == "updated");
}

TEST_CASE("LocalChannel: metadata helpers store and retrieve values") {
    LocalChannel channel(1, ChannelType::Output, MixedValueType::Empty, MixedValueType::Double, "meta");

    channel.addMetaData("units", MixedValue("V"));
    channel.setMeasurementUnits("Hz");
    std::vector<std::string> labels = {"one", "two"};
    channel.addMetaDataList("labels", labels);

    auto allMeta = channel.getMetaData();
    CHECK(allMeta.getType() == MixedValueType::Vector);

    CHECK(channel.getMetaData("units").getString() == "V");
    CHECK(channel.getMetaData("measurementUnits").getString() == "Hz");
    auto missing = channel.getMetaData("missing");
    CHECK(missing.getType() == MixedValueType::Empty);
}
