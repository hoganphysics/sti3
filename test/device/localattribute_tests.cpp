#include <catch2/catch_test_macros.hpp>

#include <sti/device/LocalAttribute.h>

#include "localattribute_tests_support.h"

using local_attribute_test_support::RefreshRecorder;
using STI::Device::LocalAttribute;

TEST_CASE("LocalAttribute: allowed values and groups") {
    LocalAttribute attr("Group::Attr", "initial", {"initial", "next"});

    CHECK(attr.getKey() == "Group::Attr");
    CHECK(attr.getGroup() == "Group");
    CHECK(attr.getAllowedValues().size() == 2);
    CHECK(attr.getValue() == "initial");

    CHECK(attr.setValue("next"));
    CHECK(attr.getValue() == "next");

    CHECK_FALSE(attr.setValue("forbidden"));
    CHECK(attr.getValue() == "next");
}

TEST_CASE("LocalAttribute: setter and refresher drive value and refresh events") {
    LocalAttribute attr("simple", "start");
    RefreshRecorder recorder;
    attr.addRefreshListener(&recorder);

    int refreshCount = 0;
    attr.setRefresher([&](std::string& result) {
        ++refreshCount;
        result = "refreshed_" + std::to_string(refreshCount);
        return true;
    });

    bool setterCalled = false;
    attr.setSetter([&](const std::string& value) {
        setterCalled = true;
        return value != "reject";
    });

    REQUIRE(attr.setValue("next"));
    CHECK(setterCalled);
    CHECK(recorder.waitForCount(1, std::chrono::milliseconds(200)));
    CHECK(recorder.lastKey == "simple");
    CHECK(recorder.lastValue.rfind("refreshed_", 0) == 0);
    const auto firstValue = attr.getValue();
    CHECK(firstValue == recorder.lastValue);
    CHECK(refreshCount >= 1);

    CHECK_FALSE(attr.setValue("reject"));
    CHECK(attr.getValue() == recorder.lastValue);
}

TEST_CASE("LocalAttribute: refreshValue emits an event when the refreshed value changes") {
    LocalAttribute attr("simple", "start");
    RefreshRecorder recorder;
    attr.addRefreshListener(&recorder);

    std::string refreshedValue = "start";
    attr.setRefresher([&]() { return refreshedValue; });

    refreshedValue = "changed";
    attr.refreshValue();

    CHECK(attr.getValue() == "changed");
    CHECK(recorder.count == 1);
    CHECK(recorder.lastKey == "simple");
    CHECK(recorder.lastValue == "changed");

    attr.refreshValue();
    CHECK(recorder.count == 1);
}
