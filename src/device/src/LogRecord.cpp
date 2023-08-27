
#include <sti/device/LogRecord.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

using STI::Device::LogRecord;

LogRecord::LogRecord()
{
}

LogRecord::~LogRecord()
{
}

template<class Archive>
void LogRecord::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("TimeStamp", timeStamp),
        cereal::make_nvp("DeviceIDs", loggedIDs)
		);
}


template void LogRecord::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void LogRecord::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

