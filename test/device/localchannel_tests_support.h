#ifndef STI_DEVICE_LOCALCHANNEL_TESTS_SUPPORT_H
#define STI_DEVICE_LOCALCHANNEL_TESTS_SUPPORT_H

#include <sti/LocalDevice.h>
#include <sti/device/Channel.h>
#include <sti/utils/MixedValue.h>

#include "../../src/device/src/ChannelRefreshListener.h"

#include <chrono>
#include <map>
#include <mutex>
#include <condition_variable>
#include <vector>

namespace local_channel_test_support {

class ChannelRefreshRecorder : public STI::Device::ChannelRefreshListener {
public:
    void handleChannelRefreshEvent(short channelNumber, const STI::Utils::MixedValue& value) override {
        std::lock_guard<std::mutex> lock(mutex);
        refreshEvents.push_back({channelNumber, value});
        cv.notify_all();
    }

    void handleChannelNameRefreshEvent(short channelNumber, const std::string& name) override {
        std::lock_guard<std::mutex> lock(mutex);
        nameEvents.push_back({channelNumber, name});
        cv.notify_all();
    }

    bool waitForEvents(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return refreshEvents.size() >= expected; });
    }

    std::vector<std::pair<short, STI::Utils::MixedValue>> refreshEvents;
    std::vector<std::pair<short, std::string>> nameEvents;

private:
    std::mutex mutex;
    std::condition_variable cv;
};

class TestLocalDevice : public STI::Device::LocalDevice {
public:
    TestLocalDevice() : LocalDevice("TestDevice", "127.0.0.1", 1, "target") {}

    bool writeChannel(short channel, const STI::Utils::MixedValue& value) override {
        lastWrite[channel] = value;
        auto it = writeResults.find(channel);
        if (it != writeResults.end()) {
            return it->second;
        }
        return defaultWriteResult;
    }

    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data) override {
        lastRead[channel] = value;
        auto it = readResponses.find(channel);
        if (it != readResponses.end()) {
            data = it->second;
            return true;
        }
        if (defaultReadResult) {
            data = defaultReadValue;
            return true;
        }
        return false;
    }

    void setWriteResult(short channel, bool success) { writeResults[channel] = success; }
    void setReadResponse(short channel, const STI::Utils::MixedValue& response) { readResponses[channel] = response; }
    void setDefaultReadResponse(const STI::Utils::MixedValue& response) {
        defaultReadResult = true;
        defaultReadValue = response;
    }

    std::map<short, STI::Utils::MixedValue> lastWrite;
    std::map<short, STI::Utils::MixedValue> lastRead;

private:
    std::map<short, bool> writeResults;
    std::map<short, STI::Utils::MixedValue> readResponses;
    bool defaultWriteResult{true};
    bool defaultReadResult{false};
    STI::Utils::MixedValue defaultReadValue{};
};

} // namespace local_channel_test_support

#endif
