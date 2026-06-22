#include <catch2/catch_test_macros.hpp>

#include "../../src/device/src/LocalDeviceMessageDispatcher.h"
#include "../../src/device/src/LocalTaskManager.h"

#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageReceiver.h>
#include <sti/utils/LocalCollection.h>

#include "task_tests_support.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

namespace {

class TaskUpdateRecorder {
public:
    void record(const std::shared_ptr<STI::Device::TaskUpdateMessage>& message)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            messages_.push_back(message);
        }
        cv_.notify_all();
    }

    bool waitForMessages(std::size_t count, std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, timeout, [&] { return messages_.size() >= count; });
    }

    std::vector<std::shared_ptr<STI::Device::TaskUpdateMessage>> snapshot() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return messages_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::shared_ptr<STI::Device::TaskUpdateMessage>> messages_;
};

} // namespace

TEST_CASE("LocalTaskManager dispatches task status and run update messages", "[taskmanager][device]")
{
    using STI::Device::DeviceID;
    using STI::Device::DeviceMessageReceiver;
    using STI::Device::LocalDeviceMessageDispatcher;
    using STI::Device::LocalTaskManager;
    using STI::Device::TaskUpdateMessage;
    using STI::Utils::TaskStatus;

    const DeviceID localID("TaskDevice", "127.0.0.1", 1, "task-server");
    auto dispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
    auto devices = std::make_shared<STI::Utils::LocalCollection<DeviceID, STI::Device::Device>>();
    DeviceMessageReceiver receiver(localID, devices, dispatcher);

    auto recorder = std::make_shared<TaskUpdateRecorder>();
    receiver.addListener<TaskUpdateMessage>(
        localID,
        "task-update-recorder",
        [recorder](const std::shared_ptr<TaskUpdateMessage>& message) {
            recorder->record(message);
        });

    LocalTaskManager manager(localID, dispatcher);
    auto task = std::make_shared<task_test_support::DummyTask>(
        "task-update", std::chrono::hours(1), false);

    manager.addTask(task);
    manager.deactivateTask(task->getID());
    manager.activateTask(task->getID());
    manager.runTask(task->getID());

    REQUIRE(task->getRunCount() == 1);
    REQUIRE(recorder->waitForMessages(5, std::chrono::milliseconds(1000)));

    auto messages = recorder->snapshot();

    const auto hasStatus = [&](TaskStatus status) {
        return std::any_of(messages.begin(), messages.end(), [&](const auto& message) {
            return message != 0 &&
                   message->updateType == TaskUpdateMessage::TaskUpdateType::Status &&
                   message->taskID == task->getID() &&
                   message->taskStatus == status &&
                   message->sourceID() == localID;
        });
    };

    CHECK(hasStatus(TaskStatus::Active));
    CHECK(hasStatus(TaskStatus::Inactive));

    const auto runIt = std::find_if(messages.begin(), messages.end(), [&](const auto& message) {
        return message != 0 &&
               message->updateType == TaskUpdateMessage::TaskUpdateType::Run &&
               message->taskID == task->getID();
    });

    REQUIRE(runIt != messages.end());
    CHECK_FALSE((*runIt)->timestamp.empty());
    CHECK((*runIt)->sourceID() == localID);
}
