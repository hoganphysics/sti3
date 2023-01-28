
#include "Convert_ResultTicket.h"
#include "Convert_EventEngine.h"

#include <sti/engine/ResultTicket.h>
#include "RemoteResultsCollector.h"
#include <sti/engine/ShotRepository.h>
//#include "NetworkShotRepositoryWrapper.h"
#include "RemoteShotRepository.h"


using STI::Network::convert;
using STI::Engine::ResultTicket;
using STI::TNetwork::TResultTicket;
using STI::TNetwork::TResultTicket_var;
using STI::Engine::ShotID;
using STI::TNetwork::TShotID;
using STI::Engine::ShotRepository;
using STI::Engine::Ticket;
using STI::TNetwork::TTicketStatus;
using STI::Engine::ShotRepository;
// using STI::TNetwork::TShotRepository_var;
// using STI::Network::NetworkShotRepositoryWrapper;
// using STI::Network::RemoteShotRepository;


//ResultsCollector
template<>
bool STI::Network::convert<TResultTicket, std::shared_ptr<ResultTicket>>(
        const TResultTicket& tResultTicket, std::shared_ptr<ResultTicket>& resultsTicket)
{

    std::shared_ptr<ShotRepository> shotRepository;
    convert<STI::TNetwork::TShotRepository_var, std::shared_ptr<ShotRepository>>(tResultTicket.repo, shotRepository);

    resultsTicket = std::make_shared<ResultTicket>(convert<TShotID, ShotID>(
                        tResultTicket.sid), 
                        shotRepository, 
                        convert<TTicketStatus, Ticket::TicketStatus>(tResultTicket.status));

    return (resultsTicket != 0);
}

template<>
bool STI::Network::convert<std::shared_ptr<ResultTicket>, TResultTicket>(
        const std::shared_ptr<ResultTicket>& resultsTicket, TResultTicket& tResultTicket)
{
    if (resultsTicket == 0) return false;
    
    std::shared_ptr<ShotRepository> shotRepository;

    if (resultsTicket->getShotRepository(shotRepository)) {
        TShotRepository_var tShotRepo;
        bool success = convert<std::shared_ptr<ShotRepository>, TShotRepository_var>(shotRepository, tShotRepo);
        tResultTicket.repo = tShotRepo;
        tResultTicket.hasRepo = success;
    }
    else {
        tResultTicket.hasRepo = false;
    }

    tResultTicket.sid = convert<ShotID, TShotID>(resultsTicket->getShotID());
    tResultTicket.status = convert<Ticket::TicketStatus, TTicketStatus>(resultsTicket->getStatus());

    return true;
}


//ShotRepository
template<>
bool STI::Network::convert<TShotRepository_var, std::shared_ptr<ShotRepository>>(
        const TShotRepository_var& tShotRepository, std::shared_ptr<ShotRepository>& shotRepository)
{
    shotRepository = std::make_shared<RemoteShotRepository>(tShotRepository);
    return (shotRepository != 0);
}

template<>
bool STI::Network::convert<std::shared_ptr<ShotRepository>, TShotRepository_var>(
        const std::shared_ptr<ShotRepository>& shotRepository, TShotRepository_var& tShotRepository)
{
    return NetworkShotRepositoryWrapper::getTShotRepositoryReference(shotRepository, tShotRepository);
}


//TicketStatus
template<>
Ticket::TicketStatus STI::Network::convert<TTicketStatus, Ticket::TicketStatus>(const TTicketStatus& tTicketStatus)
{
    Ticket::TicketStatus status;

    switch(tTicketStatus) {
        case TTicketStatus::TicketRunning:
			status = Ticket::TicketStatus::Running;
			break;
        case TTicketStatus::TicketComplete:
			status = Ticket::TicketStatus::Complete;
			break;
        case TTicketStatus::TicketCanceled:
			status = Ticket::TicketStatus::Canceled;
			break;
        case TTicketStatus::TicketNotFound:
			status = Ticket::TicketStatus::NotFound;
			break;
        default:
            status = Ticket::TicketStatus::NotFound;
            break;
    }

    return status;
}

template<>
TTicketStatus STI::Network::convert<Ticket::TicketStatus, TTicketStatus>(const Ticket::TicketStatus& ticketStatus)
{
    TTicketStatus tStatus;
    
    switch(ticketStatus) {
        case Ticket::TicketStatus::Running:
			tStatus = TTicketStatus::TicketRunning;
			break;
        case Ticket::TicketStatus::Complete:
			tStatus = TTicketStatus::TicketComplete;
			break;
        case Ticket::TicketStatus::Cancelled:
			tStatus = TTicketStatus::TicketCancelled;
			break;
        case Ticket::TicketStatus::NotFound:
			tStatus = TTicketStatus::TicketNotFound;
			break;
        default:
            tStatus = TTicketStatus::TicketNotFound;
            break;
    }

    return tStatus;
}

