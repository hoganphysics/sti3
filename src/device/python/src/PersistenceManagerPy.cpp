
#include "PersistenceManagerPy.h"
#include "ShotResult.h"
#include "RawEvent.h"


using STI::Python::PersistenceManagerPy;


PersistenceManagerPy::PersistenceManagerPy(const std::shared_ptr<STI::Device::PersistenceManager>& manager)
: persistenceManager(manager)
{   
}

PersistenceManagerPy::~PersistenceManagerPy()
{
}

std::shared_ptr<STI::Engine::ShotResult> PersistenceManagerPy::getShot(const STI::Engine::ShotID& sid)
{
    std::shared_ptr<STI::Engine::ShotResult> shotResult;

    if (persistenceManager != 0 && persistenceManager->getShot(sid, shotResult)) {
        return shotResult;
    }

    //not found
    shotResult = std::make_shared<STI::Engine::ShotResult>();
    shotResult->shotResultRecord.recordStatus = STI::Engine::RecordStatus::MissingResults;
    return shotResult;
}

STI::Engine::MeasurementVector PersistenceManagerPy::getMeasurements(const STI::Engine::ShotID& sid)
{
    std::shared_ptr<STI::Engine::MeasurementVector> measurements;

    if (persistenceManager != 0 && persistenceManager->getMeasurements(sid, measurements) && measurements != 0) {
        return *measurements;
    }

    //not found
    STI::Engine::MeasurementVector missing;
    return missing;
}

