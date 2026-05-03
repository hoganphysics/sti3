#ifndef STI_ENGINE_LOCALRESULTSCOLLECTOR_H
#define STI_ENGINE_LOCALRESULTSCOLLECTOR_H

#include <sti/engine/ResultsCollector.h>
#include <sti/engine/ShotRepository.h>
#include <sti/utils/FileHolderFactory.h>

#include <memory>
#include <string>
#include <mutex>
#include <map>

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

    bool addMeasurements(const STI::Device::DeviceID& deviceID, const MeasurementVector& measurements, const std::shared_ptr<STI::Utils::FileServer>& sourceFileServer);
    bool addAttributes(const STI::Device::DeviceID& deviceID, const std::map<std::string, std::string>& attributes);
    bool addVersionInfo(const STI::Device::DeviceID& deviceID, const std::vector<STI::Device::VersionInfo>& versions);
    bool addMessages(const std::vector<EnginePlayingMessage>& messages);

    std::shared_ptr<MeasurementMap> getMeasurements();

    void setRecord(const ShotResultRecord& shotRecord);
    std::shared_ptr<ShotResult> getResults() const;

    std::shared_ptr<ParseResult> getParseResults() const;

private:

    bool transferValue(STI::Utils::MixedValue& data, std::map<STI::Utils::FileID, STI::Utils::FileID>& cachedFileIDs, const std::shared_ptr<STI::Utils::FileServer>& sourceFileServer);

    std::shared_ptr<STI::Utils::FileHolder> makeLocalFileHandle(const std::string& basePath, const STI::Utils::FileID& remoteFileID);

    std::shared_ptr<STI::Utils::FileHolderFactory> fileHolderFactory;
    ResultsPaths resultsPaths;

    std::shared_ptr<FullShotResult> fullShotResult;
   
    mutable std::mutex collectorMutex;
};


} //Engine
} //STI

#endif
