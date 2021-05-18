
#include "ParseTicket.h"
// #include "ParseTicketManager.h"
#include "EventEngineScheduler.h"

#include "RawEvent.h"

#include <chrono>
#include <pybind11/pybind11.h>

using STI::Python::ParseTicket;
// using STI::Python::ParseTicketManager;

namespace py = pybind11;


ParseTicket::ParseTicket(const STI::Engine::ParseID& pid, 
                            const std::shared_ptr<STI::Device::Device>& server)
: Ticket(Ticket::TicketStatus::Running), pid(pid), server(server)
{
    // status = ParseTicketStatus::Parsing;

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

// void ParseTicket::wait()
// {
//     std::unique_lock<std::mutex> parseLock(parseMutex);

//     while (status == ParseTicketStatus::Parsing) {
//         parseCondition.wait_for(parseLock, std::chrono::milliseconds(100));

//         if (PyErr_CheckSignals() != 0) 
//             throw py::error_already_set();

//     }
// }

// void ParseTicket::setComplete()
// {
//     std::unique_lock<std::mutex> parseLock(parseMutex);

//     status = ParseTicketStatus::Complete;
//     parseCondition.notify_all();
// }

// void ParseTicket::cancel()
// {
//     std::unique_lock<std::mutex> parseLock(parseMutex);

//     status = ParseTicketStatus::Cancelled;
//     parseCondition.notify_all();
// }

