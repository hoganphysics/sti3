
#include "ShotResult.h"
#include <sti/engine/ShotID.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>
#include <sti/utils/FileHolder.h>


#include "CerealArchives.h"

#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::ShotResult;


ShotResult::ShotResult()
{
}

ShotResult::ShotResult(const STI::Device::DeviceID deviceID, std::set<STI::Device::DeviceID> ownedIDs)
{
    shotResultRecord.deviceID = deviceID;
    for (auto& id : ownedIDs) {
        shotResultRecord.dependencies.push_back(id);    
    }
}

void ShotResult::deleteShotFiles(ShotResult& shot)
{
//     for (auto& file : shot.timingFiles) {
//         if (file != 0) {
//             file->deleteFile();
//         }
//     }

    if (shot.measurements != 0) {
        for (auto& meas : *(shot.measurements)) {
            if (meas != 0 && meas->data().isType(STI::Utils::MixedValueType::File)) {
                meas->data().getFile()->deleteFile();
            }
        }           
    }
}

template<class Archive>
void ShotResult::serialize(Archive& archive)
{
    archive( 
        cereal::make_nvp("ShotID", sid),
        cereal::make_nvp("playTime", playTime),
        cereal::make_nvp("parseResult", parseResult),
        cereal::make_nvp("engineParseResult", engineParseResult),
        cereal::make_nvp("Attributes", attributes), 
        cereal::make_nvp("Measurements", measurements),
        // cereal::make_nvp("TimingFiles", timingFiles),
        // cereal::make_nvp("ParsedEvents", parsedEvents),
        cereal::make_nvp("ShotResultRecord", shotResultRecord)
        );
}


template void ShotResult::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ShotResult::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
