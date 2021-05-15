
#include "ResultTicketManager.h"
#include "ResultTicket.h"
#include "ShotID.h"
#include "DeviceMessage.h"

using STI::Python::ResultTicketManager;
using STI::Python::ResultTicket;
using STI::Engine::ShotID;
using STI::Device::EngineSchedulerMessage;


ResultTicketManager::ResultTicketManager()
{
}

ResultTicketManager::~ResultTicketManager()
{
}

void ResultTicketManager::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    std::shared_ptr<ResultTicket> ticket;
    
    const auto& id = mess->jobID.sid;
    if (!get(id, ticket)) {
        return;
    }

    // The engine should return to Parsed state after successful play
    if (mess->engineState == STI::Engine::EngineState::Parsed) {
        ticket->setComplete();
    }
    else {
        ticket->cancel();
    }

    remove(id);  //avoid storing ticket indefinitely (memory leak)
}
