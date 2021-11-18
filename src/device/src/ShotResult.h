
#ifndef STI_ENGINE_SHOTRESULT_H
#define STI_ENGINE_SHOTRESULT_H

#include "DeviceID.h"
#include "fwd/Measurement_fwd.h"
#include "fwd/RawEvent_fwd.h"
#include "ShotID.h"
#include "utils/FileHolder.h"
#include "Attribute.h"
#include "ShotResultRecord.h"
#include "Measurement.h"


#include <vector>
#include <memory>
#include <map>
#include <string>
#include <set>


namespace STI
{
namespace Engine
{


class ShotResult
{
public:

    ShotResult();
    ShotResult(const STI::Device::DeviceID deviceID, std::set<STI::Device::DeviceID> ownedIDs);

    ShotID sid; //contains a record of shot type (single, sequence, undocumented)

    DeviceEventMap parsedEvents;
    std::vector<std::shared_ptr<STI::Utils::FileHolder>> timingFiles;
    std::shared_ptr<MeasurementVector> measurements;
    //std::vector<std::shared_ptr<STI::Device::Attribute>> attributes;
    std::map<STI::Device::DeviceID, std::map<std::string, std::string>> attributes;

    //The result is stored by the device in a repository. Initially, only the local device data is available.
    //Data from other (owned) devices must be collected. If it isn't all collected, the result is a partial record.
    //This is the list of devices (owned by the local device) that have not been collected yet.
    //It doubles as a record of the owned devices for this shot.
    //std::vector<STI::Device::DeviceID> missingDependencies; 
    ShotResultRecord shotResultRecord;

    static void deleteShotFiles(ShotResult& shot);

    template<class Archive>
    void serialize(Archive& archive);
};


} //Engine
} //STI

#endif

