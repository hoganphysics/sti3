#ifndef STI_ENGINE_SHOTRESULT_H
#define STI_ENGINE_SHOTRESULT_H

#include <sti/device/DeviceID.h>
#include <sti/fwd/Measurement_fwd.h>
#include <sti/engine/ShotID.h>
#include <sti/device/Attribute.h>
#include <sti/device/VersionInfo.h>
#include <sti/engine/EnginePlayingMessage.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/ShotResultRecord.h>
#include <sti/utils/FileServer.h>

#include <memory>
#include <map>
#include <set>
#include <string>


namespace STI
{
namespace Engine
{

enum class ShotResultStatus {
    Unknown,
    Success,
    CompletedWithErrors,
    CanceledByUser,
    AbortedByError,
    AbortedByTimeout
};


class ShotResult
{
public:

    ShotResult();
    ShotResult(const STI::Device::DeviceID& deviceID, std::set<STI::Device::DeviceID>& ownedIDs);

    ShotID sid;
	STI::Utils::TimeStamp playTime;

    std::shared_ptr<STI::Engine::MeasurementMap> measurements;
    std::map<STI::Device::DeviceID, std::map<std::string, std::string>> attributes;
    std::map<STI::Device::DeviceID, std::vector<STI::Device::VersionInfo>> versions;
    std::vector<EnginePlayingMessage> messages;

    ShotResultStatus status;

    //The result is stored by the device in a repository. Initially, only the local device data is available.
    //Data from other (owned) devices must be collected. If it isn't all collected, the result is a partial record.
    //This is the list of devices (owned by the local device) that have not been collected yet.
    //It doubles as a record of the owned devices for this shot.
    //std::vector<STI::Device::DeviceID> missingDependencies; 
    ShotResultRecord shotResultRecord;

    static void deleteFiles(ShotResult& shot, const std::shared_ptr<STI::Utils::FileServer>& fileServer);

    template<class Archive>
    void save(Archive& archive) const;

    template<class Archive>
    void load(Archive& archive);
};

std::string ShotResultStatusToString(const ShotResultStatus& status);
ShotResultStatus ShotResultStatusFromString(const std::string& status);


} //Engine
} //STI

#endif
