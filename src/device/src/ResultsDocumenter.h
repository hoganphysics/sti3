#ifndef STI_ENGINE_RESULTSDOCUMENTER_H
#define STI_ENGINE_RESULTSDOCUMENTER_H

#include <string>
#include <memory>


namespace STI
{
namespace Engine
{

class ShotID;
class LocalResultsCollector;
class ShotResult;


class ResultsDocumenter
{
public:

    virtual ResultsPaths preparePaths(const ShotID& sid) = 0;
    virtual bool save(const ResultsPaths& paths, const std::shared_ptr<LocalResultsCollector>& resultsCollector) = 0;
    virtual bool load(const ShotID& sid, std::shared_ptr<ShotResult>& shotResult) = 0;
};


} //Engine
} //STI

#endif
