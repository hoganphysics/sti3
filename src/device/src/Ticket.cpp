
#include "Ticket.h"

#include <chrono>
// #include <pybind11/pybind11.h>


using STI::Engine::Ticket;


// namespace py = pybind11;

Ticket::Ticket(const Ticket::TicketStatus& initalStatus)
: status(initalStatus)
{
}

void Ticket::wait()
{
    std::unique_lock<std::mutex> statusLock(statusMutex);

    bool keepWaiting = true;

    while (status == TicketStatus::Running && keepWaiting) {
        statusCondition.wait_for(statusLock, std::chrono::milliseconds(100));

        keepWaiting = waitCheck();
    }
}

bool Ticket::waitCheck()
{
    // if (PyErr_CheckSignals() != 0) 
    //     throw py::error_already_set();
    
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
