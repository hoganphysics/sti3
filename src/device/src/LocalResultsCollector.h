#ifndef STI_ENGINE_LOCALRESULTSCOLLECTOR_H
#define STI_ENGINE_LOCALRESULTSCOLLECTOR_H

#include "ResultsCollector.h"
#include "ShotRepository.h"
#include "FileHolderFactory.h"

#include <memory>
#include <string>
#include <mutex>


namespace STI
{
namespace Engine
{

class EventEngine;
//class ParsedDependencyTree;
struct ResultsPaths;
class ShotResult;
class ShotResultRecord;


class LocalResultsCollector : public ResultsCollector
{
public:

    LocalResultsCollector(const STI::Engine::ShotID& sid, 
           // const std::shared_ptr<STI::Engine::EventEngine>& eventEngine, 
           // const std::shared_ptr<ParsedDependencyTree>& dependencies,
            const ResultsPaths& paths,
            const std::shared_ptr<STI::Utils::FileHolderFactory>& factory);
    virtual ~LocalResultsCollector() {}

    ShotID getShotID() const;
    //std::shared_ptr<ParsedDependencyTree> getDependencies();
    

    void addEvents(const DeviceEventMap& parsedEvents);
    void addTimingFiles(const std::vector<std::shared_ptr<STI::Utils::FileHolder>>& files);
    bool addMeasurements(const std::shared_ptr<MeasurementVector>& measurements);
    bool addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes);

    std::shared_ptr<MeasurementVector> getMeasurements();

    void setRecord(const ShotResultRecord& shotRecord);
    std::shared_ptr<ShotResult> getResults() const;

private:

    virtual std::string makeLocalPath(const std::string& basePath, const std::string& remoteFilename);
    //std::string makeUniquePath(const std::string& filename);

    //std::shared_ptr<ParsedDependencyTree> dependencies;
    //std::shared_ptr<STI::Engine::EventEngine> eventEngine;   
    std::shared_ptr<STI::Utils::FileHolderFactory> fileHolderFactory;
    ResultsPaths resultsPaths;

    std::shared_ptr<ShotResult> shotResult;

//    ShotID sid;
     
//    std::shared_ptr<MeasurementVector> measurements_;
//    DeviceEventMap parsedEvents_;

    // std::shared_ptr<STI::Engine::ResultTicket> resultTicket;
    
    mutable std::mutex collectorMutex;

};


} //Engine
} //STI

#endif
