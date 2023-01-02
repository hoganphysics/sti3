#ifndef STI_ENGINE_NETWORKRESULTSCOLLECTORFACTORY_H
#define STI_ENGINE_NETWORKRESULTSCOLLECTORFACTORY_H

#include "ResultsCollectorFactory.h"
#include "NetworkResultsCollector.h"

#include <memory>


namespace STI
{
namespace Network
{

class NetworkResultsCollectorFactory : public STI::Engine::ResultsCollectorFactory
{
public:

    NetworkResultsCollectorFactory() {}
    ~NetworkResultsCollectorFactory() {}

    std::shared_ptr<STI::Engine::LocalResultsCollector> createResultsCollector(
            const STI::Engine::ShotID& sid, 
            const STI::Engine::ResultsPaths& paths,
            const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
    {
        auto collector = std::make_shared<NetworkResultsCollector>(sid, paths, factory);
        return collector;
    }

};


} //Network
} //STI

#endif
