#include <catch2/catch_test_macros.hpp>

#include <sti/utils/TaskScheduler.h>
#include <sti/utils/IntervalTask.h>
#include <sti/utils/AppointmentTask.h>

#include "task_tests_support.h"

#include <chrono>
#include <atomic>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using task_test_support::DummyTask;
using task_test_support::RecordingListener;
using STI::Utils::TaskScheduler;
using STI::Utils::TaskSchedulerEventType;
using STI::Utils::TaskStatus;
using STI::Utils::IntervalTask;
using STI::Utils::AppointmentTask;

TEST_CASE("TaskScheduler: addTask stores and activates tasks") {
    TaskScheduler scheduler;
    RecordingListener listener;
    scheduler.addListener(&listener);

    auto task = std::make_shared<DummyTask>("task-1");
    scheduler.addTask(task);

    std::set<std::string> ids;
    scheduler.getIDs(ids);
    REQUIRE(ids.count("task-1") == 1);
    CHECK(task->getStatus() == TaskStatus::Active);

    auto events = listener.snapshot();
    REQUIRE(events.size() == 2);
    CHECK(events[0].first == TaskSchedulerEventType::Add);
    CHECK(events[0].second == "task-1");
    CHECK(events[1].first == TaskSchedulerEventType::Activate);
    CHECK(events[1].second == "task-1");
}

TEST_CASE("TaskScheduler: listener callbacks can update tasks") {
    class DeactivatingListener : public STI::Utils::TaskSchedulerListener {
    public:
        explicit DeactivatingListener(TaskScheduler& scheduler)
            : scheduler(scheduler) {}

        void handleEvent(const STI::Utils::TaskSchedulerEvent& evt) override {
            if (evt.type == TaskSchedulerEventType::Add) {
                scheduler.deactivateTask(evt.taskID);
                deactivated = true;
            }
        }

        bool deactivated{false};

    private:
        TaskScheduler& scheduler;
    };

    TaskScheduler scheduler;
    DeactivatingListener listener(scheduler);
    scheduler.addListener(&listener);

    auto task = std::make_shared<DummyTask>("listener-update");
    scheduler.addTask(task);

    CHECK(listener.deactivated);
    CHECK(task->getStatus() == TaskStatus::Inactive);
}

TEST_CASE("TaskScheduler: runNow executes ready non-repeating task and deactivates") {
    TaskScheduler scheduler;
    RecordingListener listener;
    scheduler.addListener(&listener);

    auto task = std::make_shared<DummyTask>("run-once");
    scheduler.addTask(task);

    scheduler.runNow(task->getID());

    CHECK(task->getRunCount() == 1);
    CHECK(task->getStatus() == TaskStatus::Inactive);

    auto events = listener.snapshot();
    auto timestamps = listener.timestampSnapshot();
    REQUIRE(events.size() >= 4);
    CHECK(events[2].first == TaskSchedulerEventType::Run);
    CHECK(events[2].second == "run-once");
    REQUIRE(timestamps.size() >= 4);
    REQUIRE(timestamps[2].has_value());
    REQUIRE(task->getLastRunTime().has_value());
    CHECK(timestamps[2].value() == task->getLastRunTime().value());
    CHECK(events[3].first == TaskSchedulerEventType::Deactivate);
    CHECK(events[3].second == "run-once");
    CHECK(task->hasLastRunTime());
}

TEST_CASE("TaskScheduler: runNow skips when task is not ready") {
    TaskScheduler scheduler;
    RecordingListener listener;
    scheduler.addListener(&listener);

    auto task = std::make_shared<DummyTask>("skip-me");
    task->setReady(false);
    scheduler.addTask(task);

    scheduler.runNow(task->getID());

    CHECK(task->getRunCount() == 0);
    CHECK(task->getSkipCount() == 1);
    CHECK(task->getStatus() == TaskStatus::Inactive);
    CHECK_FALSE(task->hasLastRunTime());

    auto events = listener.snapshot();
    REQUIRE(events.size() >= 3);
    CHECK(events[2].first == TaskSchedulerEventType::Deactivate);
    CHECK(events[2].second == "skip-me");
}

TEST_CASE("TaskScheduler: runNow does not emit run event when task throws") {
    TaskScheduler scheduler;
    RecordingListener listener;
    scheduler.addListener(&listener);

    auto task = std::make_shared<DummyTask>("throw-me");
    task->setThrowOnRun(true);
    scheduler.addTask(task);

    CHECK_THROWS_AS(scheduler.runNow(task->getID()), std::runtime_error);

    CHECK_FALSE(task->hasLastRunTime());
    CHECK(task->getStatus() == TaskStatus::Active);

    auto events = listener.snapshot();
    REQUIRE(events.size() == 2);
    CHECK(events[0].first == TaskSchedulerEventType::Add);
    CHECK(events[1].first == TaskSchedulerEventType::Activate);
}

TEST_CASE("TaskScheduler: deactivate and activate toggle status") {
    TaskScheduler scheduler;
    RecordingListener listener;
    scheduler.addListener(&listener);

    auto task = std::make_shared<DummyTask>("toggle");
    scheduler.addTask(task);

    scheduler.deactivateTask(task->getID());
    CHECK(task->getStatus() == TaskStatus::Inactive);

    scheduler.activateTask(task->getID());
    CHECK(task->getStatus() == TaskStatus::Active);

    auto events = listener.snapshot();
    REQUIRE(events.size() == 4);
    CHECK(events[2].first == TaskSchedulerEventType::Deactivate);
    CHECK(events[3].first == TaskSchedulerEventType::Activate);
}

TEST_CASE("TaskScheduler: activating an idle task wakes the background loop") {
    using namespace std::chrono_literals;

    TaskScheduler scheduler;
    scheduler.setMinSleep(0.01);
    scheduler.start();

    auto task = std::make_shared<DummyTask>("wake-idle-loop", 100ms, false);
    scheduler.addTask(task);

    scheduler.deactivateTask(task->getID());
    REQUIRE(task->getStatus() == TaskStatus::Inactive);

    std::this_thread::sleep_for(150ms);

    scheduler.activateTask(task->getID());
    REQUIRE(task->waitForRuns(1, 150ms));

    scheduler.stop();
    CHECK(task->getStatus() == TaskStatus::Inactive);
}

TEST_CASE("TaskScheduler: clear removes all tasks and sets them inactive") {
    TaskScheduler scheduler;
    RecordingListener listener;
    scheduler.addListener(&listener);

    auto task1 = std::make_shared<DummyTask>("clear-1");
    auto task2 = std::make_shared<DummyTask>("clear-2");
    scheduler.addTask(task1);
    scheduler.addTask(task2);

    scheduler.clear();

    std::set<std::string> ids;
    scheduler.getIDs(ids);
    CHECK(ids.empty());
    CHECK(task1->getStatus() == TaskStatus::Inactive);
    CHECK(task2->getStatus() == TaskStatus::Inactive);

    auto events = listener.snapshot();
    REQUIRE(events.size() == 5);
    CHECK(events.back().first == TaskSchedulerEventType::Refresh);
    CHECK(events.back().second.empty());
}

TEST_CASE("TaskScheduler: background loop runs ready tasks") {
    using namespace std::chrono_literals;

    TaskScheduler scheduler;
    scheduler.setMinSleep(0.01);
    scheduler.start();

    auto task = std::make_shared<DummyTask>("loop", 0ms, false);
    scheduler.addTask(task);

    REQUIRE(task->waitForRuns(1, 200ms));
    scheduler.stop();

    CHECK(task->getStatus() == TaskStatus::Inactive);
}

TEST_CASE("TaskScheduler: IntervalTask repeats while scheduler runs") {
    using namespace std::chrono_literals;

    TaskScheduler scheduler;
    scheduler.setMinSleep(0.02);
    scheduler.start();

    std::atomic<int> runs{0};
    auto task = std::make_shared<IntervalTask>("interval-loop", 0.1, [&] { runs.fetch_add(1); });
    scheduler.addTask(task);

    REQUIRE(task_test_support::waitForAtomicCount(runs, 2, 300ms));
    scheduler.stop();

    CHECK(runs.load() >= 2);
    CHECK(task->getStatus() == TaskStatus::Active);
}

TEST_CASE("TaskScheduler: AppointmentTask once runs and deactivates") {
    using namespace std::chrono_literals;

    TaskScheduler scheduler;
    scheduler.setMinSleep(0.02);
    scheduler.start();

    std::atomic<int> runs{0};
    auto timeOfDay = task_test_support::timeStringInFuture(std::chrono::seconds(2));
    auto task = std::make_shared<AppointmentTask>("appointment-once", timeOfDay, AppointmentTask::AppointmentRepeatType::Once,
        [&] { runs.fetch_add(1); });
    scheduler.addTask(task);

    REQUIRE(task_test_support::waitForAtomicCount(runs, 1, 5s));
    scheduler.stop();

    CHECK(runs.load() == 1);
    CHECK(task->getStatus() == TaskStatus::Inactive);
}
