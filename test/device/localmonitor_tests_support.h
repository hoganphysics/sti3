#ifndef STI_DEVICE_LOCALMONITOR_TESTS_SUPPORT_H
#define STI_DEVICE_LOCALMONITOR_TESTS_SUPPORT_H

#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/MonitorListener.h>

#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace local_monitor_test_support {

using STI::Device::DeviceID;
using STI::Device::MonitorUpdateMessage;
using STI::Utils::MixedValue;

inline DeviceID makeDeviceID() {
    return DeviceID("LocalMonitorDevice", "127.0.0.1", 1);
}

class MonitorListenerRecorder : public STI::Device::MonitorListener {
public:
    void activated(const std::string& id) override {
        std::lock_guard<std::mutex> lock(mutex);
        lastID = id;
        ++activationCount;
        cv.notify_all();
    }

    void deactivated(const std::string& id) override {
        std::lock_guard<std::mutex> lock(mutex);
        lastID = id;
        ++deactivationCount;
        cv.notify_all();
    }

    void valueUpdated(const std::string& id, const MixedValue& value) override {
        std::lock_guard<std::mutex> lock(mutex);
        lastID = id;
        lastValue = value;
        ++updateCount;
        cv.notify_all();
    }

    bool waitForActivations(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return activationCount >= expected; });
    }

    bool waitForDeactivations(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return deactivationCount >= expected; });
    }

    bool waitForUpdates(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return updateCount >= expected; });
    }

    std::size_t getUpdateCount() const {
        std::lock_guard<std::mutex> lock(mutex);
        return updateCount;
    }

    std::string lastID;
    MixedValue lastValue;

private:
    mutable std::mutex mutex;
    std::condition_variable cv;
    std::size_t activationCount{0};
    std::size_t deactivationCount{0};
    std::size_t updateCount{0};
};

class MonitorUpdateRecorder : public STI::Device::DeviceMessageListener<MonitorUpdateMessage> {
public:
    void handleMessage(const std::shared_ptr<MonitorUpdateMessage>& mess) override {
        std::lock_guard<std::mutex> lock(mutex);
        received.push_back(mess->updates);
        sources.push_back(mess->sourceID());
        cv.notify_all();
    }

    bool waitForMessages(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return received.size() >= expected; });
    }

    std::vector<std::map<std::string, MixedValue>> received;
    std::vector<DeviceID> sources;

private:
    std::mutex mutex;
    std::condition_variable cv;
};

} // namespace local_monitor_test_support

#endif
