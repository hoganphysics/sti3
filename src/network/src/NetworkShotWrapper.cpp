#include "NetworkShotWrapper.h"
#include "ORBManager.h"
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/ParsedVar.h>
#include <sti/engine/ParsedTag.h>
#include <sti/utils/FileHolder.h>

using STI::Network::NetworkShotWrapper;


NetworkShotWrapper::NetworkShotWrapper(const std::shared_ptr<STI::Engine::Shot>& shot)
: localshot(shot), shotEventsCBServantHolder(new STI::TNetwork::TShotCallback_i(shot))
{
    // STI::Network::ORBManager::ORBManager::activateServant(shotEventsCBServant);

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

void NetworkShotWrapper::getRootEventGroup(std::shared_ptr<STI::Engine::RawEventGroup>& rootGroup)
{
    if (localshot != 0) {
        localshot->getRootEventGroup(rootGroup);
    }
}

bool NetworkShotWrapper::getTShotReference(STI::TNetwork::TShotCallback_var& tShotCallback)
{
    tShotCallback = shotEventsCBServantHolder.getRefVar();
    return !CORBA::is_nil(tShotCallback);
}
