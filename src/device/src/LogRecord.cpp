
#include <sti/device/LogRecord.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/string.hpp>

#include <algorithm>

using STI::Device::LogRecord;
using STI::Device::DeviceLogRecord;
using STI::Device::LogFileRecord;
using STI::Device::LogNameRecord;


LogRecord::LogRecord()
{
}

LogRecord::~LogRecord()
{
}

template<class Archive>
void LogFileRecord::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("id", id),
        cereal::make_nvp("fileID", fileID),
        cereal::make_nvp("bytes", bytes),
        cereal::make_nvp("lineCount", lineCount),
        cereal::make_nvp("firstEntryTime", firstEntryTime),
        cereal::make_nvp("lastEntryTime", lastEntryTime)
		);
}

template<class Archive>
void LogNameRecord::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("logName", logName),
        cereal::make_nvp("files", files),
        cereal::make_nvp("totalBytes", totalBytes),
        cereal::make_nvp("totalLines", totalLines),
        cereal::make_nvp("nextIndex", nextIndex),
        cereal::make_nvp("lastUpdate", lastUpdate)
		);
}

void LogNameRecord::recalculateTotals()
{
    totalBytes = 0;
    totalLines = 0;
    nextIndex = 0;

    bool haveLatest = false;
    STI::Utils::TimeStamp latest;

    for (const auto& entry : files) {
        totalBytes += entry.second.bytes;
        totalLines += entry.second.lineCount;
        nextIndex = std::max(nextIndex, entry.first + 1);

        const auto& candidate = entry.second.lastEntryTime;
        if (!haveLatest || latest < candidate) {
            latest = candidate;
            haveLatest = true;
        }
    }

    if (haveLatest) {
        lastUpdate = latest;
    }
}

void DeviceLogRecord::syncLogNamesFromLogs()
{
    logNames.clear();

    for (auto& entry : logs) {
        entry.second.logName = entry.first;
        entry.second.recalculateTotals();
        logNames.insert(entry.first);
    }
}

template<class Archive>
void DeviceLogRecord::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("deviceID", deviceID),
        cereal::make_nvp("status", status),
        cereal::make_nvp("logNames", logNames),
        cereal::make_nvp("logs", logs)
		);
}

template<class Archive>
void LogRecord::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("TimeStamp", timeStamp),
        cereal::make_nvp("deviceLogRecords", deviceLogRecords)
		);
}

template void LogFileRecord::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void LogFileRecord::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template void LogNameRecord::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void LogNameRecord::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template void DeviceLogRecord::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void DeviceLogRecord::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template void LogRecord::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void LogRecord::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
