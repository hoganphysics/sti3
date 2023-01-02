#include <sti/engine/Ticket.h>

#include <chrono>

using STI::Engine::Ticket;


Ticket::Ticket(const Ticket::TicketStatus& initalStatus)
: status(initalStatus)
{
}

void Ticket::wait() const
{
    //default to (virtual) member function
    wait( [this](){ return waitCheck(); });
}

void Ticket::wait(const std::function<bool()>& waitChecker) const
{
    std::unique_lock<std::mutex> statusLock(statusMutex);

    bool keepWaiting = true;

    while (status == TicketStatus::Running && keepWaiting) {
        statusCondition.wait_for(statusLock, std::chrono::milliseconds(100));

        keepWaiting = waitChecker();
    }

}

bool Ticket::waitCheck() const
{
    return true;
}

void Ticket::setComplete()
{
    std::unique_lock<std::mutex> statusLock(statusMutex);

    status = TicketStatus::Complete;
    statusCondition.notify_all();
}

void Ticket::cancel()
{
    std::unique_lock<std::mutex> statusLock(statusMutex);

    status = TicketStatus::Canceled;
    statusCondition.notify_all();
}

void Ticket::defer()
{
    std::unique_lock<std::mutex> statusLock(statusMutex);

    status = TicketStatus::Deferred;
    statusCondition.notify_all();
}

Ticket::TicketStatus Ticket::getStatus() const
{
    std::unique_lock<std::mutex> statusLock(statusMutex);
    return status;
}

std::string Ticket::statusToString(const Ticket::TicketStatus& status)
{
    //{ Running, Complete, Canceled, NotFound, Deferred }

    std::string result = "";

    switch (status)
    {
    case Ticket::TicketStatus::Running:
        result = "Running";
        break;
    case Ticket::TicketStatus::Complete:
        result = "Complete";
        break;
    case Ticket::TicketStatus::Canceled:
        result = "Canceled";
        break;
    case Ticket::TicketStatus::NotFound:
        result = "NotFound";
        break;
    case Ticket::TicketStatus::Deferred:
        result = "Deferred";
        break;    
    default:
        break;
    }

    return result;
}