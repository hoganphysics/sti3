#ifndef STI_ENGINE_RESULTSCOLLECTORFACTORY_H
#define STI_ENGINE_RESULTSCOLLECTORFACTORY_H

#include <sti/utils/FileHolderFactory.h>

#include <memory>


namespace STI
{
namespace Engine
{

class LocalResultsCollector;
class ShotID;
struct ResultsPaths;


class ResultsCollectorFactory
{
public:

    virtual ~ResultsCollectorFactory() {}

    virtual std::shared_ptr<LocalResultsCollector> createResultsCollector(
            const ShotID& sid, 
            const ResultsPaths& paths,
            const std::shared_ptr<STI::Utils::FileHolderFactory>& factory) = 0;

};


} //Engine
} //STI

#endif
