#ifndef STI_ENGINE_NETWORKSHOTWRAPPER_H
#define STI_ENGINE_NETWORKSHOTWRAPPER_H

#include <sti/engine/Shot.h>
#include <sti/engine/ShotConfig.h>

#include "TShotRefInterface.h"
#include "TShotCallback_i.h"
#include "generated/deviceNet.h"
#include "ServantHolder.h"

#include <vector>
#include <memory>


namespace STI
{
namespace Network
{

class NetworkShotWrapper : public STI::Engine::Shot,
                           public STI::Network::TShotRefInterface	//mixin
{
public:

    NetworkShotWrapper(const std::shared_ptr<STI::Engine::Shot>& shot);
    ~NetworkShotWrapper();

    const STI::Engine::ShotConfig& getShotConfig() const;
    void getRootEventGroup(std::shared_ptr<STI::Engine::RawEventGroup>& rootGroup);

private:

    bool getTShotReference(STI::TNetwork::TShotCallback_var& tShotCallback);

    STI::Engine::ShotConfig shotConfig;

    std::shared_ptr<STI::Engine::Shot> localshot;
    // STI::TNetwork::TShotCallback_i shotEventsCBServant;
    ServantHolder<STI::TNetwork::TShotCallback_i, STI::TNetwork::TShotCallback> shotEventsCBServantHolder;
};


} //Network
} //STI

#endif
