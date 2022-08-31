
#include <sti/engine/ParseTicket.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/RawEvent.h>
#include "ParseResult.h"
#include "RawEventGroup.h"

#include <chrono>


using STI::Engine::ParseTicket;
using STI::Engine::ParseID;
using STI::Engine::EventEngineScheduler;
using STI::Engine::RawEventGroup;



ParseTicket::ParseTicket(const ParseID& pid, const std::shared_ptr<EventEngineScheduler>& scheduler)
: Ticket(Ticket::TicketStatus::Running), pid(pid), engineScheduler(scheduler)
{
    parseResultBuffered = false;
}

ParseTicket::~ParseTicket()
{
}

const STI::Engine::ParseID& ParseTicket::getParseID() const
{
    return pid;
}

bool ParseTicket::getParseResult()
{
    if (checkParseResultBuffered()) return (parseResult != 0);

    parseResultBuffered = engineScheduler != 0 && engineScheduler->getParseResult(pid, parseResult);
    return parseResultBuffered && (parseResult != 0);
}

bool ParseTicket::checkParseResultBuffered() const
{
    if (parseResultBuffered && parseResult != 0) {
        //make sure the buffered value hasn't been overwritten
        if (parseResult->pid == pid) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<RawEventGroup> ParseTicket::getEvents()
{
    if (getParseResult() && parseResult->baseEventGroup != 0) {
        return parseResult->baseEventGroup;
    }

    auto emptyGroup = std::make_shared<RawEventGroup>();
    return emptyGroup;

    // if (!eventsBuffered && server != 0 && server->getEngineScheduler(scheduler) 
    //     && scheduler != 0 && scheduler->getParsedEvents(pid, events)) {

    // if (!eventsBuffered && engineScheduler != 0 && engineScheduler->getParseResult(pid, parseResult)) {
            
    //         eventsBuffered = true;
    // }

    // return events;
}

std::vector<STI::Engine::EngineParsingMessage> ParseTicket::getMessages()
{
    if (getParseResult()) {
        return parseResult->messages;
    }

    std::vector<STI::Engine::EngineParsingMessage> emptyMessages;
    return emptyMessages;

}
