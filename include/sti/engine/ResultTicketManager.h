
#ifndef STI_ENGINE_RESULTTICKETMANAGER_H
#define STI_ENGINE_RESULTTICKETMANAGER_H

#include <sti/engine/ShotID.h>
#include <sti/engine/TicketManager.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/engine/ResultTicket.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/PersistenceManager.h>
#include <sti/engine/EventEngineScheduler.h>

#include <memory>

namespace STI
{
namespace Engine
{

template <class T = ResultTicket>
class ResultTicketManager : public STI::Engine::TicketManager<STI::Engine::ShotID, T>,
                            public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>
{
public:
    ResultTicketManager(const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager, 
                        const std::shared_ptr<EventEngineScheduler>& scheduler);
    virtual ~ResultTicketManager() {}

    std::shared_ptr<T> makeTicket(const STI::Engine::ShotID &id);

private:
    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage> &mess);

    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
    std::shared_ptr<EventEngineScheduler> eventEngineScheduler;
};


template <class T>
ResultTicketManager<T>::ResultTicketManager(const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager,
                                            const std::shared_ptr<EventEngineScheduler>& scheduler)
    : persistenceManager(persistenceManager), eventEngineScheduler(scheduler)
{
}

template <class T>
std::shared_ptr<T> ResultTicketManager<T>::makeTicket(const STI::Engine::ShotID &id)
{
    using STI::Engine::EngineJobStatus;

    auto ticket = std::make_shared<T>(id, persistenceManager);

    if (persistenceManager == 0 || eventEngineScheduler == 0) {
        //not connected
        ticket->cancel();
        return ticket;
    }

    TicketManager<STI::Engine::ShotID, T>::add(id, ticket);

    //The job could have completed before the ticket was created and added to the manager
    bool removeTicket = false;
    switch (eventEngineScheduler->getStatus(id))
    {
    case EngineJobStatus::New:
        break;
    case EngineJobStatus::Running:
        break;
    case EngineJobStatus::Completed:
        ticket->setComplete();
        removeTicket = true;
        break;
    case EngineJobStatus::Canceled:
        ticket->cancel();
        removeTicket = true;
        break;
    case EngineJobStatus::NotFound:
    case EngineJobStatus::Archived:
        //check if result is archived
        if (persistenceManager->findShot(id)) {
            ticket->setComplete();
        }
        else {
            //not found in results archive
            ticket->cancel();
        }
        removeTicket = true;    //i.e., don't wait for job events
        break;
    }

    if (removeTicket) {
        TicketManager<STI::Engine::ShotID, T>::remove(id);
    }

    return ticket;
}

template <class T>
void ResultTicketManager<T>::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage> &mess)
{
    if (mess == 0)
        return;

    std::shared_ptr<T> ticket;

    const auto &id = mess->jobID.sid;
    if (!TicketManager<STI::Engine::ShotID, T>::get(id, ticket))
    {
        return;
    }

    if (mess->schedulerMessageType == STI::Device::EngineSchedulerMessage::SchedulerMessageType::PlayComplete)
    {
        ticket->setComplete();
        TicketManager<STI::Engine::ShotID, T>::remove(id);
    }
    else
    {
        //    ticket->cancel();
    }

    //TicketManager<STI::Engine::ShotID, T>::remove(id);  //avoid storing ticket indefinitely (memory leak)
}

} //Engine
} //STI

#endif
