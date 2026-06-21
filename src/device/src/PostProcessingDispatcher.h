#ifndef STI_DEVICE_POSTPROCESSINGDISPATCHER_H
#define STI_DEVICE_POSTPROCESSINGDISPATCHER_H

#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceCollection.h>

#include <memory>


namespace STI
{

namespace Engine
{
class LocalEventEngineScheduler;
class PostProcessRequest;
class ShotID;
}

namespace Device
{

class EngineSchedulerMessage;
class PostProcessingManager;
class Logger;


//Listens for PlayComplete EngineSchedulerMessages (the same notification
//ResultTicketManager consumes) and dispatches resolved post-processing requests
//to their target devices. Runs on the dispatcher thread, never under the
//scheduler's jobMutex, so it cannot stall jobComplete(); PlayComplete also
//guarantees the shot's results are persisted (see docs/notes/postProcess.md).
class PostProcessingDispatcher : public DeviceMessageListener<EngineSchedulerMessage>
{
public:

    PostProcessingDispatcher(const DeviceID& localDeviceID,
                             const std::shared_ptr<STI::Engine::LocalEventEngineScheduler>& scheduler,
                             const std::shared_ptr<DeviceCollection>& deviceCollection,
                             const std::shared_ptr<PostProcessingManager>& localManager,
                             Logger* logger = nullptr);

    void handleMessage(const std::shared_ptr<EngineSchedulerMessage>& mess) override;

private:

    void dispatchRequest(const STI::Engine::PostProcessRequest& request, const STI::Engine::ShotID& shotID);

    DeviceID localDeviceID;
    std::shared_ptr<STI::Engine::LocalEventEngineScheduler> scheduler;
    std::shared_ptr<DeviceCollection> deviceCollection;
    std::shared_ptr<PostProcessingManager> localManager;
    Logger* logger;
};


} //Device
} //STI

#endif
