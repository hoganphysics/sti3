#ifndef STI_ENGINE_TRANSIENTREPOSITORY_H
#define STI_ENGINE_TRANSIENTREPOSITORY_H

#include <sti/engine/ShotRepository.h>

#include <sti/engine/ShotID.h>
#include "utils/OrderedBufferMap.h"

#include <memory>
#include <string>

namespace STI
{
namespace Engine
{

class TransientRepository : public ShotRepository
{
public:

    TransientRepository(const std::string& tempBasePath);
    ~TransientRepository();

    ResultsPaths preparePaths(const ShotID& sid);
    ResultsPaths preparePaths(const SequenceID& seqid);

    bool findParseResult(const ParseID& pid);
    bool findShotResult(const ShotID& sid);
    bool findSequenceResult(const SequenceID& seqid);   

    bool getParseResult(const ParseID& id, std::shared_ptr<ParseResult>& parseResult);
    bool getShotResult(const ShotID& id, std::shared_ptr<ShotResult>& shotResult);    
    bool getSequenceResult(const SequenceID& id, std::shared_ptr<SequenceResult>& sequenceResult);

    bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementMap>& measurements);

    bool saveShot(const ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult);

    bool updateSequence(const SequenceEntryID& id, const ShotID& shotID, const EngineJobStatus& shotStatus);
    bool saveSequence(const SequenceID& seqid, const std::shared_ptr<SequenceResult>& sequenceResult);

private:

    ResultsPaths preparePaths();

    std::string tempResultsPath;

    STI::Utils::OrderedBufferMap<STI::Engine::ParseID, std::shared_ptr<STI::Engine::ParseResult>> parseBuffer;
    STI::Utils::OrderedBufferMap<STI::Engine::ShotID, std::shared_ptr<STI::Engine::FullShotResult>> resultBuffer;
    STI::Utils::OrderedBufferMap<STI::Engine::SequenceID, std::shared_ptr<STI::Engine::SequenceResult>> sequenceBuffer;

};


} //Engine
} //STI

#endif








