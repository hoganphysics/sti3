
#include "ParseTicket.h"
#include "EventEngineScheduler.h"
#include "RawEvent.h"

#include <chrono>


using STI::Engine::ParseTicket;



ParseTicket::ParseTicket(const STI::Engine::ParseID& pid, 
                            const std::shared_ptr<STI::Device::Device>& server)
: Ticket(Ticket::TicketStatus::Running), pid(pid), server(server)
{
    eventsBuffered = false;
    messagesBuffered = false;
}

ParseTicket::~ParseTicket()
{
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

    if (!messagesBuffered && server != 0 && server->getEngineScheduler(scheduler) 
        && scheduler != 0 && scheduler->getParsingMessages(pid, messages)) {

            messagesBuffered = true;
    }

    return messages;
}
