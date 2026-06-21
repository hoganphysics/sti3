#ifndef STI_TNETWORK_TPOSTPROCESSINGMANAGER_I_H
#define STI_TNETWORK_TPOSTPROCESSINGMANAGER_I_H

#include <sti/device/PostProcessingManager.h>
#include <sti/device/Device.h>

#include "generated/deviceNet.h"

#include <memory>

namespace STI
{
namespace TNetwork
{


class TPostProcessingManager_i : public POA_STI::TNetwork::TPostProcessingManager,
                                 public PortableServer::RefCountServantBase
{
public:

    TPostProcessingManager_i(const std::shared_ptr<STI::Device::Device>& device);
    ~TPostProcessingManager_i();

    ::CORBA::Boolean requestPostProcessing(const char* name, const ::STI::TNetwork::TShotID& shotID,
                                           const ::STI::TNetwork::TDeviceID& shotOwnerID,
                                           const ::STI::TNetwork::TMixedValue& options);
    void getTargets(::STI::TNetwork::TPostProcessingTargetInfoSeq_out targets);
    ::CORBA::Boolean ping();

private:

    std::shared_ptr<STI::Device::PostProcessingManager> postProcessingManager;
};


} //TNetwork
} //STI

#endif
