#ifndef STI_ENGINE_SHOTREPOSITORY_H
#define STI_ENGINE_SHOTREPOSITORY_H

#include <sti/fwd/Measurement_fwd.h>
#include <sti/engine/EngineJobStatus.h>

#include <memory>
#include <string>


namespace STI
{
namespace Engine
{

class ShotID;
class ParseTicket;
class ShotResult;
class ParseResult;
class ResultsCollector;
class FullShotResult;
class ParseID;
class SequenceID;
class SequenceEntryID;
class SequenceResult;
// enum class EngineJobStatus;


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

class ShotRepository
{
public:

    virtual ~ShotRepository() {}

    virtual ResultsPaths preparePaths(const ShotID& sid) = 0;
    virtual ResultsPaths preparePaths(const SequenceID& seqid) = 0;

    virtual bool findParseResult(const ParseID& pid) = 0;
    virtual bool findShotResult(const ShotID& sid) = 0;
    virtual bool findSequenceResult(const SequenceID& seqid) = 0;

    virtual bool getParseResult(const ParseID& id, std::shared_ptr<ParseResult>& shotResult) = 0;
    virtual bool getShotResult(const ShotID& id, std::shared_ptr<ShotResult>& shotResult) = 0;
    virtual bool getSequenceResult(const SequenceID& id, std::shared_ptr<SequenceResult>& sequenceResult) = 0;

    virtual bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements) = 0;

    virtual bool saveShot(const ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult) = 0;

    virtual bool updateSequence(const SequenceEntryID& id, const ShotID& shotID, const EngineJobStatus& shotStatus) = 0;
    virtual bool saveSequence(const SequenceID& seqid, const std::shared_ptr<SequenceResult>& sequenceResult) = 0;

};


} //Engine
} //STI

#endif

