
#ifndef STI_ENGINE_NETWORKRESULTSCOLLECTOR_H
#define STI_ENGINE_NETWORKRESULTSCOLLECTOR_H

#include "LocalResultsCollector.h"
#include "TResultsCollector_i.h"
#include "deviceNet.h"

#include <memory>


namespace STI
{
namespace Network
{

class NetworkResultsCollector : public STI::Engine::LocalResultsCollector
{
public:

    NetworkResultsCollector(const STI::Engine::ShotID& sid, 
            const std::shared_ptr<STI::Engine::EventEngine>& eventEngine, 
            const std::shared_ptr<STI::Engine::ParsedDependencyTree>& dependencies,
            const STI::Engine::ResultsPaths& paths,
            const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
		: STI::Engine::LocalResultsCollector(sid, eventEngine, dependencies, paths, factory), resultsCollectorServant(this) {}

	~NetworkResultsCollector() {}


    static bool getTResultsCollector(
        const typename std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector, 
        STI::TNetwork::TResultsCollector_var& tResultsCollector)
    {
        if (resultsCollector == 0) {
            return false;
        }
        
        auto wrapper = std::dynamic_pointer_cast<NetworkResultsCollector>(resultsCollector);
        if (wrapper) {
            tResultsCollector = wrapper->resultsCollectorServant._this();
            return !CORBA::is_nil(tResultsCollector);
        }
        return false;
    }

private:

    STI::TNetwork::TResultsCollector_i resultsCollectorServant;
};



} //Network
} //STI

#endif
