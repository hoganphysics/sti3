
#include "ParseTicket.h"
#include "ParseTicketManager.h"


using STI::Python::ParseTicket;
using STI::Python::ParseTicketManager;



ParseTicket::ParseTicket(const STI::Engine::ParseID& pid, ParseTicketManager* manager)
: pid(pid), ticketManager(manager)
{
    status = ParseTicketStatus::Parsing;
}

ParseTicket::~ParseTicket()
{
    if (ticketManager != 0) {
        ticketManager->remove(pid);
    }
}

const STI::Engine::ParseID& ParseTicket::getParseID() const
{
    return pid;
}


ParseTicket& ParseTicket::wait()
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

