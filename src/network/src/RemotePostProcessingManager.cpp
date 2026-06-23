#include "RemotePostProcessingManager.h"

#include "NetworkConvert.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_PostProcessing.h"

#include <sti/engine/ShotID.h>
#include <sti/device/DeviceID.h>
#include <sti/utils/MetaData.h>
#include <sti/utils/MixedValue.h>

#include <string>

using STI::Network::RemotePostProcessingManager;
using STI::Network::convert;
using STI::TNetwork::TReferenceHolder;
using STI::TNetwork::TPostProcessingManager;
using STI::TNetwork::TPostProcessingTargetInfoSeq;
using STI::TNetwork::TShotID;
using STI::TNetwork::TDeviceID;
using STI::TNetwork::TMixedValue;
using STI::Device::PostProcessingTargetInfo;
using STI::Engine::ShotID;
using STI::Device::DeviceID;
using STI::Utils::MetaData;
using STI::Utils::MixedValue;


RemotePostProcessingManager::RemotePostProcessingManager(::STI::TNetwork::TPostProcessingManager_var manager, const std::string& originID)
: TReferenceHolder<TPostProcessingManager>(manager)
{
}

RemotePostProcessingManager::~RemotePostProcessingManager()
{
}

std::vector<PostProcessingTargetInfo> RemotePostProcessingManager::getPostProcessingTargets() const
{
    std::unique_lock<std::mutex> lock(postProcessingMutex);

    std::vector<PostProcessingTargetInfo> result;

    if (isDisabled()) return result;

    try {
        STI::TNetwork::TPostProcessingTargetInfoSeq_var tTargets(new TPostProcessingTargetInfoSeq());
        getTRef()->getTargets(tTargets);	//remote call

        convert<STI::TNetwork::TPostProcessingTargetInfo, PostProcessingTargetInfo>(tTargets.in(), result);   //seq -> vector
    }
    catch (CORBA::TRANSIENT&) {
    }
    catch (CORBA::SystemException&) {
    }
    catch (CORBA::Exception&) {
    }

    return result;
}

bool RemotePostProcessingManager::requestPostProcessing(const std::string& name,
                                                        const ShotID& shotID,
                                                        const DeviceID& shotOwnerID,
                                                        const MetaData& options)
{
    std::unique_lock<std::mutex> lock(postProcessingMutex);

    if (isDisabled()) return false;

    bool success = false;

    try {
        success = getTRef()->requestPostProcessing(
            name.c_str(),
            convert<ShotID, TShotID>(shotID),
            convert<DeviceID, TDeviceID>(shotOwnerID),
            convert<MixedValue, TMixedValue>(options.getMetaData()));	//remote call
    }
    catch (CORBA::TRANSIENT&) {
    }
    catch (CORBA::SystemException&) {
    }
    catch (CORBA::Exception&) {
    }

    return success;
}

void RemotePostProcessingManager::stop()
{
    //Worker thread lives on the device that owns the manager; nothing to stop here.
}

bool RemotePostProcessingManager::ping() const
{
    if (isDisabled()) return false;

    try {
        return getTRef()->ping();
    }
    catch (CORBA::Exception&) {
    }
    return false;
}
