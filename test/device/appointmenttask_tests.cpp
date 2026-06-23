#include <catch2/catch_test_macros.hpp>

#include <sti/utils/AppointmentTask.h>

#include "task_tests_support.h"

#include <atomic>
#include <chrono>

using STI::Utils::AppointmentTask;

TEST_CASE("AppointmentTask: once tasks do not repeat") {
    std::atomic<int> runs{0};
    auto timeOfDay = task_test_support::timeStringInFuture(std::chrono::seconds(60));
    AppointmentTask task("once", timeOfDay, AppointmentTask::AppointmentRepeatType::Once, [&] { runs.fetch_add(1); });

    task.runNow();

    CHECK(runs.load() == 1);
    CHECK(task.hasLastRunTime());
    CHECK_FALSE(task.repeat());
    CHECK(task.secondsToNextRun() > 0.0);
}

TEST_CASE("AppointmentTask: everyday tasks repeat") {
    std::atomic<int> runs{0};
    auto timeOfDay = task_test_support::timeStringInFuture(std::chrono::seconds(30));
    AppointmentTask task("daily", timeOfDay, AppointmentTask::AppointmentRepeatType::Everyday, [&] { runs.fetch_add(1); });

    task.runNow();

    CHECK(runs.load() == 1);
    CHECK(task.hasLastRunTime());
    CHECK(task.repeat());
    CHECK(task.secondsToNextRun() > 0.0);
}
