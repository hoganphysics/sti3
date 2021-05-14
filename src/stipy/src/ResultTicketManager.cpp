
#include "ResultTicketManager.h"
#include "ResultTicket.h"
#include "ShotID.h"
#include "DeviceMessage.h"

#include <iostream>

using STI::Python::ResultTicketManager;
using STI::Python::ResultTicket;
using STI::Engine::ShotID;
using STI::Device::EngineSchedulerMessage;

ResultTicketManager::ResultTicketManager()
{
}

ResultTicketManager::~ResultTicketManager()
{
}

void ResultTicketManager::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    std::shared_ptr<ResultTicket> ticket;
    
    std::cout << "ResultTicketManager::handleMessage" << std::endl;

    const auto& id = mess->jobID.sid;
    if (!get(id, ticket)) {
        return;
    }

    //handle message

    typedef EngineSchedulerMessage::SchedulerMessageType MessageType;
    if (mess->schedulerMessageType == MessageType::PlayComplete) {
        ticket->setComplete();
    }
    else {      //todo -- if cancel
        ticket->cancel();
    }

    remove(id);  //avoid storing ticket indefinitely (memory leak)
}
