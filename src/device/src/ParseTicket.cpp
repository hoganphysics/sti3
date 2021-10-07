
#include "ParseTicket.h"
#include "EventEngineScheduler.h"
#include "RawEvent.h"

#include <chrono>


using STI::Engine::ParseTicket;
using STI::Engine::ParseID;
using STI::Engine::EventEngineScheduler;


ParseTicket::ParseTicket(const ParseID& pid, const std::shared_ptr<EventEngineScheduler>& scheduler)
: Ticket(Ticket::TicketStatus::Running), pid(pid), engineScheduler(scheduler)
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
    
    // if (!eventsBuffered && server != 0 && server->getEngineScheduler(scheduler) 
    //     && scheduler != 0 && scheduler->getParsedEvents(pid, events)) {

    if (!eventsBuffered && engineScheduler != 0 && engineScheduler->getParsedEvents(pid, events)) {
            
            eventsBuffered = true;
    }

    return events;
}

std::vector<STI::Engine::EngineParsingMessage> ParseTicket::getMessages()
{
    if (!messagesBuffered && engineScheduler != 0 && engineScheduler->getParsingMessages(pid, messages)) {

            messagesBuffered = true;
    }

    return messages;
}
