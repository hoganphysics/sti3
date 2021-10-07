#ifndef STI_ENGINE_PARSETICKETMANAGER_H
#define STI_ENGINE_PARSETICKETMANAGER_H

#include "TicketManager.h"
#include "ParseID.h"
#include "DeviceMessageListener.h"
#include "DeviceMessage.h"
#include "ParseTicket.h"

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

    ParseTicketManager() {}
    virtual ~ParseTicketManager() {}

    std::shared_ptr<T> makeTicket(const STI::Engine::ParseID& id, const std::shared_ptr<EventEngineScheduler>& scheduler);

private:

    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);

};


template<class T>
std::shared_ptr<T> ParseTicketManager<T>::makeTicket(const STI::Engine::ParseID& id, const std::shared_ptr<EventEngineScheduler>& scheduler)
{
    auto ticket = std::make_shared<T>(id, scheduler);

    TicketManager<STI::Engine::ParseID, T>::add(id, ticket);

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

