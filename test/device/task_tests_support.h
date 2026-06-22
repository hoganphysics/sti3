#ifndef STI_DEVICE_TASK_TESTS_SUPPORT_H
#define STI_DEVICE_TASK_TESTS_SUPPORT_H

#include <sti/utils/Task.h>
#include <sti/utils/TaskScheduler.h>
#include <sti/utils/AppointmentTask.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace task_test_support {

class DummyTask : public STI::Utils::Task {
public:
    DummyTask(const std::string& id,
              std::chrono::milliseconds initialDelay = std::chrono::milliseconds(0),
              bool repeat = false,
              std::chrono::milliseconds repeatDelay = std::chrono::milliseconds(0))
        : Task(id),
          ready(true),
          repeatFlag(repeat),
          repeatDelay(repeatDelay),
          nextRunTime(std::chrono::steady_clock::now() + initialDelay) {}

    void setReady(bool value) { ready.store(value); }

    void setRepeat(bool value, std::chrono::milliseconds delay = std::chrono::milliseconds(0)) {
        repeatFlag = value;
        repeatDelay = delay;
    }

    void scheduleAfter(std::chrono::milliseconds delay) {
        nextRunTime = std::chrono::steady_clock::now() + delay;
    }

    int getRunCount() const {
        std::lock_guard<std::mutex> lock(mutex);
        return runCounter;
    }

    int getSkipCount() const {
        std::lock_guard<std::mutex> lock(mutex);
        return skipCounter;
    }

    bool waitForRuns(int expectedRuns, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return runCounter >= expectedRuns; });
    }

    bool waitForSkips(int expectedSkips, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return skipCounter >= expectedSkips; });
    }

    bool isReadyToRun() override { return ready.load(); }

    double secondsToNextRun() const override {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(nextRunTime - now).count();
    }

    void run() override {
        recordRun();
        if (repeatFlag) {
            nextRunTime = std::chrono::steady_clock::now() + repeatDelay;
        }
    }

    void skipTask() override {
        recordSkip();
        if (repeatFlag) {
            nextRunTime = std::chrono::steady_clock::now() + repeatDelay;
        }
    }

    bool repeat() override { return repeatFlag; }

private:
    void recordRun() {
        std::lock_guard<std::mutex> lock(mutex);
        ++runCounter;
        cv.notify_all();
    }

    void recordSkip() {
        std::lock_guard<std::mutex> lock(mutex);
        ++skipCounter;
        cv.notify_all();
    }

    std::atomic<bool> ready;
    bool repeatFlag;
    std::chrono::milliseconds repeatDelay;
    std::chrono::steady_clock::time_point nextRunTime;

    mutable std::mutex mutex;
    std::condition_variable cv;
    int runCounter{0};
    int skipCounter{0};
};

class RecordingListener : public STI::Utils::TaskSchedulerListener {
public:
    void handleEvent(const STI::Utils::TaskSchedulerEvent& evt) override {
        std::lock_guard<std::mutex> lock(mutex);
        events.emplace_back(evt.type, evt.taskID);
        timestamps.push_back(evt.timestamp);
        cv.notify_all();
    }

    std::vector<std::pair<STI::Utils::TaskSchedulerEventType, std::string>> snapshot() const {
        std::lock_guard<std::mutex> lock(mutex);
        return events;
    }

    std::vector<std::string> timestampSnapshot() const {
        std::lock_guard<std::mutex> lock(mutex);
        return timestamps;
    }

    bool waitForEvents(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex);
        return cv.wait_for(lock, timeout, [&] { return events.size() >= expected; });
    }

private:
    std::vector<std::pair<STI::Utils::TaskSchedulerEventType, std::string>> events;
    std::vector<std::string> timestamps;
    mutable std::mutex mutex;
    std::condition_variable cv;
};

inline std::string timeStringInFuture(std::chrono::seconds offset) {
    auto target = std::chrono::system_clock::now() + offset;
    std::time_t tt = std::chrono::system_clock::to_time_t(target);
    std::tm local_tm{};
#ifdef _WIN32
    localtime_s(&local_tm, &tt);
#else
    localtime_r(&tt, &local_tm);
#endif

    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(2) << local_tm.tm_hour << ":"
       << std::setfill('0') << std::setw(2) << local_tm.tm_min << ":"
       << std::setfill('0') << std::setw(2) << local_tm.tm_sec;
    return ss.str();
}

inline bool waitForAtomicCount(const std::atomic<int>& counter, int expected, std::chrono::milliseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (counter.load() >= expected) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return counter.load() >= expected;
}

} // namespace task_test_support

#endif
