#include <sti/engine/ParseTicket.h>

#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ParseResult.h>

#include <sti/engine/RawEventGroup.h>

#include <chrono>

using STI::Engine::ParseTicket;
using STI::Engine::ParseID;
using STI::Engine::EventEngineScheduler;
using STI::Engine::RawEventGroup;
using STI::Engine::ParseResult;


ParseTicket::ParseTicket(const ParseID& pid, const std::shared_ptr<EventEngineScheduler>& scheduler)
: Ticket(Ticket::TicketStatus::Running), pid(pid), engineScheduler(scheduler)
{
    // parseResultBuffered = false;
}

ParseTicket::~ParseTicket()
{
}

const STI::Engine::ParseID& ParseTicket::getParseID() const
{
    return pid;
}

std::shared_ptr<ParseResult> ParseTicket::getParseResult()
{
    if (ensureCachedParseResult()) {
        return parseResult.get();
    }

    auto missingResult = std::make_shared<ParseResult>();
    return missingResult;
}

bool ParseTicket::ensureCachedParseResult()
{
    if (parseResult.isCached()) return (parseResult.get() != 0);
    
    //only tickets that are Complete or Canceled can be queryed for results
    bool queryable = (getStatus() == Ticket::TicketStatus::Complete) || (getStatus() == Ticket::TicketStatus::Canceled);

    if (!queryable) return false;

    std::shared_ptr<ParseResult> result;

    if (engineScheduler != 0 && engineScheduler->getParseResult(pid, result) && result != 0) {
        parseResult.set(result);
        return (parseResult.get() != 0);
    }
    
    return false;

    // if (checkParseResultBuffered()) return (parseResult != 0);

    // parseResultBuffered = engineScheduler != 0 && engineScheduler->getParseResult(pid, parseResult);
    // return parseResultBuffered && (parseResult != 0);
}

// bool ParseTicket::checkParseResultBuffered() const
// {
//     if (parseResultBuffered && parseResult != 0) {
//         //make sure the buffered value hasn't been overwritten
//         if (parseResult->pid == pid) {
//             return true;
//         }
//     }
//     return false;
// }

std::shared_ptr<RawEventGroup> ParseTicket::getEvents()
{
    if (ensureCachedParseResult() && parseResult.get()->baseEventGroup != 0) {
        return parseResult.get()->baseEventGroup;
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
    if (ensureCachedParseResult()) {
        // return parseResult->messages;
        return parseResult.get()->messages;
    }

    std::vector<STI::Engine::EngineParsingMessage> emptyMessages;
    return emptyMessages;

}
