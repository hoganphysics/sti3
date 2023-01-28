#ifndef STI_ENGINE_LOCALRESULTSCOLLECTOR_H
#define STI_ENGINE_LOCALRESULTSCOLLECTOR_H

#include <sti/engine/ResultsCollector.h>
#include <sti/engine/ShotRepository.h>
#include <sti/utils/FileHolderFactory.h>

#include <memory>
#include <string>
#include <mutex>


namespace STI
{
namespace Engine
{

class EventEngine;
struct ResultsPaths;
class ShotResult;
class ShotResultRecord;
class ParseResult;
class FullShotResult;


class LocalResultsCollector : public ResultsCollector
{
public:

    LocalResultsCollector(const STI::Engine::ShotID& sid, 
            const ResultsPaths& paths,
            const std::shared_ptr<STI::Utils::FileHolderFactory>& factory);
    virtual ~LocalResultsCollector() {}

    ShotID getShotID() const;

    bool addMeasurements(const STI::Device::DeviceID& deviceID, const MeasurementVector& measurements);
    bool addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes);

    std::shared_ptr<MeasurementMap> getMeasurements();

    void setRecord(const ShotResultRecord& shotRecord);
    std::shared_ptr<ShotResult> getResults() const;

    std::shared_ptr<ParseResult> getParseResults() const;

private:

    virtual std::string makeLocalPath(const std::string& basePath, const std::string& remoteFilename);

    std::shared_ptr<STI::Utils::FileHolderFactory> fileHolderFactory;
    ResultsPaths resultsPaths;

    std::shared_ptr<FullShotResult> fullShotResult;
   
    mutable std::mutex collectorMutex;
};


} //Engine
} //STI

#endif
