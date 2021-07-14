#ifndef STI_ENGINE_LOCALRESULTSCOLLECTOR_H
#define STI_ENGINE_LOCALRESULTSCOLLECTOR_H

#include "ResultsCollector.h"
#include "ResultsDocumenter.h"
#include "FileHolderFactory.h"

#include <memory>
#include <string>

namespace STI
{
namespace Engine
{

class EventEngine;
class ParsedDependencyTree;
struct ResultsPaths;


class LocalResultsCollector : public ResultsCollector
{
public:

    LocalResultsCollector(const STI::Engine::ShotID& sid, 
            const std::shared_ptr<STI::Engine::EventEngine>& eventEngine, 
            const std::shared_ptr<ParsedDependencyTree>& dependencies,
            const ResultsPaths& paths,
            const std::shared_ptr<STI::Utils::FileHolderFactory>& factory);
    virtual ~LocalResultsCollector() {}

    ShotID getShotID();
    std::shared_ptr<ParsedDependencyTree> getDependencies();

    void addEvents(const DeviceEventMap& parsedEvents);
    void addTimingFiles(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& files);
    bool addMeasurements(const std::shared_ptr<MeasurementVector>& measurements);
    bool addAttributes(const STI::Device::DeviceID& deviceID, const std::vector<std::shared_ptr<STI::Device::Attribute>>& attributes);

    std::shared_ptr<MeasurementVector> getMeasurements();

private:

    virtual std::string makeLocalPath(const std::string& basePath, const std::string& remoteFilename);
    std::string makeUniquePath(const std::string& filename);

    ShotID sid;
    std::shared_ptr<STI::Engine::EventEngine> eventEngine;    
    std::shared_ptr<ParsedDependencyTree> dependencies;
    // std::shared_ptr<STI::Engine::ResultTicket> resultTicket;

    ResultsPaths resultsPaths;
    std::shared_ptr<STI::Utils::FileHolderFactory> fileHolderFactory;

    std::shared_ptr<MeasurementVector> measurements_;
    DeviceEventMap parsedEvents_;

};


} //Engine
} //STI

#endif
