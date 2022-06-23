
#ifndef STI_NETWORK_CONVERT_RESULTTICKET_H
#define STI_NETWORK_CONVERT_RESULTTICKET_H

#include <sti/engine/Ticket.h>
#include "NetworkConvert.h"
#include "deviceNet.h"


#include <memory>

namespace STI
{

namespace Engine
{

class Ticket;
class ResultTicket;
class ShotRepository;

} //Engine


//ResultTicket
template<>
bool Network::convert<TNetwork::TResultTicket, std::shared_ptr<Engine::ResultTicket>>(
        const TNetwork::TResultTicket& tResultTicket, std::shared_ptr<Engine::ResultTicket>& resultTicket);
template<>
bool Network::convert<std::shared_ptr<Engine::ResultTicket>, TNetwork::TResultTicket>(
        const std::shared_ptr<Engine::ResultTicket>& resultTicket, TNetwork::TResultTicket& tResultTicket);


// //ShotRepository
// template<>
// bool Network::convert<TNetwork::TShotRepository_var, std::shared_ptr<Engine::ShotRepository>>(
//         const TNetwork::TShotRepository_var& tShotRepository, std::shared_ptr<Engine::ShotRepository>& shotRepository);
// template<>
// bool Network::convert<std::shared_ptr<Engine::ShotRepository>, TNetwork::TShotRepository_var>(
//         const std::shared_ptr<Engine::ShotRepository>& shotRepository, TNetwork::TShotRepository_var& tShotRepository);


//TicketStatus
template<>
Engine::Ticket::TicketStatus Network::convert<TNetwork::TTicketStatus, Engine::Ticket::TicketStatus>(
        const TNetwork::TTicketStatus& tTicketStatus);
template<>
TNetwork::TTicketStatus Network::convert<Engine::Ticket::TicketStatus, TNetwork::TTicketStatus>(
        const Engine::Ticket::TicketStatus& ticketStatus);

} //STI

#endif

