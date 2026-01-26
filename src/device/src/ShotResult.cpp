#include <sti/engine/ShotResult.h>

#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ShotID.h>
#include <sti/utils/FileHolder.h>

#include "CerealArchives.h"
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

using STI::Engine::ShotResult;


ShotResult::ShotResult()
{
    measurements = std::make_shared<STI::Engine::MeasurementMap>();
}

ShotResult::ShotResult(const STI::Device::DeviceID& deviceID, std::set<STI::Device::DeviceID>& ownedIDs)
{
    measurements = std::make_shared<STI::Engine::MeasurementMap>();

    shotResultRecord.deviceID = deviceID;
    for (auto& id : ownedIDs) {
        shotResultRecord.dependencies.push_back(id);    
    }
}

void ShotResult::deleteFiles(ShotResult& shot, const std::shared_ptr<STI::Utils::FileServer>& fileServer)
{
    if (shot.measurements == 0) return;
    if (fileServer == 0) return;

    for (auto& tuple : *shot.measurements) {            
        for (auto& meas : tuple.second) {
            if (meas != 0 && meas->data().isType(STI::Utils::MixedValueType::File)) {
                fileServer->deleteFile(meas->data().getFileID());
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
        cereal::make_nvp("attributes", attributes), 
        cereal::make_nvp("measurements", measurements),
        cereal::make_nvp("messages", messages),
        cereal::make_nvp("shotResultRecord", shotResultRecord)
        );
}


template void ShotResult::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void ShotResult::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
