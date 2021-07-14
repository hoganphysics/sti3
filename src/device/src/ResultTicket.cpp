#include "ResultTicket.h"
#include "ShotRepository.h"
#include "PersistenceManager.h"

using STI::Engine::ResultTicket;
using STI::Engine::ShotRepository;


ResultTicket::ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<STI::Device::Device>& server)
// : Ticket(Ticket::TicketStatus::Running)
: ResultTicket(id, server, Ticket::TicketStatus::Running)
{
}

ResultTicket::ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<STI::Device::Device>& server, const TicketStatus& initialStatus)
: Ticket(initialStatus), sid(id)
{
    if (server != 0) {
        std::shared_ptr<STI::Device::PersistenceManager> manager;
        server->getPersistenceManager(manager);
        if (manager != 0) {
            manager->getShotRepository(shotRepository);
        }
    }

}

ResultTicket::ResultTicket(const STI::Engine::ShotID& id, const std::shared_ptr<ShotRepository>& repo, const TicketStatus& initialStatus)
: Ticket(initialStatus), sid(id), shotRepository(repo)
{
}

STI::Engine::ShotID ResultTicket::getShotID()
{
    return sid;
}

bool ResultTicket::getShotRepository(std::shared_ptr<ShotRepository>& repo)
{
    repo = shotRepository;
    return repo != 0;
}
