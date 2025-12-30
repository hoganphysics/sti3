#ifndef STI_DEVICE_DEVICEMESSAGE_TESTS_SUPPORT_H
#define STI_DEVICE_DEVICEMESSAGE_TESTS_SUPPORT_H

#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceID.h>

#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace device_message_test_support {

using STI::Device::AttributeUpdateMessage;
using STI::Device::DeviceID;
using STI::Device::RefreshDeviceMessage;

inline DeviceID makeDeviceID(const std::string& name) {
    return DeviceID(name, "127.0.0.1", 1);
}

inline std::shared_ptr<AttributeUpdateMessage> makeAttributeMessage(const DeviceID& source, const std::string& key,
                                                                    const std::string& value) {
    return std::make_shared<AttributeUpdateMessage>(source, key, value);
}

class CountingListener : public STI::Device::DeviceMessageListener<AttributeUpdateMessage> {
public:
    void handleMessage(const std::shared_ptr<AttributeUpdateMessage>& mess) override {
        std::lock_guard<std::mutex> lock(mutex);
        ++count;
        seen.push_back(mess->attributes);
        cv.notify_all();
    }

    bool waitFor(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return count >= expected; });
    }

    std::size_t count{0};
    std::vector<std::map<std::string, std::string>> seen;

private:
    std::mutex mutex;
    std::condition_variable cv;
};

class RefreshCountingListener : public STI::Device::DeviceMessageListener<RefreshDeviceMessage> {
public:
    void handleMessage(const std::shared_ptr<RefreshDeviceMessage>&) override {
        std::lock_guard<std::mutex> lock(mutex);
        ++count;
        cv.notify_all();
    }
    bool waitFor(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return count >= expected; });
    }
    std::size_t count{0};

private:
    std::mutex mutex;
    std::condition_variable cv;
};

} // namespace device_message_test_support

#endif
