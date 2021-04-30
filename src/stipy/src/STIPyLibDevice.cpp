
#include "STIPyLibDevice.h"
#include "DeviceID.h"
#include "DeviceCollection.h"
#include "ParseID.h"
#include "ParseTicketManager.h"
#include "DeviceMessageReceiver.h"
#include "ParseTicket.h"

#include <memory>
#include <iostream>

using STI::Python::STIPyLibDevice;
using STI::Python::ParseTicket;
using STI::Device::LocalDevice;
using STI::Python::ParseTicketManager;
using STI::Device::DeviceMessageReceiver;
using STI::Device::DeviceMessageListener;
using STI::Device::EngineSchedulerMessage;
using STI::Device::DeviceMessageListenerID;

STIPyLibDevice::STIPyLibDevice(const std::string& name, const std::string& address, unsigned short module,
		const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID)
: LocalDevice(name, address, module, serverID.getID()), serverID(serverID), serverHubID(serverHubID)
{
    addPartner(serverID);

    ticketManager = std::make_shared<ParseTicketManager>();

    std::shared_ptr<Device> server;
    getServer(server);

    std::shared_ptr<DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);

    //EventEngineScheduler message listener
    auto listener = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(ticketManager);
    DeviceMessageListenerID schedulerMessageLID;
   	schedulerMessageLID.name = getID().getID() + "::EventEngineScheduler";
	schedulerMessageLID.type = STI::Device::DeviceMessageType::EngineScheduler;
	
    receiver->addListener(server->getID(), schedulerMessageLID, listener);	//listen to events from server
}

bool STIPyLibDevice::addto(const STI::Network::HubID& target)
{
    return serverHubID == target;
}

bool STIPyLibDevice::getServer(std::shared_ptr<Device>& server)
{
    std::shared_ptr<STI::Device::DeviceCollection> deviceCollection;
    getCollection(deviceCollection);
    
    std::cout << "Collection: " << deviceCollection->size() << std::endl;

    return (deviceCollection != 0) && deviceCollection->get(serverID, server);
}


std::shared_ptr<ParseTicket> STIPyLibDevice::makeParseTicket(const STI::Engine::ParseID& pid)
{
    return ticketManager->makeParseTicket(pid);
}

