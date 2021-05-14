

#include "ParseTicketManager.h"
#include "ParseTicket.h"
#include "DeviceMessage.h"

#include <iostream>

using STI::Python::ParseTicketManager;
using STI::Python::ParseTicket;
using STI::Device::EngineSchedulerMessage;


ParseTicketManager::ParseTicketManager()
{
}

ParseTicketManager::~ParseTicketManager()
{
    // cancelAll();
}

// std::shared_ptr<ParseTicket> ParseTicketManager::makeParseTicket(const STI::Engine::ParseID& pid,
//                                                     const std::shared_ptr<STI::Device::Device>& server)
// {
//     auto ticket = std::make_shared<ParseTicket>(pid, server);

//     add(ticket);

//     return ticket;
// }

// void ParseTicketManager::add(const std::shared_ptr<ParseTicket>& ticket)
// {
//     std::unique_lock<std::mutex> ticketLock(ticketMutex);

//     if (ticket != 0) {
//         tickets[ticket->getParseID()] = ticket;
//     }
// }

// void ParseTicketManager::remove(const STI::Engine::ParseID& id)
// {
//     std::unique_lock<std::mutex> ticketLock(ticketMutex);

//     auto it = tickets.find(id);

//     if(it != tickets.end()) {
//        tickets.erase(it);    //causing double free() in python
//     }
// }

// void ParseTicketManager::cancel(const STI::Engine::ParseID& id)
// {
//     std::unique_lock<std::mutex> ticketLock(ticketMutex);

//     auto it = tickets.find(id);
    
//     if(it != tickets.end()) {
//         if (it->second != 0) {
//             it->second->cancel();
//         }
//     }
// }

// void ParseTicketManager::cancelAll()
// {
//     std::unique_lock<std::mutex> ticketLock(ticketMutex);

//     for (auto& ticket : tickets) {
//         if (ticket.second != 0 ) {
//             ticket.second->cancel();
//         }
//     }
// }

// void ParseTicketManager::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
// {
//     std::unique_lock<std::mutex> ticketLock(ticketMutex);

//     auto it = tickets.find(mess->jobID.pid);

//     if (it == tickets.end()) return;

//     typedef EngineSchedulerMessage::SchedulerMessageType MessageType;
//     if (mess->schedulerMessageType == MessageType::ParseComplete) {
//         it->second->setComplete();
//     }
//     else {      //todo -- if cancel
//         it->second->cancel();
//     }

//     // Ticket complete
//     tickets.erase(it);  //avoid storing ticket indefinitely (memory leak)
// }



void ParseTicketManager::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    std::shared_ptr<ParseTicket> ticket;

    std::cout << "ParseTicketManager::handleMessage" << std::endl;
    
    const auto& id = mess->jobID.pid;
    if (!get(id, ticket)) {
        return;
    }

    typedef EngineSchedulerMessage::SchedulerMessageType MessageType;
    if (mess->schedulerMessageType == MessageType::ParseComplete) {
        ticket->setComplete();
    }
    else {      //todo -- if cancel
        ticket->cancel();
    }

    remove(id);  //avoid storing ticket indefinitely (memory leak)
}

