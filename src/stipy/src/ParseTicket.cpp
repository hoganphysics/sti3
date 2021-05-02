
#include "ParseTicket.h"
#include "ParseTicketManager.h"
#include "EventEngineScheduler.h"


#include <iostream>

using STI::Python::ParseTicket;
using STI::Python::ParseTicketManager;



ParseTicket::ParseTicket(const STI::Engine::ParseID& pid, ParseTicketManager* manager,
                            const std::shared_ptr<STI::Device::Device>& server)
: pid(pid), ticketManager(manager), server(server)
{
    status = ParseTicketStatus::Parsing;

    eventsBuffered = false;
    messagesBuffered = false;
}

ParseTicket::~ParseTicket()
{
    if (ticketManager != 0) {
        ticketManager->remove(pid);
    }
    //std::cout << "~ParseTicket()" << std::endl;
}

const STI::Engine::ParseID& ParseTicket::getParseID() const
{
    return pid;
}


STI::Engine::DeviceEventMap& ParseTicket::getEvents()
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (!eventsBuffered && server != 0 && server->getEngineScheduler(scheduler) 
        && scheduler != 0 && scheduler->getParsedEvents(pid, events)) {
            eventsBuffered = true;
    }

    return events;
}

std::vector<STI::Engine::EngineParsingMessage> ParseTicket::getMessages()
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    std::cout << "ParseTicket::getMessages" << std::endl;

    if (!messagesBuffered && server != 0 && server->getEngineScheduler(scheduler) 
        && scheduler != 0 && scheduler->getParsingMessages(pid, messages)) {

            std::cout << "messages success" << std::endl;

            messagesBuffered = true;
    }

    std::cout << "messages size: " << messages.size() << std::endl;

    return messages;
}

void ParseTicket::wait()
{
    std::unique_lock<std::mutex> parseLock(parseMutex);

    while (status == ParseTicketStatus::Parsing) {
        parseCondition.wait(parseLock);
    }
}

void ParseTicket::setComplete()
{
    std::unique_lock<std::mutex> parseLock(parseMutex);

    status = ParseTicketStatus::Complete;
    parseCondition.notify_all();
}

void ParseTicket::cancel()
{
    std::unique_lock<std::mutex> parseLock(parseMutex);

    status = ParseTicketStatus::Cancelled;
    parseCondition.notify_all();
}

