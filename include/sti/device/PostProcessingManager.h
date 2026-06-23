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


//Author-supplied hint describing one option a target accepts in its postProcess()
//options payload. These are documentation only -- they are not validated against
//the options actually passed; a device author lists them so callers can discover
//what a target understands.
struct PostProcessingOptionInfo
{
    std::string name;
    std::string description;
};


struct PostProcessingTargetInfo
{
    std::string name;
    std::string description;
    std::vector<PostProcessingOptionInfo> options;
};


//Chainable builder returned by addPostProcessingTarget(), mirroring the
//LocalChannel metadata-setter pattern. It is a lightweight cursor over the
//PostProcessingTargetInfo stored on the device's PostProcessingManager, so option
//hints declared here appear in getPostProcessingTargets(). Registration (and thus
//these mutations) is expected during device construction, before the device is
//served, so no locking is performed here.
class PostProcessingTargetBuilder
{
public:

    //A default-constructed builder is inert (its setters are no-ops); used as the
    //return value when there is no backing target to describe.
    PostProcessingTargetBuilder() : info(nullptr) {}
    explicit PostProcessingTargetBuilder(PostProcessingTargetInfo& targetInfo) : info(&targetInfo) {}

    //Declare a supported option (name and optional human-readable description).
    PostProcessingTargetBuilder& addOption(const std::string& name, const std::string& description = "")
    {
        if (info != nullptr) {
            info->options.push_back(PostProcessingOptionInfo{name, description});
        }
        return *this;
    }

private:

    PostProcessingTargetInfo* info;   //non-owning; points into the manager's target store
};


//Abstract interface following the ChannelManager/AttributeManager Local/Remote
//split template. LocalPostProcessingManager backs it in stidevice;
//RemotePostProcessingManager backs it over CORBA. Target registration is a local
//concern (a callback cannot be registered on a remote device), so it lives on
//LocalPostProcessingManager / LocalDevice rather than on this cross-device seam.
class PostProcessingManager
{
public:

    virtual ~PostProcessingManager() = default;

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
