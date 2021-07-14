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
};


} //Python
} //STI

#endif
