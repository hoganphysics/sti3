
#ifndef STI_ENGINE_RESULTTICKETMANAGER_H
#define STI_ENGINE_RESULTTICKETMANAGER_H

#include "ShotID.h"
#include "TicketManager.h"
#include "DeviceMessageListener.h"
#include "ResultTicket.h"
#include "DeviceMessage.h"

#include <memory>


namespace STI
{
namespace Engine
{


template<class T = ResultTicket>
class ResultTicketManager : public STI::Engine::TicketManager<STI::Engine::ShotID, T>,
                            public STI::Device::DeviceMessageListener<STI::Device::EngineSchedulerMessage>
{
public:

    ResultTicketManager() {}
    virtual ~ResultTicketManager() {}

    std::shared_ptr<T> makeTicket(const STI::Engine::ShotID& id, const std::shared_ptr<ShotRepository>& shotRepository);

private:

    void handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess);

};

template<class T>
std::shared_ptr<T> ResultTicketManager<T>::makeTicket(const STI::Engine::ShotID& id, const std::shared_ptr<ShotRepository>& shotRepository)
{
    auto ticket = std::make_shared<T>(id, shotRepository);

    TicketManager<STI::Engine::ShotID, T>::add(id, ticket);

    return ticket;
}

template<class T>
void ResultTicketManager<T>::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    if (mess == 0) return;

    std::shared_ptr<T> ticket;

    const auto& id = mess->jobID.sid;
    if (!TicketManager<STI::Engine::ShotID, T>::get(id, ticket)) {
        return;
    }

    if (mess->schedulerMessageType == STI::Device::EngineSchedulerMessage::SchedulerMessageType::PlayComplete) {
        ticket->setComplete();
    }
    else {
        ticket->cancel();
    }

    TicketManager<STI::Engine::ShotID, T>::remove(id);  //avoid storing ticket indefinitely (memory leak)
}


} //Engine
} //STI

#endif

