#include <catch2/catch_test_macros.hpp>

#include <sti/device/DeviceID.h>
#include <sti/device/LogID.h>
#include <sti/utils/TimeStamp.h>

#include <string>

using STI::Device::DeviceID;
using STI::Device::LogID;
using STI::Utils::TimeStamp;

TEST_CASE("LogID ordering compares device, log name, date, then index", "[log] [logid]") {
    DeviceID devA("DeviceA", "127.0.0.1", 1);
    DeviceID devB("DeviceB", "127.0.0.1", 1);

    auto tsEarly = TimeStamp(2024, 1, 1, 0, 0, 0, 0, 0, 0).toString();
    auto tsLate = TimeStamp(2024, 1, 2, 0, 0, 0, 0, 0, 0).toString();

    LogID idDevA(devA, tsEarly, "alpha", 1);
    LogID idDevB(devB, tsEarly, "alpha", 1);
    LogID idAlpha(devA, tsEarly, "alpha", 1);
    LogID idBeta(devA, tsEarly, "beta", 1);
    LogID idEarly(devA, tsEarly, "alpha", 5);
    LogID idLate(devA, tsLate, "alpha", 0);
    LogID idLowIndex(devA, tsLate, "alpha", 1);
    LogID idHighIndex(devA, tsLate, "alpha", 2);

    CHECK(idDevA < idDevB);                    // deviceID ordering
    CHECK(idAlpha < idBeta);                   // logName ordering
    CHECK(idEarly < idLate);                   // date ordering (ignores time on same date)
    CHECK(idLowIndex < idHighIndex);           // index ordering on same date/name/device
}

TEST_CASE("LogID equality requires matching time stamp and index", "[log] [logid]") {
    DeviceID dev("DeviceX", "127.0.0.1", 3);
    auto dateMorning = TimeStamp(2024, 2, 3, 8, 0, 0, 0, 0, 0).toString();
    auto dateEvening = TimeStamp(2024, 2, 3, 20, 0, 0, 0, 0, 0).toString();

    LogID idMorning(dev, dateMorning, "named", 0);
    LogID idMorningCopy(dev, dateMorning, "named", 0);
    LogID idEvening(dev, dateEvening, "named", 0);
    LogID idIndex(dev, dateMorning, "named", 1);

    CHECK(idMorning == idMorningCopy);
    CHECK(idMorning != idEvening); // same calendar date, different time is not equal
    CHECK(idMorning != idIndex);
}

