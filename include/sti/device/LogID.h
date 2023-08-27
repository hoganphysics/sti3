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
};


} //Device
} //STI

#endif
