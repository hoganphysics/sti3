#ifndef STI_DEVICE_LOGID_H
#define STI_DEVICE_LOGID_H

#include <sti/device/DeviceID.h>

#include <string>

namespace STI
{
namespace Device
{


struct LogID
{
    LogID();
    LogID(const DeviceID& deviceID, const std::string& date, const std::string& logName, unsigned index);

	bool operator<(const LogID& rhs) const;
	bool operator==(const LogID& rhs) const;
	bool operator!=(const LogID& rhs) const;

    DeviceID deviceID;
    std::string date;
    std::string logName;
    unsigned index;    //There can be multiple log files for 'logName' on the same day if the files are large.
};


} //Device
} //STI

#endif
