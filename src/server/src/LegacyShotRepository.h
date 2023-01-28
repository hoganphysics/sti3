#ifndef STI_ENGINE_LEGACYSHOTREPOSITORY_H
#define STI_ENGINE_LEGACYSHOTREPOSITORY_H

#include <sti/engine/ShotRepository.h>

#include <sti/device/DeviceID.h>
#include <sti/engine/ShotID.h>

#include "utils/OrderedBufferMap.h"

#include <memory>
#include <string>


namespace STI
{
namespace Engine
{

class LegacySequenceXMLBuilder;

class LegacyShotRepository : public ShotRepository
{
public:

    LegacyShotRepository(const std::string& baseDevicePath);
    ~LegacyShotRepository();

    ResultsPaths preparePaths(const ShotID& sid);
    ResultsPaths preparePaths(const SequenceID& seqid);

    bool findParseResult(const ParseID& pid);
    bool findShotResult(const ShotID& sid);
    bool findSequenceResult(const SequenceID& seqid);

    bool getParseResult(const ParseID& id, std::shared_ptr<ParseResult>& shotResult);
    bool getShotResult(const ShotID& id, std::shared_ptr<ShotResult>& shotResult);
    bool getSequenceResult(const SequenceID& id, std::shared_ptr<SequenceResult>& sequenceResult);

    bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementMap>& measurements);

    bool saveShot(const ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult);

    bool updateSequence(const SequenceEntryID& id, const ShotID& shotID, const EngineJobStatus& shotStatus);
    bool saveSequence(const SequenceID& seqid, const std::shared_ptr<SequenceResult>& sequenceResult);

private:
    
    ResultsPaths preparePaths(const TimeStamp& timeStamp);

    std::string makeParseFilename(const ParseID& pid);
    std::string makeShotFilename(const ShotID& sid);
    std::string makeSequenceFilename(const SequenceID& seqid);

    void makePathIfNew(const std::string& pathName);
    
    ResultsPaths makePaths(const TimeStamp& timeStamp);
    std::string getShotBasePath(const TimeStamp& timeStamp);

    STI::Utils::OrderedBufferMap<TimeStamp, ResultsPaths> cachedPaths;
    STI::Utils::OrderedBufferMap<SequenceID, std::shared_ptr<LegacySequenceXMLBuilder>> cachedSequences;
    
    std::string baseDevicePath;

    mutable std::mutex pathMutex;
};


} //Engine
} //STI

#endif

