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
using STI::Engine::ShotResultStatus;


ShotResult::ShotResult()
: status(ShotResultStatus::Unknown)
{
    measurements = std::make_shared<STI::Engine::MeasurementMap>();
}

ShotResult::ShotResult(const STI::Device::DeviceID& deviceID, std::set<STI::Device::DeviceID>& ownedIDs)
: status(ShotResultStatus::Unknown)
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
void ShotResult::save(Archive& archive) const
{
    archive( 
        cereal::make_nvp("ShotID", sid),
        cereal::make_nvp("playTime", playTime),
        cereal::make_nvp("attributes", attributes), 
        cereal::make_nvp("measurements", measurements),
        cereal::make_nvp("messages", messages),
        cereal::make_nvp("status", status),
        cereal::make_nvp("versions", versions),
        cereal::make_nvp("shotResultRecord", shotResultRecord)
        );
}

template<class Archive>
void ShotResult::load(Archive& archive)
{
    archive( 
        cereal::make_nvp("ShotID", sid),
        cereal::make_nvp("playTime", playTime),
        cereal::make_nvp("attributes", attributes), 
        cereal::make_nvp("measurements", measurements),
        cereal::make_nvp("messages", messages)
        );

    status = ShotResultStatus::Unknown;
    try {
        archive(cereal::make_nvp("status", status));
    }
    catch (const cereal::Exception&) {
        status = ShotResultStatus::Unknown;
    }

    versions.clear();
    try {
        archive(cereal::make_nvp("versions", versions));
    }
    catch (const cereal::Exception&) {
        versions.clear();
    }

    archive(cereal::make_nvp("shotResultRecord", shotResultRecord));
}

std::string STI::Engine::ShotResultStatusToString(const ShotResultStatus& status)
{
    switch (status) {
    case ShotResultStatus::Success:
        return "Success";
    case ShotResultStatus::CompletedWithErrors:
        return "CompletedWithErrors";
    case ShotResultStatus::CanceledByUser:
        return "CanceledByUser";
    case ShotResultStatus::AbortedByError:
        return "AbortedByError";
    case ShotResultStatus::AbortedByTimeout:
        return "AbortedByTimeout";
    case ShotResultStatus::Unknown:
    default:
        return "Unknown";
    }
}

ShotResultStatus STI::Engine::ShotResultStatusFromString(const std::string& status)
{
    if (status == "Success") {
        return ShotResultStatus::Success;
    }
    if (status == "CompletedWithErrors") {
        return ShotResultStatus::CompletedWithErrors;
    }
    if (status == "CanceledByUser") {
        return ShotResultStatus::CanceledByUser;
    }
    if (status == "AbortedByError") {
        return ShotResultStatus::AbortedByError;
    }
    if (status == "AbortedByTimeout") {
        return ShotResultStatus::AbortedByTimeout;
    }
    return ShotResultStatus::Unknown;
}


template void ShotResult::save<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& ) const;
template void ShotResult::load<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
