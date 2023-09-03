#ifndef STI_DEVICE_LOGRECORD_H
#define STI_DEVICE_LOGRECORD_H


#include <sti/device/DeviceID.h>
#include <sti/utils/TimeStamp.h>

#include <map>
#include <set>
#include <string>


namespace STI
{
namespace Device
{

enum class LogRecordStatus { Unqueried, LogsPresent, NoLogs, Error };

struct DeviceLogRecord
{
    std::string deviceID;
    LogRecordStatus status;
    std::set<std::string> logNames;

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
