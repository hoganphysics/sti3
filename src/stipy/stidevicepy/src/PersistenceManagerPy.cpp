#include <pybind11/pybind11.h>

#include "PersistenceManagerPy.h"

#include <sti/engine/ParseResult.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/SequenceResult.h>
#include <sti/engine/ShotResult.h>

using STI::Python::PersistenceManagerPy;

namespace py = pybind11;


PersistenceManagerPy::PersistenceManagerPy(const std::shared_ptr<STI::Device::PersistenceManager>& manager)
: persistenceManager(manager)
{   
}

PersistenceManagerPy::~PersistenceManagerPy()
{
}


// std::map<STI::Device::DeviceID, STI::Engine::MeasurementVector> PersistenceManagerPy::getMeasurements(const STI::Engine::ShotID& sid)
// {
//     std::shared_ptr<STI::Engine::MeasurementMap> measurements;
//     std::map<STI::Device::DeviceID, STI::Engine::MeasurementVector> pyMeasurements; //need to copy map to remove shared_ptr

//     if (persistenceManager != 0 && persistenceManager->getMeasurements(sid, measurements) && measurements != 0) {
//         for (auto& tuple : *measurements) {
//             if (tuple.second != 0) {
//                 auto& measVec = pyMeasurements[tuple.first];

//                 //deep copy is cheap because vector stores pointers
//                 measVec.insert(measVec.end(), tuple.second->begin(), tuple.second->end());  
//             }
//          }
        
//     }
//     return pyMeasurements;

//     // //not found
//     // STI::Engine::MeasurementMap missing;
//     // return missing;
// }


STI::Engine::MeasurementMap PersistenceManagerPy::getMeasurements(const STI::Engine::ShotID& sid)
{
    std::shared_ptr<STI::Engine::MeasurementMap> measurements;

    if (persistenceManager != 0 && persistenceManager->getMeasurements(sid, measurements) && measurements != 0) {
        return (*measurements);
    }

    STI::Engine::MeasurementMap missing;
    return missing;
}


bool PersistenceManagerPy::findShot(const STI::Engine::ShotID& sid)
{
    if (persistenceManager != 0) {
        return persistenceManager->findShot(sid);
    }
    return false;
}

std::shared_ptr<STI::Engine::ParseResult> PersistenceManagerPy::getParseResult(const STI::Engine::ParseID& pid)
{
    std::shared_ptr<STI::Engine::ParseResult> parseResult;

    if (persistenceManager != 0 && persistenceManager->getParseResult(pid, parseResult)) {
        return parseResult;
    }

    //not found

    // throw py::type_error("Not a Vector");
    throw py::value_error("ParseResult not found for ParseID: " + pid.print());
    // throw py::PyFileNotFoundError("Resource '" + resource_name + "' not found.");

    // parseResult = std::make_shared<STI::Engine::ParseResult>();
    // return parseResult;
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

std::shared_ptr<STI::Engine::SequenceResult> PersistenceManagerPy::getSequenceResult(const STI::Engine::SequenceID& id)
{
    std::shared_ptr<STI::Engine::SequenceResult> sequenceResult;

    if (persistenceManager != 0 && persistenceManager->getSequenceResult(id, sequenceResult)) {
        return sequenceResult;
    }

    //not found
    sequenceResult = std::make_shared<STI::Engine::SequenceResult>();
    return sequenceResult;
}

std::shared_ptr<STI::Utils::FileServer> PersistenceManagerPy::getFileServer()
{
    std::shared_ptr<STI::Utils::FileServer> fileServer;

    if (persistenceManager != 0 && persistenceManager->getFileServer(fileServer)) {
        return fileServer;
    }

    //not found
    // fileServer = std::make_shared<STI::Utils::FileServer>();
    // return fileServer;
    throw py::value_error("FileServer not found.");
}
