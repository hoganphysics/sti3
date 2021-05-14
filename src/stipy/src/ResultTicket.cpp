#include "ResultTicket.h"

using STI::Python::ResultTicket;

ResultTicket::ResultTicket(const STI::Engine::ShotID& id, 
                const std::shared_ptr<STI::Device::Device>& server)
: Ticket(Ticket::TicketStatus::Running)
{
}

