#include <catch2/catch_test_macros.hpp>

#include <sti/utils/EventQueue.h>

#include <condition_variable>
#include <mutex>
#include <vector>

using STI::Utils::EventQueue;

namespace {
class RecordingQueue : public EventQueue<int> {
public:
    void handleEvent(const int& evt) override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            events_.push_back(evt);
        }
        cv_.notify_all();
    }

    void add(int evt) { addEvent(evt); }
    void addPriority(int evt) { addPriorityEvent(evt); }

    bool waitFor(std::size_t expected, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, timeout, [&] { return events_.size() >= expected; });
    }

    std::vector<int> events() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return events_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<int> events_;
};

class BlockingRecordingQueue : public RecordingQueue {
public:
    explicit BlockingRecordingQueue(int blockedEvent) : blockedEvent_(blockedEvent) {}

    ~BlockingRecordingQueue() override {
        releaseBlockedEvent();
        stop();
    }

    void handleEvent(const int& evt) override {
        if (evt == blockedEvent_) {
            std::unique_lock<std::mutex> lock(blockMutex_);
            handlingBlockedEvent_ = true;
            blockCv_.notify_all();
            blockCv_.wait(lock, [&] { return blockedEventReleased_; });
        }

        RecordingQueue::handleEvent(evt);
    }

    bool waitUntilBlocked(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(blockMutex_);
        return blockCv_.wait_for(lock, timeout, [&] { return handlingBlockedEvent_; });
    }

    void releaseBlockedEvent() {
        {
            std::lock_guard<std::mutex> lock(blockMutex_);
            blockedEventReleased_ = true;
        }
        blockCv_.notify_all();
    }

private:
    int blockedEvent_;
    std::mutex blockMutex_;
    std::condition_variable blockCv_;
    bool handlingBlockedEvent_{false};
    bool blockedEventReleased_{false};
};
} // namespace

TEST_CASE("EventQueue: processes events and honors priority order") {
    constexpr int blockingEvent = 0;
    BlockingRecordingQueue queue(blockingEvent);
    queue.start();

    // Occupy the worker so all events under test are queued before it can
    // remove any of them. Priority changes the order of queued events; it
    // cannot preempt an event that the worker has already removed.
    queue.add(blockingEvent);
    REQUIRE(queue.waitUntilBlocked(std::chrono::milliseconds(500)));

    queue.add(1);
    queue.add(2);
    queue.addPriority(99);
    queue.releaseBlockedEvent();

    REQUIRE(queue.waitFor(4, std::chrono::milliseconds(500)));

    auto events = queue.events();
    REQUIRE(events.size() == 4);
    CHECK(events[0] == blockingEvent);
    CHECK(events[1] == 99);
    CHECK(events[2] == 1);
    CHECK(events[3] == 2);

    queue.stop();
}

TEST_CASE("EventQueue: stop prevents new events") {
    RecordingQueue queue;
    queue.start();

    queue.add(1);
    REQUIRE(queue.waitFor(1, std::chrono::milliseconds(500)));

    queue.stop();
    queue.add(2); // should be ignored after stop

    // Brief wait to see if any extra events appear
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto events = queue.events();
    REQUIRE(events.size() == 1);
    CHECK(events[0] == 1);
}
