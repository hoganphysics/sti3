
#include <sti/device/LogID.h>

#include <sti/utils/TimeStamp.h>

#include "CerealArchives.h"
#include <cereal/types/string.hpp>

using STI::Device::LogID;


LogID::LogID() 
{
}

LogID::LogID(const DeviceID& deviceID, const std::string& date, const std::string& logName, unsigned index)
: deviceID(deviceID), date(date), logName(logName), index(index)
{
}

bool LogID::operator<(const LogID& rhs) const
{
    if (deviceID != rhs.deviceID) {
        return deviceID < rhs.deviceID;
    }
    if (logName != rhs.logName) {
        return logName < rhs.logName;
    }
    
    auto thisDate = STI::Utils::TimeStamp::fromString(date);
    auto rhsDate = STI::Utils::TimeStamp::fromString(rhs.date);
    
    if (!thisDate.isSameDate(rhsDate)) {
        return thisDate < rhsDate;
    }

    return index < rhs.index;
}

bool LogID::operator==(const LogID& rhs) const
{
    auto thisDate = STI::Utils::TimeStamp::fromString(date);
    auto rhsDate = STI::Utils::TimeStamp::fromString(rhs.date);

    return deviceID == rhs.deviceID && logName == rhs.logName && thisDate == rhsDate && index == rhs.index;
}

bool LogID::operator!=(const LogID& rhs) const
{
    return !((*this) == rhs);
}

template<class Archive>
void LogID::serialize(Archive& archive)
{
    archive(
        cereal::make_nvp("deviceID", deviceID),
        cereal::make_nvp("date", date),
        cereal::make_nvp("logName", logName),
        cereal::make_nvp("index", index)
        );
}

template void LogID::serialize<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&);
template void LogID::serialize<cereal::XMLInputArchive>(cereal::XMLInputArchive&);
