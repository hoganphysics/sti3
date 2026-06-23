#include "TPostProcessingManager_i.h"

#include "NetworkConvert.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_PostProcessing.h"

#include <sti/device/PostProcessingManager.h>
#include <sti/engine/ShotID.h>
#include <sti/device/DeviceID.h>
#include <sti/utils/MetaData.h>
#include <sti/utils/MixedValue.h>

#include <string>
#include <vector>

using STI::TNetwork::TPostProcessingManager_i;
using STI::Network::convert;
using STI::Device::PostProcessingTargetInfo;
using STI::Engine::ShotID;
using STI::Device::DeviceID;
using STI::Utils::MetaData;
using STI::Utils::MixedValue;
using STI::TNetwork::TShotID;
using STI::TNetwork::TDeviceID;
using STI::TNetwork::TMixedValue;
using STI::TNetwork::TPostProcessingTargetInfo;
using STI::TNetwork::TPostProcessingTargetInfoSeq;


TPostProcessingManager_i::TPostProcessingManager_i(const std::shared_ptr<STI::Device::Device>& device)
{
    if (device != 0) {
        device->getPostProcessingManager(postProcessingManager);
    }
}

TPostProcessingManager_i::~TPostProcessingManager_i()
{
}

::CORBA::Boolean TPostProcessingManager_i::requestPostProcessing(const char* name, const TShotID& shotID,
                                                                const TDeviceID& shotOwnerID, const TMixedValue& options)
{
    if (postProcessingManager == 0) {
        return false;
    }

    return postProcessingManager->requestPostProcessing(
        std::string(name),
        convert<TShotID, ShotID>(shotID),
        convert<TDeviceID, DeviceID>(shotOwnerID),
        MetaData(convert<TMixedValue, MixedValue>(options)));
}

void TPostProcessingManager_i::getTargets(TPostProcessingTargetInfoSeq_out targets)
{
    targets = new TPostProcessingTargetInfoSeq();

    if (postProcessingManager == 0) {
        return;
    }

    auto localTargets = postProcessingManager->getPostProcessingTargets();

    TPostProcessingTargetInfoSeq_var tTargets(new TPostProcessingTargetInfoSeq());
    convert<STI::Device::PostProcessingTargetInfo, TPostProcessingTargetInfo>(localTargets, tTargets.inout());   //vector -> seq

    (*targets) = tTargets;
}

::CORBA::Boolean TPostProcessingManager_i::ping()
{
    return true;
}
