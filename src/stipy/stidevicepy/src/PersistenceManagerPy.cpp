
#include "PersistenceManagerPy.h"

#include <sti/engine/ParseResult.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/RawEvent.h>


using STI::Python::PersistenceManagerPy;


PersistenceManagerPy::PersistenceManagerPy(const std::shared_ptr<STI::Device::PersistenceManager>& manager)
: persistenceManager(manager)
{   
}

PersistenceManagerPy::~PersistenceManagerPy()
{
}

std::shared_ptr<STI::Engine::ParseResult> PersistenceManagerPy::getParseResult(const STI::Engine::ParseID& pid)
{
    std::shared_ptr<STI::Engine::ParseResult> parseResult;

    if (persistenceManager != 0 && persistenceManager->getParseResult(pid, parseResult)) {
        return parseResult;
    }

    //not found
    parseResult = std::make_shared<STI::Engine::ParseResult>();
    return parseResult;
}

std::shared_ptr<STI::Engine::ShotResult> PersistenceManagerPy::getShotResult(const STI::Engine::ShotID& sid)
{
    std::shared_ptr<STI::Engine::ShotResult> shotResult;

    if (persistenceManager != 0 && persistenceManager->getShotResult(sid, shotResult)) {
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

