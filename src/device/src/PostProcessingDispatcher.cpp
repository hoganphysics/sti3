#include "PostProcessingDispatcher.h"
#include "LocalEventEngineScheduler.h"
#include "LocalEventEngine.h"

#include <sti/device/DeviceMessage.h>
#include <sti/device/PostProcessingManager.h>
#include <sti/device/Device.h>
#include <sti/device/Logger.h>
#include <sti/engine/PostProcessRequest.h>
#include <sti/engine/ShotID.h>

#include <vector>

using STI::Device::PostProcessingDispatcher;
using STI::Device::DeviceID;
using STI::Device::DeviceCollection;
using STI::Device::Device;
using STI::Device::PostProcessingManager;
using STI::Device::EngineSchedulerMessage;
using STI::Device::Logger;
using STI::Engine::LocalEventEngineScheduler;
using STI::Engine::PostProcessRequest;
using STI::Engine::ShotID;


PostProcessingDispatcher::PostProcessingDispatcher(const DeviceID& localDeviceID,
                                                   const std::shared_ptr<LocalEventEngineScheduler>& scheduler,
                                                   const std::shared_ptr<DeviceCollection>& deviceCollection,
                                                   const std::shared_ptr<PostProcessingManager>& localManager,
                                                   Logger* logger)
: localDeviceID(localDeviceID), scheduler(scheduler), deviceCollection(deviceCollection),
  localManager(localManager), logger(logger)
{
}

void PostProcessingDispatcher::handleMessage(const std::shared_ptr<EngineSchedulerMessage>& mess)
{
    if (mess == 0 || scheduler == 0) {
        return;
    }
    if (mess->schedulerMessageType != EngineSchedulerMessage::SchedulerMessageType::PlayComplete) {
        return;
    }

    const ShotID& sid = mess->jobID.sid;

    //The resolved post-processing side-list lives on the engine that parsed and
    //played this shot (job owner only). Pull it off the engine the PlayComplete
    //message carried; non-owner engines have an empty list, so this is a no-op.
    auto engine = std::dynamic_pointer_cast<STI::Engine::LocalEventEngine>(mess->getEngine());
    if (engine == 0) {
        return;
    }

    std::vector<PostProcessRequest> requests = engine->takeResolvedPostProcessRequests();
    if (requests.empty()) {
        return;   //no post-processing requests for this shot
    }

    for (auto& request : requests) {
        dispatchRequest(request, sid);
    }
}

void PostProcessingDispatcher::dispatchRequest(const PostProcessRequest& request, const ShotID& shotID)
{
    const DeviceID targetID = request.target().device().deviceID();
    const std::string targetName = request.target().name();

    //Resolve the target device's PostProcessingManager (local manager when the
    //target is this device, otherwise via the abstract DeviceCollection — which
    //yields a RemoteDevice's RemotePostProcessingManager in Phase 2).
    std::shared_ptr<PostProcessingManager> ppm;
    if (targetID == localDeviceID) {
        ppm = localManager;
    }
    else if (deviceCollection != 0) {
        std::shared_ptr<Device> device;
        if (deviceCollection->get(targetID, device) && device != 0) {
            device->getPostProcessingManager(ppm);
        }
    }

    if (ppm == 0) {
        //Resolved at parse time but unreachable now (best-effort; shot already played).
        if (logger != 0) {
            (*logger) << "PostProcessing: target device '" << targetID.getID()
                      << "' unreachable for target '" << targetName << "'\n";
        }
        return;
    }

    //shotOwnerID is this device: it owns the play job, and its PersistenceManager
    //holds the aggregated ShotResult the target will pull.
    if (!ppm->requestPostProcessing(targetName, shotID, localDeviceID, request.options())) {
        if (logger != 0) {
            (*logger) << "PostProcessing: target '" << targetName << "' not registered on device '"
                      << targetID.getID() << "'\n";
        }
    }
}
