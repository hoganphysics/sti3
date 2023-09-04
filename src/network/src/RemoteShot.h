#ifndef STI_NETWORK_REMOTESHOT_H
#define STI_NETWORK_REMOTESHOT_H

#include "generated/deviceNet.h"
#include <sti/engine/Shot.h>
#include "TReferenceHolder.h"
#include "TShotRefInterface.h"

#include <sti/engine/ShotConfig.h>
#include <sti/utils/CachedValue.h>

#include <memory>
#include <vector>


namespace STI
{
namespace Network
{

class RemoteShot : public STI::Engine::Shot,
                   public STI::TNetwork::TReferenceHolder<STI::TNetwork::TShotCallback>,	//mixin
                   public STI::Network::TShotRefInterface	//mixin
{
public:

	RemoteShot(const STI::Engine::ShotConfig& shotConfig, 
                ::STI::TNetwork::TShotCallback_ptr shotCallback);
    ~RemoteShot();

    const STI::Engine::ShotConfig& getShotConfig() const;

    void getRootEventGroup(std::shared_ptr<STI::Engine::RawEventGroup>& rootGroup);

private:

    bool getTShotReference(STI::TNetwork::TShotCallback_ptr& tShotCallback);

    void refresh();
    bool refreshRequired;
    
    bool refreshEvents();

    STI::Engine::ShotConfig shotConfig;
    std::shared_ptr<STI::Engine::RawEventGroup> rootEventGroup;

    mutable std::mutex shotMutex;
};


} //Network
} //STI

#endif

