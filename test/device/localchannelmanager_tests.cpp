#include <catch2/catch_test_macros.hpp>

#include <sti/device/Profile.h>
#include <sti/device/LocalChannel.h>
#include <sti/device/ChannelManager.h>

#include <fstream>
#include <filesystem>
#include <memory>
#include <vector>

#include "../../src/device/src/LocalChannelManager.h"
#include "localchannel_tests_support.h"

using local_channel_test_support::ChannelRefreshRecorder;
using local_channel_test_support::TestLocalDevice;
using STI::Device::Channel;
using STI::Device::ChannelManager;
using STI::Device::LocalChannel;
using STI::Device::LocalChannelManager;
using STI::Device::Profile;
using STI::Device::ProfileType;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

namespace {

std::shared_ptr<LocalChannelManager> getManager(TestLocalDevice& device) {
    std::shared_ptr<ChannelManager> base;
    device.getChannelManager(base);
    return std::dynamic_pointer_cast<LocalChannelManager>(base);
}

} // namespace

TEST_CASE("LocalChannelManager: add and fetch channels") {
    TestLocalDevice device;
    auto manager = getManager(device);
    REQUIRE(manager);

    device.addOutputChannel(1, MixedValueType::Double, "out1");
    device.addInputChannel(2, MixedValueType::Double, MixedValueType::Double, "in2");

    std::vector<std::shared_ptr<Channel>> channels;
    manager->getChannels(channels);
    CHECK(channels.size() == 2);

    std::shared_ptr<Channel> fetched;
    REQUIRE(manager->getChannel(1, fetched));
    CHECK(fetched->getChannelName() == "out1");
    CHECK_FALSE(manager->getChannel(99, fetched));
}

TEST_CASE("LocalChannelManager: write and read delegate to device and update channel state") {
    TestLocalDevice device;
    auto manager = getManager(device);
    REQUIRE(manager);

    device.addOutputChannel(5, MixedValueType::Double, "out");
    device.addInputChannel(6, MixedValueType::Double, MixedValueType::Double, "in");
    device.addInputChannel(7, MixedValueType::Double, "empty-out-in");

    MixedValue writeVal(3.14);
    REQUIRE(manager->writeChannel(5, writeVal));
    std::shared_ptr<Channel> written;
    REQUIRE(manager->getChannel(5, written));
    CHECK(written->getLastValue() == writeVal);
    CHECK(written->getLastMeasurement().isEmpty());
    CHECK(device.lastWrite.at(5) == writeVal);

    device.setWriteResult(5, false);
    MixedValue newer(6.28);
    CHECK_FALSE(manager->writeChannel(5, newer));
    CHECK(written->getLastValue() == writeVal); // unchanged on failure

    MixedValue readOut;
    device.setReadResponse(6, MixedValue(42.0));
    REQUIRE(manager->readChannel(6, MixedValue(0.0), readOut));
    CHECK(readOut == MixedValue(42.0));
    std::shared_ptr<Channel> readChannel;
    REQUIRE(manager->getChannel(6, readChannel));
    CHECK(readChannel->getLastValue() == MixedValue(0.0));
    CHECK(readChannel->getLastMeasurement() == MixedValue(42.0));

    device.setReadResponse(7, MixedValue(9.0));
    REQUIRE(manager->readChannel(7, MixedValue(), readOut));
    std::shared_ptr<Channel> emptyOutReadChannel;
    REQUIRE(manager->getChannel(7, emptyOutReadChannel));
    CHECK(emptyOutReadChannel->getLastValue().isEmpty());
    CHECK(emptyOutReadChannel->getLastMeasurement() == MixedValue(9.0));

    CHECK_FALSE(manager->readChannel(99, MixedValue(), readOut));
}

TEST_CASE("LocalDevice: direct read path updates channel state and rejects bad measurement types", "[localdevice][channel]")
{
    TestLocalDevice device;
    auto manager = getManager(device);
    REQUIRE(manager);

    device.addInputChannel(8, MixedValueType::Double, MixedValueType::Int, "direct-in");
    device.setReadResponse(8, MixedValue(12.5));

    MixedValue readOut;
    REQUIRE(device.read(8, MixedValue(4), readOut));
    CHECK(readOut == MixedValue(12.5));

    std::shared_ptr<Channel> directChannel;
    REQUIRE(manager->getChannel(8, directChannel));
    CHECK(directChannel->getLastValue() == MixedValue(4));
    CHECK(directChannel->getLastMeasurement() == MixedValue(12.5));

    device.setReadResponse(8, MixedValue("wrong type"));
    CHECK_FALSE(device.read(8, MixedValue(5), readOut));
    CHECK(directChannel->getLastValue() == MixedValue(4));
    CHECK(directChannel->getLastMeasurement() == MixedValue(12.5));
}

TEST_CASE("LocalChannelManager: profile load/save round trip for channel values") {
    TestLocalDevice device;
    auto manager = getManager(device);
    REQUIRE(manager);

    device.addOutputChannel(1, MixedValueType::Int, "out1");
    device.addOutputChannel(2, MixedValueType::Int, "out2");

    auto profile = std::make_shared<Profile>();
    profile->type = ProfileType::Channel;

    MixedValue v1(10);
    MixedValue v2(20);
    REQUIRE(manager->writeChannel(1, v1));
    REQUIRE(manager->writeChannel(2, v2));

    REQUIRE(manager->saveProfile(profile));
    REQUIRE(profile->channelData.size() == 2);
    CHECK(profile->channelData[1] == v1);
    CHECK(profile->channelData[2] == v2);

    auto updated = std::make_shared<Profile>();
    updated->type = ProfileType::Channel;
    updated->channelData[1] = MixedValue(111);
    updated->channelData[2] = MixedValue(222);

    REQUIRE(manager->loadProfile(updated));
    std::shared_ptr<Channel> ch1;
    REQUIRE(manager->getChannel(1, ch1));
    CHECK(ch1->getLastValue() == MixedValue(111));
}

TEST_CASE("LocalChannelManager: profiles include input channel configuration values", "[localchannelmanager][profile]")
{
    TestLocalDevice device;
    auto manager = getManager(device);
    REQUIRE(manager);

    device.addOutputChannel(1, MixedValueType::Int, "out");
    device.addInputChannel(2, MixedValueType::Double, MixedValueType::Int, "configured-in");
    device.addInputChannel(3, MixedValueType::Double, "plain-in");

    REQUIRE(device.write(1, MixedValue(10)));

    MixedValue readOut;
    device.setReadResponse(2, MixedValue(2.5));
    REQUIRE(device.read(2, MixedValue(7), readOut));
    device.setReadResponse(3, MixedValue(3.5));
    REQUIRE(device.read(3, MixedValue(), readOut));

    auto profile = std::make_shared<Profile>();
    profile->type = ProfileType::Channel;
    REQUIRE(manager->saveProfile(profile));

    REQUIRE(profile->channelData.size() == 2);
    CHECK(profile->channelData.at(1) == MixedValue(10));
    CHECK(profile->channelData.at(2) == MixedValue(7));
    CHECK(profile->channelData.count(3) == 0);

    auto loaded = std::make_shared<Profile>();
    loaded->type = ProfileType::Channel;
    loaded->channelData[2] = MixedValue(11);

    REQUIRE(manager->loadProfile(loaded));
    std::shared_ptr<Channel> inputChannel;
    REQUIRE(manager->getChannel(2, inputChannel));
    CHECK(inputChannel->getLastValue() == MixedValue(11));
    CHECK(inputChannel->getLastMeasurement() == MixedValue(2.5));
}

TEST_CASE("LocalChannelManager: persistence callbacks run on name updates after load") {
    TestLocalDevice device;
    auto manager = getManager(device);
    REQUIRE(manager);

    device.addOutputChannel(3, MixedValueType::Int, "persisted");

    auto tempPath = std::filesystem::temp_directory_path() / "localchannelmanager_test.ini";
    {
        std::ofstream out(tempPath);
        out << "[Channel names]\n";
        out << "3 = stored\n";
    }

    std::size_t callbackCount = 0;
    STI::Device::PersistenceTarget* target = manager.get();
    target->setPersistenceCallback([&callbackCount]() { ++callbackCount; });
    target->load(tempPath.string());

    std::shared_ptr<Channel> ch;
    REQUIRE(manager->getChannel(3, ch));
    CHECK(ch->getChannelName() == "stored");
    CHECK(callbackCount == 0);

    ch->setChannelName("renamed");
    CHECK(callbackCount == 1);

    CHECK(target->save(tempPath.string()));
}
