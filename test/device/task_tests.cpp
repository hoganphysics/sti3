#include <catch2/catch_test_macros.hpp>

#include <sti/utils/MixedValue.h>
#include <sti/utils/Task.h>

#include "task_tests_support.h"

#include <chrono>
#include <stdexcept>
#include <string>

using task_test_support::DummyTask;
using STI::Utils::MixedValue;
using STI::Utils::TaskStatus;

TEST_CASE("Task: defaults to active and stores ID") {
    DummyTask task("task-1");

    CHECK(task.getID() == "task-1");
    CHECK(task.isActive());
    CHECK(task.getStatus() == TaskStatus::Active);
    CHECK_FALSE(task.hasLastRunTime());
    CHECK_FALSE(task.getLastRunTime().has_value());
}

TEST_CASE("Task: stores and retrieves metadata entries") {
    DummyTask task("meta-task");

    task.addMetaData("iterations", 3);
    task.addMetaData("label", std::string("hello"));

    MixedValue count = task.getMetaData("iterations");
    MixedValue label = task.getMetaData("label");

    CHECK(count.getInt() == 3);
    CHECK(label.getString() == "hello");

    MixedValue missing = task.getMetaData("missing");
    CHECK(missing.isEmpty());
}

TEST_CASE("Task: comparison follows next run time") {
    using namespace std::chrono_literals;

    DummyTask sooner("sooner", 0ms);
    DummyTask later("later", 50ms);

    CHECK(sooner < later);
    CHECK_FALSE(later < sooner);
    CHECK_FALSE(sooner == later);
}

TEST_CASE("Task: runNow stores returned timestamp after successful run") {
    DummyTask task("run-now");

    const auto runTime = task.runNow();
    auto lastRunTime = task.getLastRunTime();

    REQUIRE(lastRunTime.has_value());
    CHECK(lastRunTime.value() == runTime);
    CHECK(task.hasLastRunTime());
    CHECK(task.getRunCount() == 1);
}

TEST_CASE("Task: runNow does not update last run time when run throws") {
    DummyTask task("throw-run");
    task.setThrowOnRun(true);

    CHECK_THROWS_AS(task.runNow(), std::runtime_error);
    CHECK_FALSE(task.hasLastRunTime());
    CHECK_FALSE(task.getLastRunTime().has_value());
    CHECK(task.getRunCount() == 0);
}
