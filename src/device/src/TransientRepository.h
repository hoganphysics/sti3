
#ifndef STI_ENGINE_TRANSIENTREPOSITORY_H
#define STI_ENGINE_TRANSIENTREPOSITORY_H

#include "ShotRepository.h"
#include "ShotID.h"
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

    bool findShot(const ShotID& sid);
    bool getShot(const ShotID& id, std::shared_ptr<ShotResult>& shotResult);
    bool saveShot(const ShotID& sid, const std::shared_ptr<ShotResult>& shotResult);

    ResultsPaths preparePaths(const ShotID& sid);

    bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements);

private:

    std::string tempResultsPath;
    STI::Utils::OrderedBufferMap<STI::Engine::ShotID, std::shared_ptr<STI::Engine::ShotResult>> resultBuffer;

};


} //Engine
} //STI

#endif








