#include "PyResultTicket.h"

#include <pybind11/pybind11.h>

using STI::Python::PyResultTicket;
using STI::Engine::ResultTicket;
using STI::Engine::ShotID;

namespace py = pybind11;


PyResultTicket::PyResultTicket(const ShotID& id, const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager)
: ResultTicket(id, persistenceManager)
{
}

PyResultTicket::PyResultTicket(const ShotID& id, 
            const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager, const TicketStatus& initialStatus)
: ResultTicket(id, persistenceManager, initialStatus)
{
}

bool PyResultTicket::waitCheck() const
{
    {
        pybind11::gil_scoped_acquire acquire;

        if (PyErr_CheckSignals() != 0) {
            throw py::error_already_set();
            return false;
        }        
    }

    pybind11::gil_scoped_release release;

    return true;
}
