#include <sti/engine/Ticket.h>

#include <chrono>

using STI::Engine::Ticket;


Ticket::Ticket(const Ticket::TicketStatus& initalStatus)
: status(initalStatus)
{
}

void Ticket::wait()
{
    //default to (virtual) member function
    wait( [this](){ return waitCheck(); });
}

void Ticket::wait(const std::function<bool()>& waitChecker)
{
    std::unique_lock<std::mutex> statusLock(statusMutex);

    bool keepWaiting = true;

    while (status == TicketStatus::Running && keepWaiting) {
        statusCondition.wait_for(statusLock, std::chrono::milliseconds(100));

        keepWaiting = waitChecker();
    }

}

bool Ticket::waitCheck()
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

Ticket::TicketStatus Ticket::getStatus()
{
    std::unique_lock<std::mutex> statusLock(statusMutex);
    return status;
}
