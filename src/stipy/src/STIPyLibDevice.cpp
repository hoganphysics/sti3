
#include "STIPyLibDevice.h"
#include "DeviceID.h"
#include "DeviceCollection.h"
#include "ParseID.h"
#include "PyParseTicketManager.h"
#include "DeviceMessageReceiver.h"
#include "PyParseTicket.h"
#include "DeviceMessage.h"
#include "PyResultTicket.h"
#include "PyResultTicketManager.h"

#include <memory>
#include <iostream>

using STI::Python::STIPyLibDevice;
using STI::Python::PyParseTicket;
using STI::Device::LocalDevice;
using STI::Python::PyResultTicket;
using STI::Python::PyParseTicketManager;
using STI::Python::PyResultTicketManager;
using STI::Device::DeviceMessageReceiver;
using STI::Device::DeviceMessageListener;
using STI::Device::EngineSchedulerMessage;
using STI::Device::DeviceMessageListenerID;


STIPyLibDevice::STIPyLibDevice(const std::string& name, const std::string& address, unsigned short module,
		const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID)
: LocalDevice(name, address, module, serverID.getID()), serverID(serverID), serverHubID(serverHubID)
{
    addPartner(serverID);

    parseTicketManager = std::make_shared<PyParseTicketManager>();
    resultTicketManager = std::make_shared<PyResultTicketManager>();

    std::shared_ptr<DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);

    //EventEngineScheduler message listener
    engineMessageListener = std::make_shared<STIPyLibDevice::TicketManagerListener>(parseTicketManager, resultTicketManager);
    auto listener = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(engineMessageListener);
    
   	schedulerMessageLID.name = getID().getID() + "::EventEngineScheduler::TicketManagers";
	schedulerMessageLID.type = STI::Device::DeviceMessageType::EngineScheduler;
	
    if (receiver != 0) {
        receiver->addListener(serverID, schedulerMessageLID, listener);	//listen to engine events from server
    }
}

STIPyLibDevice::~STIPyLibDevice()
{
    std::cout << "~STIPyLibDevice()" << std::endl;

    std::shared_ptr<DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);

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

    return (deviceCollection != 0) && deviceCollection->get(serverID, server) && (server != 0);
}


std::shared_ptr<PyParseTicket> STIPyLibDevice::makeParseTicket(const STI::Engine::ParseID& pid)
{
    std::shared_ptr<Device> server;
    bool connected = getServer(server);

    auto ticket = parseTicketManager->makeTicket(pid, server);
    
    if (!connected && ticket != 0) {
        ticket->cancel();
    }
    return ticket;
}


std::shared_ptr<PyResultTicket> STIPyLibDevice::makeResultTicket(const STI::Engine::ShotID& sid)
{
    std::shared_ptr<Device> server;
    bool connected = getServer(server);

    auto ticket = resultTicketManager->makeTicket(sid, server);
    
    if (!connected && ticket != 0) {
        ticket->cancel();
    }
    return ticket;
}


void STIPyLibDevice::TicketManagerListener::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    if (mess == 0) return;

    typedef STI::Device::EngineSchedulerMessage::SchedulerMessageType MessageSubtype;

    switch (mess->schedulerMessageType)
    {
    case MessageSubtype::ParseComplete:
        parseTicketManager->handleMessage(mess);
        break;
    case MessageSubtype::PlayComplete:
        resultTicketManager->handleMessage(mess);
    default:
        break;
    }
}
