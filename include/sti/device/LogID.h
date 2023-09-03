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
    DeviceID deviceID;
    std::string date;
    std::string logName;
    unsigned index;    //There can be multiple log files for 'logName' on the same day if the files are large.
};


} //Device
} //STI

#endif
