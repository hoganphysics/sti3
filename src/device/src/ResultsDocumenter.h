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

//should this be shot-specific, or general?

struct ResultsPaths
{
    std::string tempPath;
    std::string basePath;
    std::string dataPath;
    std::string timingPath;
    std::string experimentPath;
    std::string sequencePath;
};

class ResultsDocumenter
{
public:

    virtual ResultsPaths preparePaths(const ShotID& sid) = 0;
    virtual bool save(const ResultsPaths& paths, const std::shared_ptr<LocalResultsCollector>& resultsCollector) = 0;

};


} //Engine
} //STI

#endif
