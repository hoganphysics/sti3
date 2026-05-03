#include <sti/LocalDevice.h>

#include <sti/device/AutoMonitor.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceMessageReceiver.h>
#include <sti/device/LocalAttribute.h>
#include <sti/device/LocalChannel.h>
#include <sti/device/ServerMessageRelayer.h>
#include <sti/device/VersionInfo.h>
#include <sti/device/VersionManager.h>

#include <sti/engine/Measurement.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/ParseTicket.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/ShotRepository.h>

#include <sti/utils/Configuration.h>
#include <sti/utils/LocalFileHolder.h>
#include <sti/utils/MixedValue.h>

#include "DeviceMessageListenerForwarder.h"
#include "LocalAttributeManager.h"
#include "LocalChannelManager.h"
#include "LocalDeviceMessageDispatcher.h"
#include "LocalEventEngineFactory.h"
#include "LocalEventEngineScheduler.h"
#include "LocalLogManager.h"
#include "LocalMonitorManager.h"
#include "LocalPersistenceManager.h"
#include "LocalProfileManager.h"
#include "LocalTaskManager.h"
#include "LocalShot.h"
#include "PseudoSynchronousEvent.h"

#include <filesystem>
#include <memory>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdint>

using STI::Device::AttributeManager;
using STI::Device::AutoMonitor;
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
using STI::Device::LocalMonitor;
using STI::Device::LocalMonitorManager;
using STI::Device::Monitor;
using STI::Device::MonitorManager;
using STI::Device::PartnerDevice;
using STI::Device::LocalProfileManager;
using STI::Device::TaskManager;
using STI::Device::LocalTaskManager;
using STI::Device::LogManager;
using STI::Device::LocalLogManager;
using STI::Device::VersionInfo;
using STI::Device::VersionManager;

using STI::Engine::LocalEventEngineFactory;
using STI::Engine::LocalEventEngineScheduler;
using STI::Engine::ParseID;
using STI::Engine::ShotID;

using STI::Utils::Configuration;

static auto constructorConfigError = [](const std::string& key) {
	std::stringstream message;
	message << "LocalDevice constructor error: ";
	message << "Required parameter '" << key << "' was not found in the Configuration.";
	throw std::runtime_error(message.str());
};

namespace {

std::uintmax_t getLogMaxFileSizeBytes(const Configuration& config)
{
    auto maxFileSizeBytes = config.get<std::uintmax_t>(
        "Logs",
        "Max File Size Bytes",
        config.get<std::uintmax_t>("Logs", "Max File Size", LocalLogManager::DefaultMaxLogFileSizeBytes).get()).get();

    if (maxFileSizeBytes == 0) {
        return LocalLogManager::DefaultMaxLogFileSizeBytes;
    }

    return maxFileSizeBytes;
}

} // namespace


LocalDevice::LocalDevice(const std::map<std::string, std::string>& config)
: LocalDevice( Configuration(config) )
{
}

LocalDevice::LocalDevice(const Configuration& config, const std::string& section)
: LocalDevice(
	config.getOrThrow<std::string>(section, "Device Name", constructorConfigError), 
	config.getOrThrow<std::string>(section, "IP Address", constructorConfigError), 
	config.getOrThrow<unsigned short>(section, "Module", constructorConfigError),
	config.getOrThrow<std::string>(section, "Target Server", constructorConfigError),
	config)
{
}

LocalDevice::LocalDevice(const std::string& name, const std::string& address, unsigned short module,
	const std::string& targetServer, const STI::Utils::Configuration& config)
: STI::Engine::DeviceEventParser(), id(name, address, module, targetServer), usingParseDefault(false), usingRWdefault(false)

{
	std::shared_ptr<DeviceCollectionPolicy> policy = std::make_shared<DeviceCollectionPolicy>(this);;
	localCollection = std::make_shared<STI::Utils::LocalCollection<DeviceID, Device>>(policy);

	deviceMessageDispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
	deviceMessageReceiver = std::make_shared<DeviceMessageReceiver>(id, localCollection, deviceMessageDispatcher);

	auto deviceCollectionListener = std::make_shared<STI::Device::LocalDevice::DeviceCollectionListener>(this);
	addCollectionListener(deviceCollectionListener);

	auto deviceRootPath = std::filesystem::current_path();	//cwd
	deviceRootPath /= ".sti";

	auto basePath = LocalPersistenceManager::makeBasePath(
		config.get<std::string>("PersistenceManager", "root path", deviceRootPath.generic_string()),
		config.get<std::string>("PersistenceManager", "device subdirectory", getID().getID())
	);

	localChannelManager = std::make_shared<LocalChannelManager>(this, deviceMessageDispatcher);
	localAttributeManager = std::make_shared<LocalAttributeManager>(id, deviceMessageDispatcher);
	
	localProfileManager = std::make_shared<LocalProfileManager>(getID(), localCollection);
	localProfileManager->addProfileTarget(localAttributeManager);
	localProfileManager->addProfileTarget(localChannelManager);
	
	localTaskManager = std::make_shared<LocalTaskManager>();
	localMonitorManager = std::make_shared<LocalMonitorManager>(id, deviceMessageDispatcher);
	versionManager = STI::Device::makeVersionManager();

	auto localFileHolderFactory = std::make_shared<STI::Utils::LocalFileHolderFactory>(getID().getID());
	localPersistenceManager = std::make_shared<LocalPersistenceManager>(getID(), config, basePath, localFileHolderFactory, localCollection, versionManager);

	localPersistenceManager->addPersistenceTarget(localAttributeManager);
	localPersistenceManager->addPersistenceTarget(localChannelManager);
	localPersistenceManager->addPersistenceTarget(localProfileManager);
	localPersistenceManager->addPersistenceTarget(localTaskManager);


    auto engineFactory = std::make_shared<LocalEventEngineFactory>(getID(), localChannelManager, localAttributeManager, deviceMessageDispatcher, 
																	localCollection, localPersistenceManager);
	eventEngineScheduler = std::make_shared<LocalEventEngineScheduler>(this, engineFactory, deviceMessageDispatcher, localPersistenceManager);
	localPersistenceManager->attachEngineScheduler(eventEngineScheduler);

	listenerForwarder = std::make_shared<STI::Device::DeviceMessageListenerForwarder>(this);

	schedulerMessageLID.name = getID().getID() + "::EventEngineScheduler";
	schedulerMessageLID.type = STI::Device::DeviceMessageType::EngineScheduler;

	serverMessageRelayer = std::make_shared<STI::Device::ServerMessageRelayer>(getID(), deviceMessageDispatcher);

	serverMessageRelayer->addFilter<STI::Device::EngineJobUpdateDeviceMessage>( 
		[](const std::shared_ptr<STI::Device::EngineJobUpdateDeviceMessage>& message)->bool {
			//only relay job messages that originate from the job owner (avoids duplicates)
			return message->getJobOwner() == message->originalSourceID();
		});

	// serverMessageRelayer->addFilter<STI::Device::EngineStateMessage>( 
	// 	[](const std::shared_ptr<STI::Device::EngineStateMessage>& message)->bool {
	// 		//only relay job messages that originate from the job owner (avoids duplicates)
	// 		return true;
	// 	});

	// STI::Engine::EngineID id0(0);
	// addEventEngine(id0);

	//0=async, >0=sync engines
	int engineCount = config.get<int>("EngineManager", "Engine Count", 2); //sync engines
	for (unsigned i = 0; i < engineCount + 1; ++i) {
		STI::Engine::EngineID id(i);
		addEventEngine(id);
	}

	parseTicketManager = std::make_shared<STI::Engine::ParseTicketManager<>>(eventEngineScheduler);
	resultTicketManager = std::make_shared<STI::Engine::ResultTicketManager<>>(localPersistenceManager, eventEngineScheduler);

	deviceMessageReceiver->addListener<EngineSchedulerMessage>(getID(), "ParseTicketManagerScheduler", parseTicketManager);
	deviceMessageReceiver->addListener<EngineSchedulerMessage>(getID(), "ResultTicketManagerScheduler", resultTicketManager);

	deviceMessageReceiver->addListener<STI::Device::EngineJobUpdateDeviceMessage>(getID(), "ParseTicketManagerJobUpdate", parseTicketManager);
	deviceMessageReceiver->addListener<STI::Device::EngineJobUpdateDeviceMessage>(getID(), "ResultTicketManagerJobUpdate", resultTicketManager);
	
	localLogManager = std::make_shared<LocalLogManager>(this, localPersistenceManager, getLogMaxFileSizeBytes(config));
}

LocalDevice::~LocalDevice()
{
	// disable();
	localCollection->clear();
}

void LocalDevice::activate()
{
	if (localPersistenceManager != 0) {
		localPersistenceManager->loadPersistenceTargets();
	}
}

void LocalDevice::disable()
{
	if (eventEngineScheduler != 0) {
		eventEngineScheduler->cancelAll();
		eventEngineScheduler->stopAll();
		eventEngineScheduler->clearAll();
	}

	if (deviceMessageReceiver != 0) {
		deviceMessageReceiver->clearListeners();
	}

	if (localCollection != 0) {
		localCollection->clearListeners();
	}
}

void LocalDevice::kill()
{
	if (removerCallback) {
		//callback to remove this LocalDevice from the LocalHub
		removerCallback();
	}
}

void LocalDevice::setRemoveCB(const std::function<void(void)>& remover) 
{
	removerCallback = remover;
}

void LocalDevice::addEventTarget(const DeviceID& id, const std::string& alias)
{
	addPartner(id, alias);	//an event target must be a partner
	addEventTarget(id);
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
	PartnerDevice partner(this, id, device);
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
	addPartner(id);
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

STI::Device::Logger& LocalDevice::log()
{
	return localLogManager->log();
}

STI::Device::Logger& LocalDevice::log(const std::string& name)
{
	return localLogManager->log(name);
}

std::shared_ptr<STI::Utils::FileHolder> LocalDevice::makeFileHolder(const std::string& path, const std::string& filename)
{
	std::shared_ptr<STI::Utils::FileHolder> file;

	if (localPersistenceManager != 0) {
		file = localPersistenceManager->makeFileHolder(path, filename);
	}
	return file;
}

void LocalDevice::setShotRepository(const std::shared_ptr<STI::Engine::ShotRepository>& repo)
{
	if (localPersistenceManager != 0) {
		localPersistenceManager->setShotRepository(repo);
	}
}

void LocalDevice::setEngineConflictPolicy(const std::shared_ptr<STI::Engine::EngineConflictPolicy>& policy)
{
	if (eventEngineScheduler != 0) {
		eventEngineScheduler->setEngineConflictPolicy(policy);
	}
}

void LocalDevice::sendMessage(const std::shared_ptr<DeviceMessage>& mess)
{
	if (deviceMessageDispatcher != 0) {
		deviceMessageDispatcher->addMessage(mess);
	}
}

void LocalDevice::addTask(const std::shared_ptr<STI::Utils::Task>& task)
{
	if (localTaskManager != 0) {
		localTaskManager->addTask(task);
	}
}

void LocalDevice::addCollectionListener(const std::shared_ptr<STI::Utils::LocalCollectionListenerAdapter<DeviceID>>& listener)
{
	if (localCollection != 0) {
		localCollection->addListener(listener);
	}
}

bool LocalDevice::refresh()
{
	if (localCollection == 0) {
		return true;
	}

	return true;

	std::set<DeviceID> ids;
	localCollection->getIDs(ids);
	const auto selfID = getID();

	for (auto& id : ids) {
		if (id == selfID) {
			continue;
		}

		std::shared_ptr<Device> node;
		bool alive = localCollection->get(id, node) && node != 0 && node->refresh();
		if (!alive) {
			localCollection->remove(id);
		}
	}

	return true;
}

//LocalDeviceCollection event handler
void LocalDevice::DeviceCollectionListener::add(const DeviceID& id)
{
	std::cout << "++++ add( " << id.getID() << " )" << std::endl;

	// Listen to EngineScheduler messages from:
	// 1) Declared event targets and 2) any device that has this device as a target server.

	// if (localDevice->isTargetServerOf(id)) {
	// 	localDevice->addEventTarget(id);
	// }

	if (localDevice->isTargetServerOf(id)) {
		STI::Device::ServerMessageRelayer::addAllListeners(localDevice->deviceMessageReceiver, id, localDevice->serverMessageRelayer);
	}

	if (localDevice->isEventTarget(id) || localDevice->isTargetServerOf(id)) {
		auto listener = localDevice->eventEngineScheduler->getMessageListener();

		//listen to events on new device 'id'
		localDevice->deviceMessageReceiver->addListener(id, localDevice->schedulerMessageLID, listener);
	}
	
	//The DeviceMessageListenerForwarder allows newly added devices to register message listeners via the
	//local DeviceMessageReceiver, allowing update messages to be passed to the stored device instance.
	std::shared_ptr<Device> newDevice;
	if (localDevice->localCollection->get(id, newDevice) && newDevice != 0) {
		newDevice->attachMessageListenerForwarder(localDevice->listenerForwarder);
	}

	auto mess = std::make_shared<CollectionUpdateMessage>(localDevice->getID());
	localDevice->sendMessage(mess);
}

//LocalDeviceCollection event handler
void LocalDevice::DeviceCollectionListener::remove(const DeviceID& id)
{
	auto mess = std::make_shared<CollectionUpdateMessage>(localDevice->getID());

	localDevice->sendMessage(mess);

	if ( localDevice->isTargetServerOf(id) ) {
		STI::Device::ServerMessageRelayer::removeAllListeners(localDevice->deviceMessageReceiver, id, localDevice->serverMessageRelayer);
	}

	localDevice->deviceMessageReceiver->removeListener(id, localDevice->schedulerMessageLID);

	std::cout << "---- remove( " << id.getID() << " )" << std::endl;
}

const DeviceID LocalDevice::getID() const
{
	return id;
}


bool LocalDevice::write(short channel, const STI::Utils::MixedValue& value)
{
	std::shared_ptr<STI::Device::Channel> ch;
	
	//type check
	if (localChannelManager->getChannel(channel, ch) 
		&& ch->getType() == STI::Device::ChannelType::Output 
		&& value.isType(ch->getOutputType())) {
		
		return writeChannel(channel, value);
	}
	return false;	//value has wrong type	
}

bool LocalDevice::read(short channel, STI::Utils::MixedValue& data)
{
	return read(channel, STI::Utils::MixedValueType::Empty, data);
}

bool LocalDevice::read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
	std::shared_ptr<STI::Device::Channel> ch;

	//type check
	if (localChannelManager->getChannel(channel, ch)
		&& ch->getType() == STI::Device::ChannelType::Input
		&& value.isType(ch->getOutputType())) {
		
		return readChannel(channel, value, data);
	}
	return false;	//value has wrong type	
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

	std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
	if (getEngineScheduler(scheduler)) {
		scheduler->cancelAll();
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
		parseTicket->cancel();
		return false;
	}

	auto playJobStatus = eventEngineScheduler->play(parseJobStatus.pid, shotConfig.jobSourceID);

	resultTicket = resultTicketManager->makeTicket(playJobStatus.sid);

	tF = std::chrono::system_clock::now() + std::chrono::seconds(1);
	resultTicket->wait( [&tF](){ return (tF > std::chrono::system_clock::now()); } );	//wait 1s max

	if (resultTicket->getStatus() != STI::Engine::Ticket::TicketStatus::Complete) {
		resultTicket->cancel();
		return false;
	}

	return true;
}

bool LocalDevice::writeChannelDefault(short channel, const STI::Utils::MixedValue& value)
{
	usingRWdefault = true;
	if (usingParseDefault) return false;

	double eventTime = getMinimumEventStartTime();
	STI::Engine::RawEventTarget eventTarget(getID(), channel);
	STI::Engine::RawEvent evt0(eventTarget, eventTime, value, 0, STI::Engine::RawEventType::Play);
	std::shared_ptr<STI::Engine::ResultTicket> resultTicket;

	if (!playSingleEvent(evt0, resultTicket))
		return false;

	return true;
}

bool LocalDevice::readChannelDefault(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
	usingRWdefault = true;
	if (usingParseDefault) return false;

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

void LocalDevice::parseEventsDefault(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
{
	usingParseDefault = true;

	for (auto& evts : events) {
		
		if (usingRWdefault) {
			throw STI::Engine::EventParsingException(evts.second.at(0), "Error: Recursive parse detected. Atempted to use both the default parseDeviceEvents and the default read/write channel. The device must override at least one of these.");
		}

		auto pseudoSyncEvt = std::make_shared<STI::Engine::PseudoSynchronousEvent>(evts.first, evts.second, this);
		synchedEvents.push_back(pseudoSyncEvt);
	}
}

std::string LocalDevice::getAttribute(const std::string& key)
{
	std::shared_ptr<AttributeManager> manager;
	getAttributeManager(manager);

	if (manager != 0) {
		return manager->getValue(key);
	}
	return "";
}

bool LocalDevice::setAttribute(const std::string& key, const std::string& value)
{
	std::shared_ptr<AttributeManager> manager;
	getAttributeManager(manager);

	if (manager != 0) {
		return manager->setValue(key, value);
	}
	return false;
}

bool LocalDevice::getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute)
{
	std::shared_ptr<AttributeManager> manager;
	getAttributeManager(manager);

	if (manager != 0) {
		return manager->getAttribute(key, attribute);
	}
	return false;
}


void LocalDevice::addEventEngine(const STI::Engine::EngineID& engineID)
{
//	auto engine = eventEngineFactory->createEngine(getID(), localChannels, this, deviceMessageDispatcher, localCollection);

	if (eventEngineScheduler != 0) {
		eventEngineScheduler->addEngine(engineID, this, this);
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

LocalChannel& LocalDevice::addInputChannel(unsigned short channelNumber, STI::Utils::MixedValueType inputType, const std::string& defaultName)
{
	return addInputChannel(channelNumber, inputType, STI::Utils::MixedValueType::Empty, defaultName);
}

LocalChannel& LocalDevice::addInputChannel(unsigned short channelNumber, STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName)
{
	return addChannel(channelNumber, STI::Device::ChannelType::Input, inputType, outputType, defaultName);
}

LocalChannel& LocalDevice::addOutputChannel(unsigned short channelNumber, STI::Utils::MixedValueType outputType, const std::string& defaultName)
{
	return addChannel(channelNumber, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, outputType, defaultName);
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

void LocalDevice::addMonitor(const std::shared_ptr<LocalMonitor>& monitor)
{
	if (localMonitorManager != 0 && monitor != 0) {
		localMonitorManager->addMonitor(monitor);
	}
}

void LocalDevice::addMonitor(const std::string& id, std::shared_ptr<LocalMonitor>& monitor)
{
	if (localMonitorManager == 0) {
		monitor.reset();
		return;
	}

	std::shared_ptr<Monitor> existing;
	if (localMonitorManager->getMonitor(id, existing)) {
		monitor = std::dynamic_pointer_cast<LocalMonitor>(existing);
		return;
	}

	monitor = std::make_shared<LocalMonitor>(id);
	localMonitorManager->addMonitor(monitor);
}

LocalMonitor& LocalDevice::addMonitor(const std::string& id)
{
	std::shared_ptr<LocalMonitor> monitor;
	addMonitor(id, monitor);
	return *monitor;
}

void LocalDevice::addAutoMonitor(
	const std::string& id,
	double updateInterval_s,
	const std::function<STI::Utils::MixedValue(void)>& updater,
	std::shared_ptr<AutoMonitor>& monitor)
{
	if (localMonitorManager == 0 || localTaskManager == 0) {
		monitor.reset();
		return;
	}

	std::shared_ptr<Monitor> existing;
	if (localMonitorManager->getMonitor(id, existing)) {
		monitor = std::dynamic_pointer_cast<AutoMonitor>(existing);

		if (monitor == 0) {
			throw std::runtime_error("Monitor with id '" + id + "' already exists and is not an AutoMonitor.");
		}
		return;
	}

	monitor = AutoMonitor::create(id, updateInterval_s, updater, localTaskManager);
	localMonitorManager->addMonitor(monitor);
}

AutoMonitor& LocalDevice::addAutoMonitor(
	const std::string& id,
	double updateInterval_s,
	const std::function<STI::Utils::MixedValue(void)>& updater)
{
	std::shared_ptr<AutoMonitor> monitor;
	addAutoMonitor(id, updateInterval_s, updater, monitor);
	return *monitor;
}


void LocalDevice::getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection)
{
	collection = localCollection;
}

void LocalDevice::getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
{
	dispatcher = deviceMessageDispatcher;
}

bool LocalDevice::getMessageReceiver(std::shared_ptr<DeviceMessageReceiver>& receiver)
{
	receiver = deviceMessageReceiver;
	return receiver != 0;
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

bool LocalDevice::getProfileManager(std::shared_ptr<ProfileManager>& manager)
{
	manager = localProfileManager;
	return manager != 0;
}

bool LocalDevice::getTaskManager(std::shared_ptr<TaskManager>& manager)
{
	manager = localTaskManager;
	return manager != 0;
}

bool LocalDevice::getMonitorManager(std::shared_ptr<MonitorManager>& manager)
{
	manager = localMonitorManager;
	return manager != 0;
}

bool LocalDevice::getLogManager(std::shared_ptr<LogManager>& manager)
{
	manager = localLogManager;
	return manager != 0;
}

bool LocalDevice::getVersionManager(std::shared_ptr<VersionManager>& manager)
{
	manager = versionManager;
	return manager != 0;
}

bool LocalDevice::getMonitorManager(std::shared_ptr<LocalMonitorManager>& manager)
{
	manager = localMonitorManager;
	return manager != 0;
}

bool LocalDevice::getLogManager(std::shared_ptr<LocalLogManager>& manager)
{
	manager = localLogManager;
	return manager != 0;
}

bool LocalDevice::getFileServer(std::shared_ptr<STI::Utils::FileServer>& fileServer)
{
	if (localPersistenceManager == 0) return false;

	return localPersistenceManager->getFileServer(fileServer);
}

bool LocalDevice::addVersionInfo(const VersionInfo& version)
{
	return versionManager != 0 && versionManager->addVersionInfo(version);
}

bool LocalDevice::addVersionInfo(const std::string& component, const std::string& version)
{
	return addVersionInfo(VersionInfo(component, version));
}

bool DeviceCollectionPolicy::include(const STI::Device::DeviceID& key) const 
{
	return device->isPartnerDevice(key) || device->isTargetServerOf(key);
}
