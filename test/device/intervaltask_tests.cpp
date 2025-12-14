#include <catch2/catch_test_macros.hpp>

#include <sti/utils/IntervalTask.h>

#include "task_tests_support.h"

#include <atomic>
#include <chrono>

using STI::Utils::IntervalTask;

TEST_CASE("IntervalTask: run executes callback and repeats") {
    std::atomic<int> runs{0};
    IntervalTask task("interval", 1.0, [&] { runs.fetch_add(1); });

    task.run();
    CHECK(runs.load() == 1);
    CHECK(task.repeat());
    auto wait = task.secondsToNextRun();
    CHECK(wait >= 0.0);
    CHECK(wait <= 1.2);
}

TEST_CASE("IntervalTask: string constructor enforces minimum wait of one second") {
    IntervalTask task("from-string", "00:00:00", [] {});

    auto wait = task.secondsToNextRun();
    CHECK(wait > 0.0);
    CHECK(wait <= 1.1);
}
