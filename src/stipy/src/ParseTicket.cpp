
#include "ParseTicket.h"

using STI::Python::ParseTicket;

ParseTicket::ParseTicket(const STI::Engine::ParseID& pid)
: pid(pid)
{
}

const STI::Engine::ParseID& ParseTicket::getParseID() const
{
    return pid;
}
