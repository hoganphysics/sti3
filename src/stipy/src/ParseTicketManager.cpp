

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
    cancelAll();
}

std::shared_ptr<ParseTicket> ParseTicketManager::makeParseTicket(const STI::Engine::ParseID& pid,
                                                    const std::shared_ptr<STI::Device::Device>& server)
{
    auto ticket = std::make_shared<ParseTicket>(pid, this, server);

    add(ticket);

    return ticket;
}

void ParseTicketManager::add(const std::shared_ptr<ParseTicket>& ticket)
{
    if (ticket != 0) {
        tickets[ticket->getParseID()] = ticket;
    }
}

void ParseTicketManager::remove(const STI::Engine::ParseID& id)
{
    auto it = tickets.find(id);

    if(it != tickets.end()) {
    //    tickets.erase(it);    //causing double free() in python
    }
}

void ParseTicketManager::cancel(const STI::Engine::ParseID& id)
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
        if (ticket.second != 0 ) {
            ticket.second->cancel();
        }
    }
}

void ParseTicketManager::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    auto it = tickets.find(mess->jobID.pid);

    if (it == tickets.end()) return;

    typedef EngineSchedulerMessage::SchedulerMessageType MessageType;
    if (mess->schedulerMessageType == MessageType::ParseComplete) {
        it->second->setComplete();
    }
    else {      //todo -- if cancel
        it->second->cancel();
    }
}

