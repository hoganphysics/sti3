

#include "JPersistenceManager.h"
#include "ShotResult.h"
#include <sti/engine/ShotID.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>

#include <memory>

using STI::Device::JPersistenceManager;


JPersistenceManager::JPersistenceManager(std::shared_ptr<STI::Device::PersistenceManager>& manager)
: persistenceManager(manager)
{   
}

JPersistenceManager::~JPersistenceManager()
{  
}


std::shared_ptr<STI::Engine::ShotResult> JPersistenceManager::getShot(const STI::Engine::ShotID& sid)
{
    auto shotResult = std::make_shared<STI::Engine::ShotResult>();

    if (persistenceManager != 0) {
        persistenceManager->getShot(sid, shotResult);
    }

    return shotResult;
}

std::shared_ptr<STI::Engine::MeasurementVector> JPersistenceManager::getMeasurements(const STI::Engine::ShotID& sid)
{
    auto measurements = std::make_shared<STI::Engine::MeasurementVector>();

    if (persistenceManager != 0) {
        persistenceManager->getMeasurements(sid, measurements);
    }

    return measurements;
}

