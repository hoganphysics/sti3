
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
};


} //Python
} //STI

#endif

