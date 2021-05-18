#include "ResultTicket.h"

using STI::Python::ResultTicket;

ResultTicket::ResultTicket(const STI::Engine::ShotID& id, 
                const std::shared_ptr<STI::Device::Device>& server)
// : Ticket(Ticket::TicketStatus::Running)
: ResultTicket(id, server, Ticket::TicketStatus::Running)
{
}

ResultTicket::ResultTicket(const STI::Engine::ShotID& id, 
            const std::shared_ptr<STI::Device::Device>& server, const TicketStatus& initialStatus)
: Ticket(initialStatus), sid(id), server(server)
{
}
