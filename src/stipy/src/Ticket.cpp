
#include "Ticket.h"

#include <chrono>
#include <pybind11/pybind11.h>


using STI::Python::Ticket;


namespace py = pybind11;

Ticket::Ticket(const Ticket::TicketStatus& initalStatus)
: status(initalStatus)
{
}

void Ticket::wait()
{
    std::unique_lock<std::mutex> statusLock(statusMutex);

    while (status == TicketStatus::Running) {
        statusCondition.wait_for(statusLock, std::chrono::milliseconds(100));

        if (PyErr_CheckSignals() != 0) 
            throw py::error_already_set();
    }
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

    status = TicketStatus::Cancelled;
    statusCondition.notify_all();
}

