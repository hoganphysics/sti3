#pragma once

#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageListener.h>

#include "../../src/device/src/AttributeRefreshListener.h"

#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace local_attribute_test_support {

using STI::Device::AttributeUpdateMessage;
using STI::Device::DeviceID;

inline DeviceID makeDeviceID() {
    return DeviceID("Local", "127.0.0.1", 1);
}

class RefreshRecorder : public STI::Device::AttributeRefreshListener {
public:
    void handleAttributeRefreshEvent(const std::string& key, const std::string& value) override {
        std::lock_guard<std::mutex> lock(mutex);
        lastKey = key;
        lastValue = value;
        ++count;
        cv.notify_all();
    }

    bool waitForCount(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return count >= expected; });
    }

    std::string lastKey;
    std::string lastValue;
    std::size_t count{0};

private:
    std::mutex mutex;
    std::condition_variable cv;
};

class AttributeUpdateRecorder : public STI::Device::DeviceMessageListener<AttributeUpdateMessage> {
public:
    void handleMessage(const std::shared_ptr<AttributeUpdateMessage>& mess) override {
        std::lock_guard<std::mutex> lock(mutex);
        received.push_back(mess->attributes);
        cv.notify_all();
    }

    bool waitForMessages(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return received.size() >= expected; });
    }

    std::vector<std::map<std::string, std::string>> received;

private:
    std::mutex mutex;
    std::condition_variable cv;
};

} // namespace local_attribute_test_support
