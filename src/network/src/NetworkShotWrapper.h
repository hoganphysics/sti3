#ifndef STI_ENGINE_NETWORKSHOTWRAPPER_H
#define STI_ENGINE_NETWORKSHOTWRAPPER_H

#include "Shot.h"
#include "TShotRefInterface.h"

#include "TShotEventsCallback_i.h"
#include "deviceNet.h"

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

    NetworkShotWrapper(const std::shared_ptr<STI::Engine::Shot>& shot)
    : localshot(shot), parsedShotServant(shot)
    {
        if (localshot != 0) {
            shotConfig = localshot->getShotConfig();
        }
    }

    ~NetworkShotWrapper()
    {
    }

    STI::Engine::ShotConfig& getShotConfig()
    {
        return shotConfig;
    }

    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts)
    {
        if (localshot != 0) {
            localshot->getEvents(evts);
        }
    }

private:

    bool getTShotReference(STI::TNetwork::TShotEventsCallback_ptr& tShotCallback)
    {
        tShotCallback = parsedShotServant._this();
        return !CORBA::is_nil(tShotCallback);
    }

    STI::Engine::ShotConfig shotConfig;

    std::shared_ptr<STI::Engine::Shot> localshot;
    STI::TNetwork::TShotEventsCallback_i parsedShotServant;

};


} //Network
} //STI

#endif
