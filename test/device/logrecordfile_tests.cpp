#include <catch2/catch_test_macros.hpp>

#include <sti/device/DeviceID.h>
#include <sti/device/LogRecord.h>

#include "LogRecordFile.h"
#include "fileholder_tests_support.h"

#include <filesystem>
#include <string>

using STI::Device::DeviceID;
using STI::Device::LogRecord;
using STI::Device::LogRecordFile;
using STI::Device::LogRecordStatus;

TEST_CASE("LogRecordFile reports missing files", "[log] [logrecordfile]") {
    fileholder_test_support::TempDir tempDir("logrecordfile-missing-");
    auto recordPath = tempDir.path / "logRecord.ini";

    LogRecordFile recordFile(recordPath.string());
    CHECK(recordFile.exists() == false);

    LogRecord loaded;
    CHECK(recordFile.copyRecord(loaded) == false);
}

TEST_CASE("LogRecordFile saves status and merges log names", "[log] [logrecordfile]") {
    fileholder_test_support::TempDir tempDir("logrecordfile-merge-");
    auto recordPath = tempDir.path / "logRecord.ini";
    DeviceID device("LogDevice", "127.0.0.1", 5);

    {
        LogRecordFile recordFile(recordPath.string());
        recordFile.setLogStatus(device, "");
        recordFile.setLogStatus(device, "custom");

        LogRecord stored;
        REQUIRE(recordFile.copyRecord(stored));

        auto it = stored.deviceLogRecords.find(device.getID());
        REQUIRE(it != stored.deviceLogRecords.end());
        CHECK(it->second.status == LogRecordStatus::LogsPresent);
        CHECK(it->second.logNames.count("") == 1);
        CHECK(it->second.logNames.count("custom") == 1);
    }

    {
        // Reload from disk to ensure persistence
        LogRecordFile reloaded(recordPath.string());
        REQUIRE(reloaded.exists());

        LogRecord stored;
        REQUIRE(reloaded.copyRecord(stored));

        auto it = stored.deviceLogRecords.find(device.getID());
        REQUIRE(it != stored.deviceLogRecords.end());
        CHECK(it->second.logNames.size() == 2);
        CHECK(it->second.status == LogRecordStatus::LogsPresent);
    }
}

