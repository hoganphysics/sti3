
#include "STIPyLibDevice.h"
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceCollection.h>
#include <sti/engine/ParseID.h>
#include "PyParseTicketManager.h"
#include <sti/device/DeviceMessageReceiver.h>
#include "PyParseTicket.h"
#include <sti/device/DeviceMessage.h>
#include "PyResultTicket.h"
#include "PyResultTicketManager.h"
#include <sti/device/PersistenceManager.h>

#include <memory>
// #include <iostream>

#include <pybind11/pybind11.h>


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
: LocalDevice(name, address, module, serverID.getID()), serverID(serverID), serverHubID(serverHubID), connected(false)
{
    addPartner(serverID);

    std::shared_ptr<DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);
    
    if (receiver != 0) {
        
        receiver->addListener<STI::Device::CollectionUpdateMessage>(getID(), "localCollectionUpdateListener",
            [this](const std::shared_ptr<STI::Device::CollectionUpdateMessage>& message)
            {
                // std::cout << "Collection update message: " << message->sourceID().getID() << std::endl;
                connectToServer();
            }
        );
    }

}

STIPyLibDevice::~STIPyLibDevice()
{
    std::shared_ptr<DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);

    if (receiver != 0) {
        receiver->removeListener(serverID, schedulerMessageLID);
    }
}

void STIPyLibDevice::connectToServer()
{
    std::unique_lock<std::mutex> statusLock(connectionMutex);

    std::shared_ptr<DeviceMessageReceiver> receiver;
    getMessageReceiver(receiver);

    std::shared_ptr<Device> server;
    connected = getServer(server);
    
    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
    std::shared_ptr<STI::Engine::EventEngineScheduler> eventEngineScheduler;
    if (connected && server != 0) {
        server->getPersistenceManager(persistenceManager);
        server->getEngineScheduler(eventEngineScheduler);
    }
    else {
        connected = false;
    }

    parseTicketManager = std::make_shared<PyParseTicketManager>(eventEngineScheduler);
    resultTicketManager = std::make_shared<PyResultTicketManager>(persistenceManager, eventEngineScheduler);

    //EventEngineScheduler message listener
    engineMessageListener = std::make_shared<STIPyLibDevice::TicketManagerListener>(parseTicketManager, resultTicketManager);
    //auto listener = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(engineMessageListener);
    
   	schedulerMessageLID.name = getID().getID() + "::EventEngineScheduler::TicketManagers";
	schedulerMessageLID.type = STI::Device::DeviceMessageType::EngineScheduler;
	
    if (receiver != 0) {
        receiver->addListener<EngineSchedulerMessage>(serverID, schedulerMessageLID, engineMessageListener);	//listen to engine events from server
    }

    connectionCondition.notify_all();

}

void STIPyLibDevice::waitForConnection()
{
    std::unique_lock<std::mutex> connectionLock(connectionMutex);

    bool keepWaiting = true;

    while (!connected && keepWaiting) {
        connectionCondition.wait_for(connectionLock, std::chrono::milliseconds(100));

        if (PyErr_CheckSignals() != 0) 
            throw pybind11::error_already_set();
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
    if (parseTicketManager != 0) {
        auto ticket = parseTicketManager->makeTicket(pid);
        return ticket;        
    }

    std::shared_ptr<STI::Engine::EventEngineScheduler> eventEngineScheduler;
    auto badTicket = std::make_shared<PyParseTicket>(pid, eventEngineScheduler);
    badTicket->cancel();
    return badTicket;
}


std::shared_ptr<PyResultTicket> STIPyLibDevice::makeResultTicket(const STI::Engine::ShotID& sid)
{
    if (resultTicketManager != 0) {
        auto ticket = resultTicketManager->makeTicket(sid);
        return ticket;        
    }

    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
    auto badTicket = std::make_shared<PyResultTicket>(sid, persistenceManager);
    badTicket->cancel();
    return badTicket;
}


void STIPyLibDevice::TicketManagerListener::handleMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& mess)
{
    if (mess == 0 || parseTicketManager == 0 || resultTicketManager == 0) return;

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
