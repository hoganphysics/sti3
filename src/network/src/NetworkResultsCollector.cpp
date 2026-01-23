
#include "NetworkResultsCollector.h"
#include "ORBManager.h"

using STI::Network::NetworkResultsCollector;


NetworkResultsCollector::NetworkResultsCollector(const STI::Engine::ShotID& sid, 
            const STI::Engine::ResultsPaths& paths,
            const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
: STI::Engine::LocalResultsCollector(sid, paths, factory), resultsCollectorServantHolder(new STI::TNetwork::TResultsCollector_i(this))
{
}

NetworkResultsCollector::~NetworkResultsCollector()
{
}

bool NetworkResultsCollector::getTResultsCollector(
    const typename std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector, 
    STI::TNetwork::TResultsCollector_var& tResultsCollector)
{
    if (resultsCollector == 0) {
        return false;
    }
    
    auto wrapper = std::dynamic_pointer_cast<NetworkResultsCollector>(resultsCollector);
    if (wrapper) {
        tResultsCollector = wrapper->resultsCollectorServantHolder.getRefVar();
        return !CORBA::is_nil(tResultsCollector);
    }
    return false;
}
