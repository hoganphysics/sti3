#ifndef STI_DEVICE_LOGRECORD_H
#define STI_DEVICE_LOGRECORD_H


#include <sti/device/DeviceID.h>
#include <sti/utils/TimeStamp.h>

#include <map>


namespace STI
{
namespace Device
{

enum class LogRecordStatus { Unqueried, LogsPresent, NoLogs, Error };

class LogRecord
{
public:

    LogRecord();
    ~LogRecord();

    STI::Utils::TimeStamp timeStamp;
    std::map<std::string, LogRecordStatus> loggedIDs;

	template<class Archive>
	void serialize(Archive& archive);
};



} //Device
} //STI

#endif
