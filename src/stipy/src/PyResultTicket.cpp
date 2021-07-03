#include "PyResultTicket.h"

#include <pybind11/pybind11.h>


using STI::Python::PyResultTicket;
using STI::Engine::ResultTicket;

namespace py = pybind11;


PyResultTicket::PyResultTicket(const STI::Engine::ShotID& id, 
                const std::shared_ptr<STI::Device::Device>& server)
: ResultTicket(id, server)
{
}

PyResultTicket::PyResultTicket(const STI::Engine::ShotID& id, 
            const std::shared_ptr<STI::Device::Device>& server, const TicketStatus& initialStatus)
: ResultTicket(id, server, initialStatus)
{
}


bool PyResultTicket::waitCheck()
{
    if (PyErr_CheckSignals() != 0) 
        throw py::error_already_set();
    
    return true;
}

