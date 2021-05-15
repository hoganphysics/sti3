

#include "ParseTicketManager.h"
#include "ParseTicket.h"
#include "DeviceMessage.h"

using STI::Python::ParseTicketManager;
using STI::Python::ParseTicket;
using STI::Device::EngineSchedulerMessage;


ParseTicketManager::ParseTicketManager()
{
}

ParseTicketManager::~ParseTicketManager()
{
}

void ParseTicketManager::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    std::shared_ptr<ParseTicket> ticket;
  
    const auto& id = mess->jobID.pid;
    if (!get(id, ticket)) {
        return;
    }

    if (mess->engineState == STI::Engine::EngineState::Parsed) {
        ticket->setComplete();
    }
    else {
        ticket->cancel();
    }

    remove(id);  //avoid storing ticket indefinitely (memory leak)
}

