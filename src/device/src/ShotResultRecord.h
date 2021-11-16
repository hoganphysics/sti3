
#ifndef STI_ENGINE_SHOTRESULTRECORD_H
#define STI_ENGINE_SHOTRESULTRECORD_H

#include "DeviceID.h"

#include <vector>


namespace STI
{
namespace Engine
{

class ShotResultRecord;

class ShotResultRecord
{
public:

    enum class RecordStatus { Unqueried, Complete, MissingDevice, MissingResults, Error };

    ShotResultRecord() 
    : recordStatus(RecordStatus::Unqueried)
    {
    }

    ShotResultRecord(const STI::Device::DeviceID& deviceID) 
    : deviceID(deviceID), recordStatus(RecordStatus::Unqueried)
    {
    }

    bool isPartialRecord()
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

    
    STI::Device::DeviceID deviceID;
    RecordStatus recordStatus;
    std::vector<ShotResultRecord> dependencies;  //devices this device owns

    template<class Archive>
    void serialize(Archive& archive);
};


} //Engine
} //STI

#endif

