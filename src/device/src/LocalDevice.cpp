
#include "LocalDevice.h"
#include "LocalDeviceMessageDispatcher.h"
#include "DeviceMessageReceiver.h"
#include "LocalEventEngineScheduler.h"
#include "DeviceMessageListener.h"
#include "DeviceMessage.h"
#include "LocalEventEngineFactory.h"

#include "LocalFileHolder.h"

#include "MixedValue.h"
#include "LocalChannelManager.h"
#include "LocalChannel.h"

#include "LocalAttribute.h"
#include "LocalAttributeManager.h"

#include "DeviceMessageListenerForwarder.h"

#include "ServerMessageRelayer.h"
#include "LocalPersistenceManager.h"
//#include "SerializedRepository.h"

#include "LocalShot.h"
#include "ParseTicket.h"

#include "ShotRepository.h"

#include "Measurement.h"
#include "Configuration.h"

#include <memory>
#include <iostream>

using STI::Device::Device;
using STI::Device::DeviceID;
using STI::Device::LocalDevice;
using STI::Device::DeviceMessageDispatcher;
using STI::Device::LocalDeviceMessageDispatcher;
using STI::Device::DeviceMessageReceiver;
using STI::Engine::LocalEventEngineScheduler;
using STI::Device::LocalChannelManager;
using STI::Device::LocalChannel;
using STI::Device::ChannelManager;
using STI::Device::AttributeManager;
using STI::Device::DeviceCollectionPolicy;
using STI::Device::LocalAttribute;
using STI::Device::DeviceMessageListener;
using STI::Device::EngineSchedulerMessage;
using STI::Device::DeviceMessageListenerID;
using STI::Device::CollectionUpdateMessage;
using STI::Engine::LocalEventEngineFactory;
//using STI::Engine::SerializedRepository;
using STI::Engine::ParseID;
using STI::Engine::ShotID;
using STI::Device::Configuration;


LocalDevice::LocalDevice(const Configuration& config, const std::string& section)
: LocalDevice(
	config.get<std::string>(section, "Device Name", ""), 
	config.get<std::string>(section, "IP Address", ""), 
	config.get<unsigned short>(section, "Module", 0),
	config.get<std::string>(section, "Target Server", ""))
{
}

LocalDevice::LocalDevice(const std::string& name, const std::string& address, unsigned short module,
	const std::string& targetServer) : id(name, address, module, targetServer)
{
	std::shared_ptr<DeviceCollectionPolicy> policy = std::make_shared<DeviceCollectionPolicy>(this);;
	localCollection = std::make_shared<STI::Utils::LocalCollection<DeviceID, Device>>(policy);

	deviceMessageDispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
	deviceMessageReceiver = std::make_shared<DeviceMessageReceiver>(id, localCollection, deviceMessageDispatcher);

	auto deviceCollectionListener = std::make_shared<STI::Device::LocalDevice::DeviceCollectionListener>(this);
	localCollection->addListener(deviceCollectionListener);

	auto basePath = LocalPersistenceManager::makeBasePath(".sti", getID());

	localChannelManager = std::make_shared<LocalChannelManager>(this, deviceMessageDispatcher);
	localAttributeManager = std::make_shared<LocalAttributeManager>(id, deviceMessageDispatcher);
	
	
	//localSerializedRepository = std::make_shared<SerializedRepository>(basePath);

	auto localFileHolderFactory = std::make_shared<STI::Utils::LocalFileHolderFactory>();
	localPersistenceManager = std::make_shared<LocalPersistenceManager>(getID(), basePath, localFileHolderFactory, localCollection);

	// localPersistenceManager->setFileHolderFactory(localFileHolderFactory);


    auto engineFactory = std::make_shared<LocalEventEngineFactory>(getID(), localChannelManager, localAttributeManager, deviceMessageDispatcher, 
																	localCollection, localPersistenceManager);
	eventEngineScheduler = std::make_shared<LocalEventEngineScheduler>(this, engineFactory, deviceMessageDispatcher);





	//setEngineFactory(engineFactory);

	listenerForwarder = std::make_shared<STI::Device::DeviceMessageListenerForwarder>(this);

	//EngineSchedulerMessage ListenerID
	schedulerMessageLID.name = getID().getID() + "::EventEngineScheduler";
	schedulerMessageLID.type = STI::Device::DeviceMessageType::EngineScheduler;
	messageListenerIDs.push_back(schedulerMessageLID);

	//CollectionMessage ListenerID
	collectionMessageLID.name = getID().getID() + "::DeviceCollection";
	collectionMessageLID.type = STI::Device::DeviceMessageType::CollectionUpdate;
	messageListenerIDs.push_back(collectionMessageLID);

	serverMessageRelayer = std::make_shared<STI::Device::ServerMessageRelayer>(getID(), deviceMessageDispatcher);

	serverMessageRelayer->addFilter<STI::Device::EngineJobUpdateDeviceMessage>( 
		[](const std::shared_ptr<STI::Device::EngineJobUpdateDeviceMessage>& message)->bool {
			//only relay job messages that originate from the job owner (avoids duplicates)
			return message->getEngineJob()->getJobOwner() == message->originalSourceID();
		});

	// serverMessageRelayer->addFilter<STI::Device::EngineStateMessage>( 
	// 	[](const std::shared_ptr<STI::Device::EngineStateMessage>& message)->bool {
	// 		//only relay job messages that originate from the job owner (avoids duplicates)
	// 		return true;
	// 	});


	// serverMessageRelayer->test<STI::Device::EngineJobUpdateDeviceMessage>( 
	// 	[](const std::shared_ptr<STI::Device::EngineJobUpdateDeviceMessage>& message)->bool {
	// 		return true;
	// 	});

	// auto listenerTest = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(r1);
	// deviceMessageReceiver->addListener(id, schedulerMessageLID, listenerTest);

	// auto listenerTest2 = std::static_pointer_cast<DeviceMessageListener<STI::Device::CollectionUpdateMessage>>(r1);
	// deviceMessageReceiver->addListener(id, schedulerMessageLID, listenerTest2);

	STI::Engine::EngineID id0(0);
	addEventEngine(id0);


	parseTicketManager = std::make_shared<STI::Engine::ParseTicketManager<>>(eventEngineScheduler);
	resultTicketManager = std::make_shared<STI::Engine::ResultTicketManager<>>(localPersistenceManager, eventEngineScheduler);
	// DeviceMessageListenerID lid(DeviceMessageType::EngineScheduler, "");

	// deviceMessageReceiver->addListener(id, DeviceMessageListenerID(DeviceMessageType::EngineScheduler, ""), 
	// 	std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(parseTicketManager));
	// deviceMessageReceiver->addListener(id, DeviceMessageListenerID(DeviceMessageType::EngineScheduler, ""), 
	// 	std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(resultTicketManager));


	deviceMessageReceiver->addListener<EngineSchedulerMessage>(getID(), "ParseTicketManager", parseTicketManager);
	deviceMessageReceiver->addListener<EngineSchedulerMessage>(getID(), "ResultTicketManager", resultTicketManager);



}

LocalDevice::~LocalDevice()
{
	// std::cout << "~LocalDevice()" << std::endl;
	localCollection->clear();
}


void LocalDevice::disable()
{
	if (deviceMessageReceiver != 0) {
		deviceMessageReceiver->clearListeners();
	}

	if (localCollection != 0) {
		localCollection->clearListeners();
	}
}


void LocalDevice::addEventTarget(const STI::Device::DeviceID& id)
{
	eventTargets.insert(id);
}

bool LocalDevice::isEventTarget(const DeviceID& id)
{
	auto it = eventTargets.find(id);
	return (it != eventTargets.end());
}

//true if the LocalDevice is the target server of ID (i.e., this device is acting as a server)
bool LocalDevice::isTargetServerOf(const DeviceID& id)
{
	return id.getTargetServerID() == getID().getID();
}

void LocalDevice::sendMessage(const std::shared_ptr<DeviceMessage>& mess)
{
	if (deviceMessageDispatcher != 0) {
		deviceMessageDispatcher->addMessage(mess);
	}
}

//LocalDeviceCollection event handler
void LocalDevice::DeviceCollectionListener::add(const DeviceID& id)
{
	std::cout << "++++ DeviceCollectionListener::add( " << id.getID() << " )" << std::endl;

	// Listen to EngineScheduler messages from:
	// 1) Declared event targets and 2) any device that has this device as a target server.

	// if (localDevice->isTargetServerOf(id)) {
	// 	localDevice->addEventTarget(id);
	// }

	if ( localDevice->isTargetServerOf(id) ) {
		STI::Device::ServerMessageRelayer::addAllListeners(localDevice->deviceMessageReceiver, id, localDevice->serverMessageRelayer);
	}


	if( localDevice->isEventTarget(id) || localDevice->isTargetServerOf(id)) {
		
		//auto listener = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(localDevice->eventEngineScheduler);
		auto listener = localDevice->eventEngineScheduler->getMessageListener();
		
		localDevice->deviceMessageReceiver->addListener(id, localDevice->schedulerMessageLID, listener);	//listen to events on new device 'id'
	}
	
	//The DeviceMessageListenerForwarder allows newly added devices to register message listeners via the
	//local DeviceMessageReceiver, allowing update messages to be passed to the stored device instance.
	std::shared_ptr<Device> newDevice;
	if (localDevice->localCollection->get(id, newDevice) && newDevice != 0) {
		newDevice->attachMessageListenerForwarder(localDevice->listenerForwarder);
	}

	auto mess = std::make_shared<CollectionUpdateMessage>(localDevice->getID());
//	auto mess = CollectionUpdateMessage::makeMessage(localDevice->getID());
	localDevice->sendMessage(mess);

}

//LocalDeviceCollection event handler
void LocalDevice::DeviceCollectionListener::remove(const DeviceID& id)
{
	auto mess = std::make_shared<CollectionUpdateMessage>(localDevice->getID());
//	auto mess = CollectionUpdateMessage::makeMessage(localDevice->getID());
	localDevice->sendMessage(mess);

	if ( localDevice->isTargetServerOf(id) ) {
		STI::Device::ServerMessageRelayer::removeAllListeners(localDevice->deviceMessageReceiver, id, localDevice->serverMessageRelayer);
	}
	// std::cout << "DeviceCollectionListener::remove" << std::endl;

	localDevice->deviceMessageReceiver->removeListener(id, localDevice->schedulerMessageLID);

	std::cout << "---- DeviceCollectionListener::remove( " << id.getID() << " )" << std::endl;


}

const DeviceID LocalDevice::getID() const
{
	return id;
}


bool LocalDevice::write(short channel, const STI::Utils::MixedValue& value)
{
	return writeChannel(channel, value);
}


bool LocalDevice::read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
	return readChannel(channel, value, data);
}

void LocalDevice::stopRW()
{
	auto pids = parseTicketManager->getIDs();

	auto pit = std::find_if(pids.begin(), pids.end(), [](const ParseID& pid){ return false; });

	if (pit != pids.end()) {
		parseTicketManager->cancel(*pit);
	}

	auto sids = resultTicketManager->getIDs();	//ShotIDs

	auto sit = std::find_if(sids.begin(), sids.end(), [](const ShotID& sid){ return false; });

	if (sit != sids.end()) {
		resultTicketManager->cancel(*sit);
	}
}

bool LocalDevice::playSingleEvent(const STI::Engine::RawEvent& event, std::shared_ptr<STI::Engine::ResultTicket>& resultTicket)
{
	std::unique_lock<std::mutex> playLock(deviceMutex);

	STI::Engine::ShotConfig shotConfig;
	shotConfig.targetEnginePool = 0;	//0=async pool
	shotConfig.shotType = STI::Engine::ShotType::SingleUndocumented;
	shotConfig.jobSourceID.user = "<async play>";
	shotConfig.jobSourceID.machine = getID().getAddress();
	
	auto shot = std::make_shared<STI::Engine::LocalShot>(shotConfig);
	auto events = std::make_shared<std::vector<STI::Engine::RawEvent>>();

	events->push_back(event);
	shot->setEvents(events);

	auto parseID = eventEngineScheduler->parse(shot);

	auto parseTicket = parseTicketManager->makeTicket(parseID);
	parseTicket->wait();

	if (parseTicket->getStatus() != STI::Engine::Ticket::TicketStatus::Complete) {
		return false;
	}

	auto sid = eventEngineScheduler->play(parseID, shotConfig.jobSourceID);

	resultTicket = resultTicketManager->makeTicket(sid);
	resultTicket->wait();

	if (resultTicket->getStatus() != STI::Engine::Ticket::TicketStatus::Complete) {
		return false;
	}

	return true;
}

bool LocalDevice::writeChannelDefault(short channel, const STI::Utils::MixedValue& value)
{

	double eventTime = 100;
	STI::Engine::RawEvent evt0(getID(), eventTime, channel, value, "writeChannelDefault", 0, STI::Engine::RawEventType::Play);
	std::shared_ptr<STI::Engine::ResultTicket> resultTicket;

	if (!playSingleEvent(evt0, resultTicket))
		return false;

	return true;
}

bool LocalDevice::readChannelDefault(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{

	double eventTime = 100;
	STI::Engine::RawEvent evt0(getID(), eventTime, channel, value, "readChannelDefault", 0, STI::Engine::RawEventType::Measurement);
	std::shared_ptr<STI::Engine::ResultTicket> resultTicket;

	if (!playSingleEvent(evt0, resultTicket))
		return false;

	auto measurements = resultTicket->measurements(getID());
	
	if (measurements.size() > 0) {
		measurements.at(0)->extractMeasurementResult(data);
		return true;
	}

	return false;
}


void LocalDevice::addEventEngine(const STI::Engine::EngineID& engineID)
{
//	auto engine = eventEngineFactory->createEngine(getID(), localChannels, this, deviceMessageDispatcher, localCollection);

	if (eventEngineScheduler != 0) {
		eventEngineScheduler->addEngine(engineID, this);		
	}

}


LocalChannel& LocalDevice::addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName)
{
	auto channel = std::make_shared<LocalChannel>(channelNumber, type, inputType, outputType, defaultName);
	
	localChannelManager->addChannel(channel);

	return *channel;
}


void LocalDevice::addAttribute(const std::string& key, const std::string& initialValue, 
								std::shared_ptr<STI::Device::LocalAttribute>& attribute)
{
    attribute = std::make_shared<STI::Device::LocalAttribute>(key, initialValue);
	localAttributeManager->addAttribute(attribute);
}

LocalAttribute& LocalDevice::addAttribute(const std::string& key, const std::string& initialValue)
{
	std::shared_ptr<STI::Device::LocalAttribute> attribute;
	addAttribute(key, initialValue, attribute);
	return *attribute;
}

void LocalDevice::addAttribute(const std::string& key, const std::string& initialValue, std::vector<std::string> allowedValues, 
								std::shared_ptr<STI::Device::LocalAttribute>& attribute)
{
    attribute = std::make_shared<STI::Device::LocalAttribute>(key, initialValue, allowedValues);
	localAttributeManager->addAttribute(attribute);
}

LocalAttribute& LocalDevice::addAttribute(const std::string& key, const std::string& initialValue, std::vector<std::string> allowedValues)
{
	std::shared_ptr<STI::Device::LocalAttribute> attribute;
	addAttribute(key, initialValue, allowedValues, attribute);
	return *attribute;
}


void LocalDevice::getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection)
{
	collection = localCollection;
}

void LocalDevice::getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
{
	dispatcher = deviceMessageDispatcher;
}

void LocalDevice::getMessageReceiver(std::shared_ptr<DeviceMessageReceiver>& receiver)
{
	receiver = deviceMessageReceiver;
}

bool LocalDevice::getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
{
	scheduler = eventEngineScheduler;
	return scheduler != 0;
}

bool LocalDevice::getEngineScheduler(std::shared_ptr<STI::Engine::LocalEventEngineScheduler>& scheduler)
{
	scheduler = eventEngineScheduler;
	return scheduler != 0;
}

void LocalDevice::getChannelManager(std::shared_ptr<ChannelManager>& manager)
{
	manager = localChannelManager;
}

void LocalDevice::getAttributeManager(std::shared_ptr<AttributeManager>& manager)
{
	manager = localAttributeManager;	
}

bool LocalDevice::getPersistenceManager(std::shared_ptr<PersistenceManager>& manager)
{
	manager = localPersistenceManager;
	return manager != 0;
}

bool LocalDevice::isPartnerDevice(const DeviceID& id)
{
	auto it = partnerDevices.find(id);

	return it != partnerDevices.end();
}

bool DeviceCollectionPolicy::include(const STI::Device::DeviceID& key) const 
{
	return device->isPartnerDevice(key) || device->isTargetServerOf(key);
}
