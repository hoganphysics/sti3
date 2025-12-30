#include <catch2/catch_test_macros.hpp>

#include <sti/utils/EvaluationBarrier.h>

#include <atomic>
#include <condition_variable>
#include <mutex>

using STI::Utils::EvaluationBarrier;

TEST_CASE("EvaluationBarrier: coalesces rapid calls into single action") {
    EvaluationBarrier barrier;
    std::atomic<int> count{0};
    std::mutex m;
    std::condition_variable cv;

    auto action = [&] {
        count.fetch_add(1);
        cv.notify_all();
    };

    barrier.wait(std::chrono::milliseconds(50), action);
    barrier.wait(std::chrono::milliseconds(50), action); // should be ignored while waiting

    std::unique_lock<std::mutex> lock(m);
    cv.wait_for(lock, std::chrono::milliseconds(200), [&] { return count.load() >= 1; });
    barrier.join();
    CHECK(count.load() == 1);
}

TEST_CASE("EvaluationBarrier: can be reused after completion") {
    EvaluationBarrier barrier;
    std::atomic<int> count{0};
    std::mutex m;
    std::condition_variable cv;

    auto action = [&] {
        count.fetch_add(1);
        cv.notify_all();
    };

    barrier.wait(std::chrono::milliseconds(20), action);
    {
        std::unique_lock<std::mutex> lock(m);
        cv.wait_for(lock, std::chrono::milliseconds(200), [&] { return count.load() >= 1; });
    }
    barrier.join();
    REQUIRE(count.load() == 1);

    barrier.wait(std::chrono::milliseconds(20), action);
    {
        std::unique_lock<std::mutex> lock(m);
        cv.wait_for(lock, std::chrono::milliseconds(200), [&] { return count.load() >= 2; });
    }
    barrier.join();
    CHECK(count.load() == 2);
}
