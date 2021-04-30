

#include "ParseTicketManager.h"
#include "ParseTicket.h"

using STI::Python::ParseTicketManager;
using STI::Python::ParseTicket;

ParseTicketManager::ParseTicketManager()
{
}

ParseTicketManager::~ParseTicketManager()
{
}

std::shared_ptr<ParseTicket> ParseTicketManager::makeParseTicket(const STI::Engine::ParseID& pid)
{
    auto ticket = std::make_shared<ParseTicket>(pid, this);

    return ticket;
}

void ParseTicketManager::add(const ParseTicket& ticket)
{
    tickets[ticket.getParseID()] = &ticket;
}

void ParseTicketManager::remove(const STI::Engine::ParseID& id)
{
    auto it = tickets.find(id);

    if(it != tickets.end()) {
        tickets.erase(it);
    }
}

void ParseTicketManager::cancel(const STI::Engine::ParseID&)
{
    auto it = tickets.find(id);
    
    if(it != tickets.end()) {
        if (it->second != 0) {
            it->second->cancel();
        }
    }
}

void ParseTicketManager::cancelAll()
{
    for (auto& ticket : tickets) {
        if (ticket->second != 0) {
            ticket->second->cancel();
        }
    }
}

void ParseTicketManager::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    auto it = tickets.find(mess->jobID.pid);

    if (it == tickets.end()) return;

    typedef EngineSchedulerMessage::SchedulerMessageType MessageType;
    if (mess->schedulerMessageType == MessageType::ParseComplete) {
        it->second.setComplete();
    }
    else {      //todo -- if cancel
        it->second.cancel();
    }

}

