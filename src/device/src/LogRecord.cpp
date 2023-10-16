
#include <sti/device/LogRecord.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/string.hpp>

using STI::Device::LogRecord;
using STI::Device::DeviceLogRecord;


LogRecord::LogRecord()
{
}

LogRecord::~LogRecord()
{
}

template<class Archive>
void DeviceLogRecord::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("deviceID", deviceID),
        cereal::make_nvp("status", status),
        cereal::make_nvp("logNames", logNames)
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

template void LogRecord::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void LogRecord::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

template void DeviceLogRecord::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void DeviceLogRecord::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
