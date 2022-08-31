#include <sti/engine/ResultTicket.h>
#include "ShotRepository.h"
#include <sti/device/PersistenceManager.h>
#include <sti/engine/Measurement.h>
#include "ShotResult.h"


using STI::Engine::ResultTicket;
using STI::Engine::ShotRepository;
using STI::Device::PersistenceManager;
using STI::Engine::ShotResult;


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
: Ticket(initialStatus), sid(id), persistenceManager(persistenceManager), measurements_loaded(false)
{
}

STI::Engine::ShotID ResultTicket::getShotID()
{
    return sid;
}

// bool ResultTicket::getShotRepository(std::shared_ptr<ShotRepository>& repo)
// {
//     repo = shotRepository;
//     return repo != 0;
// }

STI::Engine::MeasurementVector ResultTicket::measurements()
{
    loadMeasurements();
    return *measurements_;
}

STI::Engine::MeasurementVector ResultTicket::measurements(const STI::Device::DeviceID& id)
{
    loadMeasurements();

    STI::Engine::MeasurementVector selected;

    for (auto& m : *measurements_) {
        if (m->device() == id) {
            selected.push_back(m);
        }
    }
    return selected;
}

void ResultTicket::loadMeasurements()
{
    if (measurements_loaded && measurements_ != 0) {
        return;
    }

    // if (shotResult == 0) {
    //     shotResult = std::make_shared<ShotResult>();
    // }
    
    if (persistenceManager != 0 && persistenceManager->getMeasurements(sid, measurements_)) {
        measurements_loaded = true;
        return;
    }
    
    measurements_ = std::make_shared<STI::Engine::MeasurementVector>();
}

