#ifndef STI_ENGINE_PARSETICKETMANAGER_H
#define STI_ENGINE_PARSETICKETMANAGER_H

#include <sti/engine/TicketManager.h>
#include <sti/engine/ParseID.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceMessage.h>
#include <sti/engine/ParseTicket.h>
#include <sti/engine/EventEngineScheduler.h>

#include <memory>
#include <mutex>


namespace STI
{
namespace Engine
{

class ParseID;


template<class T = ParseTicket>
class ParseTicketManager : public STI::Engine::TicketManager<STI::Engine::ParseID, T>,
                           public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>,
                           public STI::Device::DeviceMessageListener<STI::Device::EngineJobUpdateDeviceMessage>
{
public:

    ParseTicketManager(const std::shared_ptr<EventEngineScheduler>& scheduler);
    virtual ~ParseTicketManager() {}

    std::shared_ptr<T> makeTicket(const STI::Engine::ParseID& id);

private:

    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);
    void handleMessage(const std::shared_ptr<STI::Device::EngineJobUpdateDeviceMessage>& mess);

    std::shared_ptr<EventEngineScheduler> eventEngineScheduler;

    std::mutex managerMutex;

};


template<class T>
ParseTicketManager<T>::ParseTicketManager(const std::shared_ptr<EventEngineScheduler>& scheduler)
: eventEngineScheduler(scheduler)
{
}

template<class T>
std::shared_ptr<T> ParseTicketManager<T>::makeTicket(const STI::Engine::ParseID& id)
{
    std::unique_lock<std::mutex> managerLock(managerMutex);

    auto ticket = std::make_shared<T>(id, eventEngineScheduler);

    if (eventEngineScheduler == 0) {
        //not connected
        ticket->cancel();
        return ticket;
    }

    TicketManager<STI::Engine::ParseID, T>::add(id, ticket);
 
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
            ticket->cancel();
            removeTicket = true;
            break;
        case EngineJobStatus::Deferred:
            ticket->defer();
            removeTicket = true;
            break;
    }

    if (removeTicket) {
        TicketManager<STI::Engine::ParseID, T>::remove(id);
    }

    return ticket;
}


template<class T>
void ParseTicketManager<T>::handleMessage(const std::shared_ptr<STI::Device::EngineJobUpdateDeviceMessage>& mess)
{
    if (mess == 0) return;
    // if (mess->getEngineJob() == 0) return;
    if (mess->getJobID().type != EventEngineJobType::Parse) return;

    std::unique_lock<std::mutex> managerLock(managerMutex);

    std::shared_ptr<T> ticket;

    const auto& id = mess->getJobID().pid;
    if (!TicketManager<STI::Engine::ParseID, T>::get(id, ticket)) {
        return;
    }

    if (mess->getStatus() == EngineJobStatus::Canceled) {
        ticket->cancel();
        TicketManager<STI::Engine::ParseID, T>::remove(id);  //avoid storing ticket indefinitely (memory leak)
    }
    if (mess->getStatus() == EngineJobStatus::Completed) {
        ticket->setComplete();
        TicketManager<STI::Engine::ParseID, T>::remove(id);  //avoid storing ticket indefinitely (memory leak)
    }
}

template<class T>
void ParseTicketManager<T>::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    if (mess == 0) return;

    std::unique_lock<std::mutex> managerLock(managerMutex);

    std::shared_ptr<T> ticket;

    const auto& id = mess->jobID.pid;
    if (!TicketManager<STI::Engine::ParseID, T>::get(id, ticket)) {
        return;
    }

    if (mess->schedulerMessageType == STI::Device::EngineSchedulerMessage::SchedulerMessageType::ParseComplete) {
        ticket->setComplete();
        TicketManager<STI::Engine::ParseID, T>::remove(id);  //avoid storing ticket indefinitely (memory leak)
    }
    else {
        ticket->cancel();
        TicketManager<STI::Engine::ParseID, T>::remove(id);  //avoid storing ticket indefinitely (memory leak)
    }
}


} //Engine
} //STI

#endif

