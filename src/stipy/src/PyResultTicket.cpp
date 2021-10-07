#include "PyResultTicket.h"

#include <pybind11/pybind11.h>


using STI::Python::PyResultTicket;
using STI::Engine::ResultTicket;
using STI::Engine::ShotRepository;
using STI::Engine::ShotID;

namespace py = pybind11;


PyResultTicket::PyResultTicket(const ShotID& id, const std::shared_ptr<ShotRepository>& shotRepository)
: ResultTicket(id, shotRepository)
{
}

PyResultTicket::PyResultTicket(const ShotID& id, 
            const std::shared_ptr<ShotRepository>& shotRepository, const TicketStatus& initialStatus)
: ResultTicket(id, shotRepository, initialStatus)
{
}


bool PyResultTicket::waitCheck()
{
    if (PyErr_CheckSignals() != 0) 
        throw py::error_already_set();
    
    return true;
}

