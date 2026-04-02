#ifndef STI_DEVICE_LOGRECORD_H
#define STI_DEVICE_LOGRECORD_H

#include <sti/device/LogID.h>
#include <sti/utils/FileID.h>
#include <sti/utils/TimeStamp.h>

#include <cstdint>
#include <map>
#include <set>
#include <string>


namespace STI
{
namespace Device
{

enum class LogRecordStatus { Unqueried, LogsPresent, NoLogs, Error };

struct LogFileRecord
{
    LogID id;
    STI::Utils::FileID fileID;

    std::uint64_t bytes = 0;
    std::uint64_t lineCount = 0;

    STI::Utils::TimeStamp firstEntryTime;
    STI::Utils::TimeStamp lastEntryTime;

    template<class Archive>
    void serialize(Archive& archive);
};

struct LogNameRecord
{
    std::string logName;
    std::map<unsigned, LogFileRecord> files;

    std::uint64_t totalBytes = 0;
    std::uint64_t totalLines = 0;

    unsigned nextIndex = 0;
    STI::Utils::TimeStamp lastUpdate;

    void recalculateTotals();

    template<class Archive>
    void serialize(Archive& archive);
};

struct DeviceLogRecord
{
    std::string deviceID;
    LogRecordStatus status;

    // Legacy name cache retained for staged compatibility with the current
    // network layer. Local device/library code should prefer `logs`.
    std::set<std::string> logNames;
    std::map<std::string, LogNameRecord> logs;

    void syncLogNamesFromLogs();

    template<class Archive>
	void serialize(Archive& archive);

    static std::string logRecordStatusToString(const LogRecordStatus& status)
    {
        switch(status) {
            case LogRecordStatus::Unqueried:
                return "Unqueried";
            case LogRecordStatus::LogsPresent:
                return "LogsPresent";
            case LogRecordStatus::NoLogs:
                return "NoLogs";
            case LogRecordStatus::Error:
                return "Error";
        }
        return "Error";
    }
};


class LogRecord
{
public:

    LogRecord();
    ~LogRecord();

    STI::Utils::TimeStamp timeStamp;
    std::map<std::string, DeviceLogRecord> deviceLogRecords;    // {DeviceID, record}

	template<class Archive>
	void serialize(Archive& archive);
};



} //Device
} //STI

#endif
