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
} // namespace

TEST_CASE("EventQueue: processes events and honors priority order") {
    RecordingQueue queue;
    queue.start();

    queue.add(1);
    queue.add(2);
    queue.addPriority(99);

    REQUIRE(queue.waitFor(3, std::chrono::milliseconds(500)));

    auto events = queue.events();
    REQUIRE(events.size() == 3);
    CHECK(events[0] == 99); // priority goes first
    CHECK(events[1] == 1);
    CHECK(events[2] == 2);

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
