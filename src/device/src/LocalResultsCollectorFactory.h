#ifndef STI_ENGINE_LOCALRESULTSCOLLECTORFACTORY_H
#define STI_ENGINE_LOCALRESULTSCOLLECTORFACTORY_H


#include "ResultsCollectorFactory.h"

#include <memory>


namespace STI
{
namespace Engine
{

class LocalResultsCollectorFactory : public ResultsCollectorFactory
{
public:

    LocalResultsCollectorFactory() {}
    ~LocalResultsCollectorFactory() {}

    std::shared_ptr<LocalResultsCollector> createResultsCollector(
            const ShotID& sid, 
            const ResultsPaths& paths,
            const std::shared_ptr<STI::Utils::FileHolderFactory>& factory)
    {
        auto collector = std::make_shared<LocalResultsCollector>(sid, paths, factory);
        return collector;
    }

};


} //Engine
} //STI

#endif
