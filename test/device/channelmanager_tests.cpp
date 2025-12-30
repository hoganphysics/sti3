#include <catch2/catch_test_macros.hpp>

#include <sti/device/ChannelManager.h>
#include <sti/utils/MixedValue.h>

#include <map>
#include <memory>
#include <vector>

#include "channel_tests_support.h"

using STI::Device::Channel;
using STI::Device::ChannelManager;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

// Lightweight manager used to exercise the interface.
class DummyChannelManager : public ChannelManager {
public:
    bool addChannel(const std::shared_ptr<Channel>& channel) {
        return channels.emplace(channel->getChannelNumber(), channel).second;
    }

    void setReadResponse(short channel, const MixedValue& value) { readResponses[channel] = value; }

    void getChannels(std::vector<std::shared_ptr<Channel>>& out) override {
        out.clear();
        for (auto& entry : channels) {
            out.push_back(entry.second);
        }
    }

    bool getChannel(short channelNumber, std::shared_ptr<Channel>& channel) override {
        auto it = channels.find(channelNumber);
        if (it == channels.end()) {
            return false;
        }
        channel = it->second;
        return channel != nullptr;
    }

    bool writeChannel(short channel, const MixedValue& value) override {
        std::shared_ptr<Channel> ch;
        if (!getChannel(channel, ch)) {
            return false;
        }
        writes.push_back({channel, value});
        ch->saveLastValue(value);
        return true;
    }

    bool readChannel(short channel, const MixedValue& value, MixedValue& data) override {
        reads.push_back({channel, value});
        auto it = readResponses.find(channel);
        if (it == readResponses.end()) {
            return false;
        }
        data = it->second;
        return true;
    }

    void stop() override { stopped = true; }

    std::vector<std::pair<short, MixedValue>> writes;
    std::vector<std::pair<short, MixedValue>> reads;
    bool stopped{false};

private:
    std::map<short, std::shared_ptr<Channel>> channels;
    std::map<short, MixedValue> readResponses;
};

TEST_CASE("ChannelManager: add, list, and fetch channels") {
    DummyChannelManager manager;
    auto chA = std::make_shared<TestChannel>(1, STI::Device::ChannelType::Output,
                                             MixedValueType::Int, MixedValueType::Int, "A");
    auto chB = std::make_shared<TestChannel>(2, STI::Device::ChannelType::Input,
                                             MixedValueType::Double, MixedValueType::Double, "B");

    REQUIRE(manager.addChannel(chA));
    REQUIRE(manager.addChannel(chB));
    CHECK_FALSE(manager.addChannel(chA)); // duplicate

    std::shared_ptr<Channel> fetched;
    REQUIRE(manager.getChannel(1, fetched));
    CHECK(fetched->getChannelName() == "A");
    CHECK_FALSE(manager.getChannel(99, fetched));

    std::vector<std::shared_ptr<Channel>> all;
    manager.getChannels(all);
    CHECK(all.size() == 2);
}

TEST_CASE("ChannelManager: write/read pathways update channels") {
    DummyChannelManager manager;
    auto ch = std::make_shared<TestChannel>(5, STI::Device::ChannelType::Output,
                                            MixedValueType::Int, MixedValueType::Int, "data");
    REQUIRE(manager.addChannel(ch));

    REQUIRE(manager.writeChannel(5, MixedValue(42)));
    CHECK(ch->getLastValue() == MixedValue(42));
    CHECK(manager.writes.size() == 1);

    MixedValue out;
    manager.setReadResponse(5, MixedValue(7));
    REQUIRE(manager.readChannel(5, MixedValue(), out));
    CHECK(out == MixedValue(7));
    CHECK(manager.reads.size() == 1);

    CHECK_FALSE(manager.writeChannel(10, MixedValue(1)));
    CHECK_FALSE(manager.readChannel(10, MixedValue(), out));
}

TEST_CASE("ChannelManager: stop is callable") {
    DummyChannelManager manager;
    CHECK_FALSE(manager.stopped);
    manager.stop();
    CHECK(manager.stopped);
}
