#include <catch2/catch_test_macros.hpp>

#include <sti/utils/MixedValue.h>
#include <sti/utils/Task.h>

#include "task_tests_support.h"

#include <chrono>
#include <string>

using task_test_support::DummyTask;
using STI::Utils::MixedValue;
using STI::Utils::TaskStatus;

TEST_CASE("Task: defaults to active and stores ID") {
    DummyTask task("task-1");

    CHECK(task.getID() == "task-1");
    CHECK(task.isActive());
    CHECK(task.getStatus() == TaskStatus::Active);
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
