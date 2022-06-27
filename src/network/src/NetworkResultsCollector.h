
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
            const STI::Engine::ResultsPaths& paths,
            const std::shared_ptr<STI::Utils::FileHolderFactory>& factory);

	~NetworkResultsCollector();


    static bool getTResultsCollector(
        const typename std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector, 
        STI::TNetwork::TResultsCollector_var& tResultsCollector);

private:

    STI::TNetwork::TResultsCollector_i resultsCollectorServant;
};



} //Network
} //STI

#endif
