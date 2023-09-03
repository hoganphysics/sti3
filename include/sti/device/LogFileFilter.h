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

    //Index range of 'logName' log for a given day.
    //Negative values allowed, measured from end (-1 is last)
    int startIndex;
    int endIndex;
};


} //Device
} //STI

#endif
