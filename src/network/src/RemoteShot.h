#ifndef STI_NETWORK_REMOTESHOT_H
#define STI_NETWORK_REMOTESHOT_H

#include "deviceNet.h"

#include <sti/engine/ShotConfig.h>
#include "Shot.h"
#include "TReferenceHolder.h"
#include "TShotRefInterface.h"

#include <memory>
#include <vector>


namespace STI
{
namespace Network
{

class RemoteShot : public STI::Engine::Shot,
                   public STI::TNetwork::TReferenceHolder<STI::TNetwork::TShotEventsCallback>,	//mixin
                   public STI::Network::TShotRefInterface	//mixin
{
public:

	RemoteShot(const STI::Engine::ShotConfig& shotConfig, ::STI::TNetwork::TShotEventsCallback_ptr shotCallback);
    ~RemoteShot();

    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& events);

    const STI::Engine::ShotConfig& getShotConfig() const;


private:

    bool getTShotReference(STI::TNetwork::TShotEventsCallback_ptr& tShotCallback);

    void _refreshEvents();
    bool refreshRequired;

    std::shared_ptr<std::vector<STI::Engine::RawEvent>> storedEvents;
    STI::Engine::ShotConfig shotConfig;

    //::STI::TNetwork::TShot_var _tShot;    //remote reference
    mutable std::mutex shotMutex;
};


} //Network
} //STI


#endif

