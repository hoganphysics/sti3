#include <catch2/catch_test_macros.hpp>

#include <sti/utils/TimeStamp.h>

using STI::Utils::TimeStamp;

TEST_CASE("TimeStamp: formatting and accessors") {
    TimeStamp ts(2023, 9, 15, 11, 45, 9, 235, 567, 129);

    CHECK(ts.date_YYYY_MM_DD("/") == "2023/09/15");
    CHECK(ts.date_YYYY_MM_DD("-") == "2023-09-15");
    CHECK(ts.time_hh_mm_ss(":") == "11:45:09");
    CHECK(ts.time_mmmuuunnn(".") == "235.567.129");
    CHECK(ts.time_hh_mm_ss_mmmuuunnn() == "11_45_09_235567129");
    CHECK(ts.toString() == "2023/09/15|11:45:09.235.567.129");

    CHECK(ts.year() == 2023);
    CHECK(ts.month() == 9);
    CHECK(ts.day() == 15);
    CHECK(ts.hour() == 11);
    CHECK(ts.minute() == 45);
    CHECK(ts.sec() == 9);
    CHECK(ts.millis() == 235);
    CHECK(ts.micros() == 567);
    CHECK(ts.nanos() == 129);
}

TEST_CASE("TimeStamp: parsing and comparison") {
    auto ts = TimeStamp::fromString("2023/09/15|11:45:09.235.567.129");
    TimeStamp same(2023, 9, 15, 11, 45, 9, 235, 567, 129);
    TimeStamp earlier(2023, 9, 15, 11, 44, 0, 0, 0, 0);

    CHECK(ts == same);
    CHECK(ts != earlier);
    CHECK(earlier < ts);
    CHECK(earlier <= ts);
    CHECK(ts.isSameDate(earlier));
}

TEST_CASE("TimeStamp: incremental add methods update fields") {
    TimeStamp ts(2023, 1, 1, 0, 0, 0, 0, 0, 0);

    ts.add_day();
    ts.add_hour(2);
    ts.add_minute(3);
    ts.add_sec(4);
    ts.add_ms(5);
    ts.add_ns(6);

    CHECK(ts.day() == 2);
    CHECK(ts.hour() == 2);
    CHECK(ts.minute() == 3);
    CHECK(ts.sec() == 4);
    CHECK(ts.millis() == 5);
    CHECK(ts.nanos() == 6);
}
