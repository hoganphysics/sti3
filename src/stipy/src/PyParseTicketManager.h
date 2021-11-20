#ifndef STI_PYTHON_PYPARSETICKETMANAGER_H
#define STI_PYTHON_PYPARSETICKETMANAGER_H

#include "ParseTicketManager.h"
#include "PyParseTicket.h"

namespace STI
{
namespace Python
{


class PyParseTicketManager : public STI::Engine::ParseTicketManager<PyParseTicket>
{
public:
    PyParseTicketManager(const std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler) 
    : STI::Engine::ParseTicketManager<PyParseTicket>(scheduler) {}
};


} //Python
} //STI

#endif
