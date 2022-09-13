#ifndef STI_ENGINE_TRANSIENTREPOSITORY_H
#define STI_ENGINE_TRANSIENTREPOSITORY_H

#include "ShotRepository.h"

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

    bool findShotResult(const ShotID& sid);
    bool findParseResult(const ParseID& pid);

    bool getShotResult(const ShotID& id, std::shared_ptr<ShotResult>& shotResult);
    bool getParseResult(const ParseID& id, std::shared_ptr<ParseResult>& parseResult);
    
    bool saveShot(const ShotID& sid, const std::shared_ptr<FullShotResult>& fullShotResult);

    bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements);

private:

    std::string tempResultsPath;

    STI::Utils::OrderedBufferMap<STI::Engine::ParseID, std::shared_ptr<STI::Engine::ParseResult>> parseBuffer;
    STI::Utils::OrderedBufferMap<STI::Engine::ShotID, std::shared_ptr<STI::Engine::FullShotResult>> resultBuffer;

};


} //Engine
} //STI

#endif








