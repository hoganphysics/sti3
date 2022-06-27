
#include "NetworkShotWrapper.h"
#include "ORBManager.h"

using STI::Network::NetworkShotWrapper;


NetworkShotWrapper::NetworkShotWrapper(const std::shared_ptr<STI::Engine::Shot>& shot)
: localshot(shot), shotEventsCBServant(shot)
{
    STI::Network::ORBManager::ORBManager::activateServant(shotEventsCBServant);

    if (localshot != 0) {
        shotConfig = localshot->getShotConfig();
    }
}

NetworkShotWrapper::~NetworkShotWrapper()
{
}

const STI::Engine::ShotConfig& NetworkShotWrapper::getShotConfig() const
{
    return shotConfig;
}

void NetworkShotWrapper::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts)
{
    if (localshot != 0) {
        localshot->getEvents(evts);
    }
}


bool NetworkShotWrapper::getTShotReference(STI::TNetwork::TShotEventsCallback_ptr& tShotCallback)
{
    tShotCallback = shotEventsCBServant._this();
    return !CORBA::is_nil(tShotCallback);
}

