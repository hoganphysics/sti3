#ifndef STI_DEVICE_POSTPROCESSINGMANAGER_H
#define STI_DEVICE_POSTPROCESSINGMANAGER_H

#include <sti/engine/ShotID.h>
#include <sti/device/DeviceID.h>
#include <sti/utils/MetaData.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>


namespace STI
{
namespace Engine
{
class ShotResult;
}

namespace Device
{


//User callback run on a background worker thread when a shot is ready for
//processing. Receives the completed shot's pulled ShotResult (already resolved
//from the owning device by the worker) and the per-request options; returns a
//MetaData result that is broadcast in a PostProcessingCompleteMessage.
using PostProcessingFunction =
    std::function<STI::Utils::MetaData(const std::shared_ptr<STI::Engine::ShotResult>&, const STI::Utils::MetaData&)>;


struct PostProcessingTargetInfo
{
    std::string name;
    std::string description;
};


//Abstract interface following the ChannelManager/AttributeManager Local/Remote
//split template. LocalPostProcessingManager backs it in stidevice;
//RemotePostProcessingManager will back it over CORBA in Phase 2.
class PostProcessingManager
{
public:

    virtual ~PostProcessingManager() = default;

    //Device-author-facing: register a named target and its callback.
    virtual void addPostProcessingTarget(const std::string& name,
                                          PostProcessingFunction function,
                                          const std::string& description = "") = 0;

    //Discovery (interactive sessions, frontend).
    virtual std::vector<PostProcessingTargetInfo> getPostProcessingTargets() const = 0;

    //Entry point called when a shot is ready for processing. Enqueues and returns
    //immediately; returns false if `name` is not a registered target on this
    //device (caller should log, not treat as fatal).
    virtual bool requestPostProcessing(const std::string& name,
                                        const STI::Engine::ShotID& shotID,
                                        const STI::Device::DeviceID& shotOwnerID,
                                        const STI::Utils::MetaData& options) = 0;

    virtual void stop() = 0;
};


} //Device
} //STI

#endif
