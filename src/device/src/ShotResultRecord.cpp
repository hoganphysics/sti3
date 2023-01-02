#include <sti/engine/ShotResultRecord.h>

#include "CerealArchives.h"
#include <cereal/types/vector.hpp>

using STI::Engine::ShotResultRecord;
using STI::Engine::RecordStatus;

ShotResultRecord::ShotResultRecord() 
: recordStatus(RecordStatus::Unqueried)
{
}

ShotResultRecord::ShotResultRecord(const STI::Device::DeviceID& deviceID) 
: deviceID(deviceID), recordStatus(RecordStatus::Unqueried)
{
}

bool ShotResultRecord::isPartialRecord()
{
    if (recordStatus != RecordStatus::Complete) return true;

    bool isPartial = false;

    for (auto& record : dependencies) {
        if(record.isPartialRecord()) {
            isPartial = true;
            break;
        }
    }
    return isPartial;
}

std::string ShotResultRecord::statusToString(const RecordStatus& status)
{
    //{ Unqueried, Complete, MissingDevice, MissingResults, Error }

    std::string result = "";

    switch (status)
    {
    case RecordStatus::Unqueried:
        result = "Running";
        break;
    case RecordStatus::Complete:
        result = "Complete";
        break;
    case RecordStatus::MissingDevice:
        result = "MissingDevice";
        break;
    case RecordStatus::MissingResults:
        result = "MissingResults";
        break;
    case RecordStatus::Error:
        result = "Error";
        break;    
    default:
        break;
    }

    return result;
}

template<class Archive>
void ShotResultRecord::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("DeviceID", deviceID),
        cereal::make_nvp("RecordStatus", recordStatus), 
        cereal::make_nvp("Dependencies", dependencies)
        );
}


template void ShotResultRecord::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ShotResultRecord::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
