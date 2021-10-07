
#include "PyParseTicket.h"

#include <pybind11/pybind11.h>

using STI::Python::PyParseTicket;
using STI::Engine::ParseTicket;

namespace py = pybind11;


PyParseTicket::PyParseTicket(const STI::Engine::ParseID& id, 
            const std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
: ParseTicket(id, scheduler)
{
}
 
bool PyParseTicket::waitCheck()
{
    if (PyErr_CheckSignals() != 0) 
        throw py::error_already_set();
    
    return true;
}

