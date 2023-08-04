#include <sti/LocalDevice.h>

#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceMessageReceiver.h>
#include <sti/device/LocalAttribute.h>
#include <sti/device/LocalChannel.h>
#include <sti/device/ServerMessageRelayer.h>

#include <sti/engine/Measurement.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/ParseTicket.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEventGroup.h>

#include <sti/utils/Configuration.h>
#include <sti/utils/LocalFileHolder.h>
#include <sti/utils/MixedValue.h>

#include "DeviceMessageListenerForwarder.h"
#include "LocalAttributeManager.h"
#include "LocalChannelManager.h"
#include "LocalDeviceMessageDispatcher.h"
#include "LocalEventEngineFactory.h"
#include "LocalEventEngineScheduler.h"
#include "LocalPersistenceManager.h"
#include "LocalShot.h"
#include "ShotRepository.h"

#include <filesystem>
#include <memory>
#include <iostream>

using STI::Device::AttributeManager;
using STI::Device::ChannelManager;
using STI::Device::CollectionUpdateMessage;
using STI::Device::Device;
using STI::Device::DeviceCollectionPolicy;
using STI::Device::DeviceID;
using STI::Device::DeviceMessageDispatcher;
using STI::Device::DeviceMessageListener;
using STI::Device::DeviceMessageListenerID;
using STI::Device::DeviceMessageReceiver;
using STI::Device::EngineSchedulerMessage;
using STI::Device::LocalAttribute;
using STI::Device::LocalChannel;
using STI::Device::LocalChannelManager;
using STI::Device::LocalDevice;
using STI::Device::LocalDeviceMessageDispatcher;
using STI::Device::PartnerDevice;

using STI::Engine::LocalEventEngineFactory;
using STI::Engine::LocalEventEngineScheduler;
using STI::Engine::ParseID;
using STI::Engine::ShotID;

using STI::Utils::Configuration;


LocalDevice::LocalDevice(const std::map<std::string, std::string>& config)
: LocalDevice( Configuration(config) )
{
}

LocalDevice::LocalDevice(const Configuration& config, const std::string& section)
: LocalDevice(
	config.get<std::string>(section, "Device Name", ""), 
	config.get<std::string>(section, "IP Address", ""), 
	config.get<unsigned short>(section, "Module", 0),
	config.get<std::string>(section, "Target Server", ""))
{
}

LocalDevice::LocalDevice(const std::string& name, const std::string& address, unsigned short module,
	const std::string& targetServer) 
: STI::Engine::DeviceEventParser(), id(name, address, module, targetServer)
{
	std::shared_ptr<DeviceCollectionPolicy> policy = std::make_shared<DeviceCollectionPolicy>(this);;
	localCollection = std::make_shared<STI::Utils::LocalCollection<DeviceID, Device>>(policy);

	deviceMessageDispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
	deviceMessageReceiver = std::make_shared<DeviceMessageReceiver>(id, localCollection, deviceMessageDispatcher);

	auto deviceCollectionListener = std::make_shared<STI::Device::LocalDevice::DeviceCollectionListener>(this);
	addCollectionListener(deviceCollectionListener);
	// localCollection->addListener(deviceCollectionListener);

    // std::cout << "CWD: " << std::filesystem::current_path().c_str() << std::endl;
	auto deviceRootPath = std::filesystem::current_path();	//cwd
	deviceRootPath /= ".sti";

	auto basePath = LocalPersistenceManager::makeBasePath(deviceRootPath.generic_string(), getID());

	localChannelManager = std::make_shared<LocalChannelManager>(this, deviceMessageDispatcher);
	localAttributeManager = std::make_shared<LocalAttributeManager>(id, deviceMessageDispatcher);
	
	
	//localSerializedRepository = std::make_shared<SerializedRepository>(basePath);

	//temp for localPersistenceManager; TODO: expose to constructor
	Configuration configuration;
	configuration.set<int>("PersistenceManager", "resultBufferSize", 5);
	configuration.set<int>("PersistenceManager", "sequenceBufferSize", 5);
	// configuration.set<std::string>("PersistenceManager", "basePath", basePath);


	auto localFileHolderFactory = std::make_shared<STI::Utils::LocalFileHolderFactory>();
	localPersistenceManager = std::make_shared<LocalPersistenceManager>(getID(), configuration, basePath, localFileHolderFactory, localCollection);

	// localPersistenceManager->setFileHolderFactory(localFileHolderFactory);



    auto engineFactory = std::make_shared<LocalEventEngineFactory>(getID(), localChannelManager, localAttributeManager, deviceMessageDispatcher, 
																	localCollection, localPersistenceManager);
	eventEngineScheduler = std::make_shared<LocalEventEngineScheduler>(this, engineFactory, deviceMessageDispatcher, localPersistenceManager);
	localPersistenceManager->attachEngineScheduler(eventEngineScheduler);


	//setEngineFactory(engineFactory);

	listenerForwarder = std::make_shared<STI::Device::DeviceMessageListenerForwarder>(this);

	//EngineSchedulerMessage ListenerID
	schedulerMessageLID.name = getID().getID() + "::EventEngineScheduler";
	schedulerMessageLID.type = STI::Device::DeviceMessageType::EngineScheduler;
//	messageListenerIDs.push_back(schedulerMessageLID);

	// //CollectionMessage ListenerID
	// collectionMessageLID.name = getID().getID() + "::DeviceCollection";
	// collectionMessageLID.type = STI::Device::DeviceMessageType::CollectionUpdate;
	// messageListenerIDs.push_back(collectionMessageLID);

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

void LocalDevice::addEventTarget(const DeviceID& id)
{
	addPartner(id);	//an event target must be a partner
	eventTargets.insert(id);
}

void LocalDevice::getEventTargets(std::set<DeviceID>& targetIDs)
{
	targetIDs = eventTargets;
}

bool LocalDevice::isEventTarget(const DeviceID& id)
{
	auto it = eventTargets.find(id);
	return (it != eventTargets.end());
}

PartnerDevice LocalDevice::partner(const DeviceID& id)
{
	std::shared_ptr<STI::Device::Device> device;

	auto it = partnerDevices.find(id);
	if (it != partnerDevices.end()) {
		localCollection->get(id, device);
	}
	PartnerDevice partner(this, device);
	return partner;
}

PartnerDevice LocalDevice::partner(const std::string& alias)
{
	auto it = partnerAliases.find(alias);
	
	if (it != partnerAliases.end()) {
		return partner(it->second);
	}

	//not found
	DeviceID id;	//null
	return partner(id);
}

void LocalDevice::addPartner(const DeviceID& id, const std::string& alias)
{
	auto it = partnerAliases.find(alias);

	if (it != partnerAliases.end()) {
		//duplicate alias
		//error
	}
	else {
		partnerAliases[alias] = id;
	}
}

void LocalDevice::addPartner(const DeviceID& id)
{
	partnerDevices.insert(id);
}

bool LocalDevice::isPartnerDevice(const DeviceID& id)
{
	auto it = partnerDevices.find(id);
	return it != partnerDevices.end();
}

//true if the LocalDevice is the target server of ID (i.e., this device is acting as a server)
bool LocalDevice::isTargetServerOf(const DeviceID& id)
{
	return id.getTargetServerID() == getID().getID();
}


std::shared_ptr<STI::Utils::FileHolder> LocalDevice::makeFileHolder(const std::string& filename)
{
	std::shared_ptr<STI::Utils::FileHolder> file;

	if (localPersistenceManager != 0) {
		file = localPersistenceManager->makeFileHolder(filename);
	}
	return file;
}

void LocalDevice::sendMessage(const std::shared_ptr<DeviceMessage>& mess)
{
	if (deviceMessageDispatcher != 0) {
		deviceMessageDispatcher->addMessage(mess);
	}
}

void LocalDevice::addCollectionListener(const std::shared_ptr<STI::Utils::LocalCollectionListenerAdapter<DeviceID>>& listener)
{
	if (localCollection != 0) {
		localCollection->addListener(listener);
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
	
	auto eventGroup = std::make_shared<STI::Engine::RawEventGroup>("SingleEvent", "");
	auto shot = std::make_shared<STI::Engine::LocalShot>(shotConfig, eventGroup);

	eventGroup->addEvent(event);

	auto parseJobStatus = eventEngineScheduler->parse(shot);

	auto parseTicket = parseTicketManager->makeTicket(parseJobStatus.pid);

	auto tF = std::chrono::system_clock::now() + std::chrono::seconds(1);
	parseTicket->wait( [&tF](){ return (tF > std::chrono::system_clock::now()); } );	//wait 1s max

	if (parseTicket->getStatus() != STI::Engine::Ticket::TicketStatus::Complete) {
		return false;
	}

	auto playJobStatus = eventEngineScheduler->play(parseJobStatus.pid, shotConfig.jobSourceID);

	resultTicket = resultTicketManager->makeTicket(playJobStatus.sid);

	tF = std::chrono::system_clock::now() + std::chrono::seconds(1);
	resultTicket->wait( [&tF](){ return (tF > std::chrono::system_clock::now()); } );	//wait 1s max

	if (resultTicket->getStatus() != STI::Engine::Ticket::TicketStatus::Complete) {
		return false;
	}

	return true;
}

bool LocalDevice::writeChannelDefault(short channel, const STI::Utils::MixedValue& value)
{
	double eventTime = 100;
	STI::Engine::RawEventTarget eventTarget(getID(), channel);
	STI::Engine::RawEvent evt0(eventTarget, eventTime, value, 0, STI::Engine::RawEventType::Play);
	std::shared_ptr<STI::Engine::ResultTicket> resultTicket;

	if (!playSingleEvent(evt0, resultTicket))
		return false;

	return true;
}

bool LocalDevice::readChannelDefault(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
	double eventTime = 100;
	STI::Engine::RawEventTarget eventTarget(getID(), channel);
	STI::Engine::RawEvent evt0(eventTarget, eventTime, value, 0, STI::Engine::RawEventType::Measurement);
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
	std::shared_ptr<STI::Device::LocalChannel> channel;
	addChannel(channelNumber, type, inputType, outputType, defaultName, channel);
	return *channel;
}

void LocalDevice::addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
	STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName, std::shared_ptr<STI::Device::LocalChannel>& channel)
{
	channel = std::make_shared<LocalChannel>(channelNumber, type, inputType, outputType, defaultName);
	localChannelManager->addChannel(channel);
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

bool DeviceCollectionPolicy::include(const STI::Device::DeviceID& key) const 
{
	return device->isPartnerDevice(key) || device->isTargetServerOf(key);
}
