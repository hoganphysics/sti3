#ifndef STI_DEVICE_LOGFILEFILTER_H
#define STI_DEVICE_LOGFILEFILTER_H

#include <string>


namespace STI
{
namespace Device
{


struct LogFileFilter
{
    std::string logName;
    std::string startDate;
    std::string endDate;
};


} //Device
} //STI

#endif
