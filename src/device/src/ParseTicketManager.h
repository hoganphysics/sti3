#ifndef STI_ENGINE_PARSETICKETMANAGER_H
#define STI_ENGINE_PARSETICKETMANAGER_H

#include "TicketManager.h"
#include "ParseID.h"
#include "DeviceMessageListener.h"
#include "DeviceMessage.h"
#include "ParseTicket.h"
#include "EventEngineScheduler.h"

#include <memory>


namespace STI
{
namespace Engine
{

class ParseID;


template<class T = ParseTicket>
class ParseTicketManager : public STI::Engine::TicketManager<STI::Engine::ParseID, T>,
                           public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>
{
public:

    ParseTicketManager(const std::shared_ptr<EventEngineScheduler>& scheduler);
    virtual ~ParseTicketManager() {}

    std::shared_ptr<T> makeTicket(const STI::Engine::ParseID& id);

private:

    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);

    std::shared_ptr<EventEngineScheduler> eventEngineScheduler;

};


template<class T>
ParseTicketManager<T>::ParseTicketManager(const std::shared_ptr<EventEngineScheduler>& scheduler)
: eventEngineScheduler(scheduler)
{
}

template<class T>
std::shared_ptr<T> ParseTicketManager<T>::makeTicket(const STI::Engine::ParseID& id)
{
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
        ticket->cancel();
        removeTicket = true;
        break;
    }

    if (removeTicket) {
        TicketManager<STI::Engine::ParseID, T>::remove(id);
    }

    return ticket;
}


template<class T>
void ParseTicketManager<T>::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    std::shared_ptr<T> ticket;
  
    const auto& id = mess->jobID.pid;
    if (!TicketManager<STI::Engine::ParseID, T>::get(id, ticket)) {
        return;
    }

    if (mess->schedulerMessageType == STI::Device::EngineSchedulerMessage::SchedulerMessageType::ParseComplete) {
        ticket->setComplete();
    }
    else {
        ticket->cancel();
    }

    TicketManager<STI::Engine::ParseID, T>::remove(id);  //avoid storing ticket indefinitely (memory leak)
}


} //Engine
} //STI

#endif

