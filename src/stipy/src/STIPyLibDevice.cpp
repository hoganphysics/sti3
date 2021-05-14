
#include "STIPyLibDevice.h"
#include "DeviceID.h"
#include "DeviceCollection.h"
#include "ParseID.h"
#include "ParseTicketManager.h"
#include "DeviceMessageReceiver.h"
#include "ParseTicket.h"

#include "ResultTicketManager.h"
#include <memory>
#include <iostream>

using STI::Python::STIPyLibDevice;
using STI::Python::ParseTicket;
using STI::Device::LocalDevice;
using STI::Python::ResultTicket;
using STI::Python::ParseTicketManager;
using STI::Python::ResultTicketManager;
using STI::Device::DeviceMessageReceiver;
using STI::Device::DeviceMessageListener;
using STI::Device::EngineSchedulerMessage;
using STI::Device::DeviceMessageListenerID;


STIPyLibDevice::STIPyLibDevice(const std::string& name, const std::string& address, unsigned short module,
		const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID)
: LocalDevice(name, address, module, serverID.getID()), serverID(serverID), serverHubID(serverHubID)
{
    addPartner(serverID);

    parseTicketManager = std::make_shared<ParseTicketManager>();
    resultTicketManager = std::make_shared<ResultTicketManager>();

    // std::shared_ptr<Device> server;
    // getServer(server);

    std::shared_ptr<DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);

    //EventEngineScheduler message listener
    auto listener = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(parseTicketManager);
    
   	schedulerMessageLID.name = getID().getID() + "::EventEngineScheduler::ParseResult";
	schedulerMessageLID.type = STI::Device::DeviceMessageType::EngineScheduler;
	
    auto listener2 = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(resultTicketManager);
    
   	schedulerMessageLID2.name = getID().getID() + "::EventEngineScheduler::ResultTicket";
	schedulerMessageLID2.type = STI::Device::DeviceMessageType::EngineScheduler;

    if (receiver != 0) {
        receiver->addListener(serverID, schedulerMessageLID, listener);	//listen to events from server
        receiver->addListener(serverID, schedulerMessageLID2, listener2);	//listen to events from server
    }

    // std::cout << "STIPyLibDevice connecting"<<std::endl;
}

STIPyLibDevice::~STIPyLibDevice()
{
    std::shared_ptr<DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);

    //Should make this RAII -- the listener class should call removeListener on destruction.
    //Could make a mixin class that remembers the listenerID and automatically calls remove
    if (receiver != 0) {
        receiver->removeListener(serverID, schedulerMessageLID);
    }
}

bool STIPyLibDevice::addto(const STI::Network::HubID& target)
{
    return serverHubID == target;
}

bool STIPyLibDevice::getServer(std::shared_ptr<Device>& server)
{
    std::shared_ptr<STI::Device::DeviceCollection> deviceCollection;
    getCollection(deviceCollection);
    
    //std::cout << "Collection: " << deviceCollection->size() << std::endl;

    return (deviceCollection != 0) && deviceCollection->get(serverID, server) && (server != 0);
}


std::shared_ptr<ParseTicket> STIPyLibDevice::makeParseTicket(const STI::Engine::ParseID& pid)
{
    std::shared_ptr<Device> server;
    bool connected = getServer(server);

    auto ticket = parseTicketManager->makeTicket(pid, server);
    
    if (!connected && ticket != 0) {
        ticket->cancel();
    }
    return ticket;
}


std::shared_ptr<ResultTicket> STIPyLibDevice::makeResultTicket(const STI::Engine::ShotID& sid)
{
    std::shared_ptr<Device> server;
    bool connected = getServer(server);

    auto ticket = resultTicketManager->makeTicket(sid, server);
    
    if (!connected && ticket != 0) {
        ticket->cancel();
    }
    return ticket;
}