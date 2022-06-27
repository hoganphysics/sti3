#ifndef STI_ENGINE_NETWORKSHOTWRAPPER_H
#define STI_ENGINE_NETWORKSHOTWRAPPER_H

#include "Shot.h"
#include "TShotRefInterface.h"

#include "TShotEventsCallback_i.h"
#include "deviceNet.h"
#include <sti/engine/ShotConfig.h>

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

    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts);

private:

    bool getTShotReference(STI::TNetwork::TShotEventsCallback_ptr& tShotCallback);

    STI::Engine::ShotConfig shotConfig;

    std::shared_ptr<STI::Engine::Shot> localshot;
    STI::TNetwork::TShotEventsCallback_i shotEventsCBServant;

};


} //Network
} //STI

#endif
