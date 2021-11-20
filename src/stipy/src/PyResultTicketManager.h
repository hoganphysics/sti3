
#ifndef STI_PYTHON_PYRESULTTICKETMANAGER_H
#define STI_PYTHON_PYRESULTTICKETMANAGER_H

#include "ResultTicketManager.h"
#include "PyResultTicket.h"


namespace STI
{
namespace Python
{


class PyResultTicketManager : public STI::Engine::ResultTicketManager<PyResultTicket>
{
public:
    PyResultTicketManager(const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager, 
                          const std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler) 
    : STI::Engine::ResultTicketManager<PyResultTicket>(persistenceManager, scheduler) {}
};


} //Python
} //STI

#endif

