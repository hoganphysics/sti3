#ifndef STI_ENGINE_SERIALIZEDREPOSITORY_H
#define STI_ENGINE_SERIALIZEDREPOSITORY_H

#include <sti/engine/ShotRepository.h>

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

    bool saveSequenceParseResult(const SequenceEntryID& id, const std::shared_ptr<ParseResult>& parseResult, const EngineJobStatus& parseStatus);
    bool updateSequence(const SequenceEntryID& id, const ShotID& shotID, const EngineJobStatus& shotStatus);
    bool saveSequence(const SequenceID& seqid, const std::shared_ptr<SequenceResult>& sequenceResult);

    bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementMap>& measurements);

    //ResultsDocumenter
    std::string prepareLogPath(const STI::Utils::TimeStamp& timeStamp, bool autocreate);
    ResultsPaths preparePaths(const ParseID& pid);
    ResultsPaths preparePaths(const ShotID& sid);
    ResultsPaths preparePaths(const SequenceID& seqid);

private:

    ResultsPaths preparePaths(const STI::Utils::TimeStamp& timeStamp);

    std::string makeParseFilename(const ParseID& pid);
    std::string makeShotFilename(const ShotID& sid);
    std::string makeSequenceFilename(const SequenceID& seqid);

    ResultsPaths makePaths(const STI::Utils::TimeStamp& timeStamp);

    void makePathIfNew(const std::string& pathName);

    std::string getShotBasePath(const STI::Utils::TimeStamp& timeStamp);
    std::string getLogBasePath(const STI::Utils::TimeStamp& timeStamp);

    std::string baseDevicePath;

    STI::Utils::OrderedBufferMap<STI::Utils::TimeStamp, ResultsPaths> cachedPaths;

    mutable std::mutex pathMutex;
};


} //Engine
} //STI

#endif




