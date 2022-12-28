
#ifndef STI_ENGINE_SERIALIZEDREPOSITORY_H
#define STI_ENGINE_SERIALIZEDREPOSITORY_H

#include "ShotRepository.h"

#include <sti/device/DeviceID.h>
#include <sti/engine/ShotID.h>

#include "utils/OrderedBufferMap.h"

#include <memory>
#include <string>
#include <mutex>


namespace STI
{
namespace Engine
{

class ShotResult;

class SerializedRepository : public ShotRepository
                           //  public ResultsDocumenter
{
public:

    SerializedRepository(const std::string& baseDevicePath);


    //ShotRepositroy
    bool findParseResult(const ParseID& pid);
    bool findShotResult(const ShotID& sid);
    bool findSequenceResult(const SequenceID& seqid);

    bool getShotResult(const ShotID& id, std::shared_ptr<ShotResult>& shotResult);
    bool getParseResult(const ParseID& id, std::shared_ptr<ParseResult>& shotResult);
    bool getSequenceResult(const SequenceID& id, std::shared_ptr<SequenceResult>& sequenceResult);

    bool saveShot(const ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult);

    bool updateSequence(const SequenceEntryID& id, const ShotID& shotID, const EngineJobStatus& shotStatus);
    bool saveSequence(const SequenceID& seqid, const std::shared_ptr<SequenceResult>& sequenceResult);

    bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements);
    // bool getParseTicket(const ShotID& sid, std::shared_ptr<ParseTicket>& parseTicket); 

    //ResultsDocumenter
    ResultsPaths preparePaths(const ShotID& sid);
    ResultsPaths preparePaths(const SequenceID& seqid);
    // bool save(const ResultsPaths& paths, const std::shared_ptr<LocalResultsCollector>& resultsCollector);

    // bool load(const ShotID& sid, std::shared_ptr<ShotResult>& shotResult);

private:

    ResultsPaths preparePaths(const TimeStamp& timeStamp);

    std::string makeParseFilename(const ParseID& pid);
    std::string makeShotFilename(const ShotID& sid);
    std::string makeSequenceFilename(const SequenceID& seqid);

    // ResultsPaths makePaths(const ShotID& sid);
    // ResultsPaths makePaths(const SequenceID& seqid);

    ResultsPaths makePaths(const TimeStamp& sid);
    // std::string makeBaseDevicePath();

    void makePathIfNew(const std::string& pathName);

    std::string getShotBasePath(const TimeStamp& sid);

    // STI::Device::DeviceID deviceID;
    std::string rootPath;
    std::string baseDevicePath;

    std::string archiveFilename;

    STI::Utils::OrderedBufferMap<TimeStamp, ResultsPaths> cachedPaths;
    // STI::Utils::OrderedBufferMap<SequenceID, ResultsPaths> cachedSequencePaths;

    mutable std::mutex pathMutex;

};


} //Engine
} //STI

#endif





