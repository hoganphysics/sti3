#ifndef STI_NETWORK_REMOTEPOSTPROCESSINGMANAGER_H
#define STI_NETWORK_REMOTEPOSTPROCESSINGMANAGER_H

#include "generated/deviceNet.h"
#include <sti/device/PostProcessingManager.h>
#include "TReferenceHolder.h"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace STI
{
namespace Network
{

//Client-side proxy: forwards requestPostProcessing()/getPostProcessingTargets()
//to a remote device's TPostProcessingManager over CORBA. Target registration is
//local-only (a callback cannot be registered remotely), so it is not part of this
//proxy; completion results arrive as broadcast PostProcessingCompleteMessages.
class RemotePostProcessingManager : public STI::Device::PostProcessingManager,
                                    public STI::TNetwork::TReferenceHolder<STI::TNetwork::TPostProcessingManager>	//mixin
{
public:

    RemotePostProcessingManager(::STI::TNetwork::TPostProcessingManager_var manager, const std::string& originID);
    ~RemotePostProcessingManager();

    std::vector<STI::Device::PostProcessingTargetInfo> getPostProcessingTargets() const override;

    bool requestPostProcessing(const std::string& name,
                               const STI::Engine::ShotID& shotID,
                               const STI::Device::DeviceID& shotOwnerID,
                               const STI::Utils::MetaData& options) override;

    void stop() override;

    bool ping() const;

private:

    mutable std::mutex postProcessingMutex;
};


} //Network
} //STI

#endif
