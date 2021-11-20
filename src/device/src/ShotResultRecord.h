
#ifndef STI_ENGINE_SHOTRESULTRECORD_H
#define STI_ENGINE_SHOTRESULTRECORD_H

#include "DeviceID.h"

#include <vector>


namespace STI
{
namespace Engine
{

class ShotResultRecord;

enum class RecordStatus { Unqueried, Complete, MissingDevice, MissingResults, Error };


class ShotResultRecord
{
public: 

    ShotResultRecord();
    ShotResultRecord(const STI::Device::DeviceID& deviceID) ;

    bool isPartialRecord();
    
    STI::Device::DeviceID deviceID;
    RecordStatus recordStatus;
    std::vector<ShotResultRecord> dependencies;  //devices this device owns

    template<class Archive>
    void serialize(Archive& archive);
};


} //Engine
} //STI

#endif

