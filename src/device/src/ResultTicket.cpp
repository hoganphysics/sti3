#include <sti/engine/ResultTicket.h>

#include <sti/device/PersistenceManager.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/ShotResult.h>

#include "ShotRepository.h"

using STI::Engine::ResultTicket;
using STI::Engine::ShotRepository;
using STI::Device::PersistenceManager;
using STI::Engine::ShotResult;
using STI::Engine::ParseResult;


ResultTicket::ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<PersistenceManager>& persistenceManager)
// : Ticket(Ticket::TicketStatus::Running)
: ResultTicket(id, persistenceManager, Ticket::TicketStatus::Running)
{
}

// ResultTicket::ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<STI::Device::Device>& server, const TicketStatus& initialStatus)
// : Ticket(initialStatus), sid(id), measurements_loaded(false)
// {
    // if (server != 0) {
    //     std::shared_ptr<STI::Device::PersistenceManager> manager;
    //     server->getPersistenceManager(manager);
    //     if (manager != 0) {
    //         manager->getShotRepository(shotRepository);
    //     }
    // }

// }

ResultTicket::ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<PersistenceManager>& persistenceManager, const TicketStatus& initialStatus)
: Ticket(initialStatus), sid(id), persistenceManager(persistenceManager)
{
}

STI::Engine::ShotID ResultTicket::getShotID() const
{
    return sid;
}

// bool ResultTicket::getShotRepository(std::shared_ptr<ShotRepository>& repo)
// {
//     repo = shotRepository;
//     return repo != 0;
// }

std::shared_ptr<ParseResult> ResultTicket::getParseResult()
{
    if (ensureCachedParseResult()) {
        return parseResult.get();
    }

    auto missing = std::shared_ptr<ParseResult>();
    return missing;
}

std::shared_ptr<ShotResult> ResultTicket::getShotResult()
{
    if (ensureCachedShotResult()) {
        return shotResult.get();
    }

    auto missing = std::shared_ptr<ShotResult>();
    return missing;
}

STI::Engine::MeasurementVector ResultTicket::measurements()
{
    if (ensureCachedMeasurements()) {
        return *(cachedMeasurements.get());
    }

    STI::Engine::MeasurementVector missing;
    return missing;
}

STI::Engine::MeasurementVector ResultTicket::measurements(const STI::Device::DeviceID& id)
{
    STI::Engine::MeasurementVector selected;

    if (ensureCachedMeasurements()) {
        for (auto& m : *cachedMeasurements.get()) {
            if (m != 0 && m->device() == id) {
                selected.push_back(m);
            }
        }
    }

    return selected;
}

STI::Engine::MeasurementVector ResultTicket::measurements(const std::string& id)
{
    STI::Device::DeviceID deviceID(id);

    return measurements(deviceID);
}

bool ResultTicket::isQueryable()
{
    //only tickets that are Complete or Canceled can be queryed for results
    bool queryable = (getStatus() == Ticket::TicketStatus::Complete) || (getStatus() == Ticket::TicketStatus::Canceled);
    return queryable;
}

bool ResultTicket::ensureCachedParseResult()
{
    if (parseResult.isCached()) return (parseResult.get() != 0);

    if (!isQueryable()) return false;

    std::shared_ptr<ParseResult> result;

    if (persistenceManager != 0 && persistenceManager->getParseResult(sid.parseID, result) && result != 0) {
        parseResult.set(result);
        return (parseResult.get() != 0);
    }
    
    return false;
}

bool ResultTicket::ensureCachedShotResult()
{
    if (shotResult.isCached()) return (shotResult.get() != 0);

    if (!isQueryable()) return false;

    std::shared_ptr<ShotResult> result;

    if (persistenceManager != 0 && persistenceManager->getShotResult(sid, result) && result != 0) {
        shotResult.set(result);
        return (shotResult.get() != 0);
    }
    
    return false;
}

bool ResultTicket::ensureCachedMeasurements()
{
    if (cachedMeasurements.isCached()) return (cachedMeasurements.get() != 0);

    if (shotResult.isCached() && shotResult.get() != 0) {
        cachedMeasurements.set(shotResult.get()->measurements);
        return (cachedMeasurements.get() != 0);
    }

    if (!isQueryable()) return false;

    std::shared_ptr<STI::Engine::MeasurementVector> measResults;
    if (persistenceManager != 0 && persistenceManager->getMeasurements(sid, measResults) && measResults != 0) {
        cachedMeasurements.set(measResults);
        return (cachedMeasurements.get() != 0);
    }

    return false;
}


// void ResultTicket::loadMeasurements()
// {
//     if (measurements_loaded && measurements_ != 0) {
//         return;
//     }

//     // if (shotResult == 0) {
//     //     shotResult = std::make_shared<ShotResult>();
//     // }
    
//     if (persistenceManager != 0 && persistenceManager->getMeasurements(sid, measurements_)) {
//         measurements_loaded = true;
//         return;
//     }
    
//     measurements_ = std::make_shared<STI::Engine::MeasurementVector>();
// }

