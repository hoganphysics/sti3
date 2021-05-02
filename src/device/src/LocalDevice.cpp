
#include "LocalDevice.h"
#include "LocalDeviceMessageDispatcher.h"
#include "DeviceMessageReceiver.h"
#include "LocalEventEngineScheduler.h"
#include "DeviceMessageListener.h"
#include "DeviceMessage.h"
#include "LocalEventEngineFactory.h"

#include "MixedValue.h"
#include "LocalChannelManager.h"
#include "LocalChannel.h"

#include "LocalAttribute.h"
#include "LocalAttributeManager.h"

#include <memory>
#include <iostream>

using std::cout;
using std::endl;

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

LocalDevice::LocalDevice(const std::string& name, const std::string& address, unsigned short module,
	const std::string& targetServer) : id(name, address, module, targetServer)
{
	std::shared_ptr<DeviceCollectionPolicy> policy = std::make_shared<DeviceCollectionPolicy>(this);;
	localCollection = std::make_shared<STI::Utils::LocalCollection<DeviceID, Device>>(policy);

	deviceMessageDispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
	deviceMessageReceiver = std::make_shared<DeviceMessageReceiver>(id, localCollection);

	auto deviceCollectionListener = std::make_shared<STI::Device::LocalDevice::DeviceCollectionListener>(this);
	localCollection->addListener(deviceCollectionListener);

	eventEngineScheduler = std::make_shared<LocalEventEngineScheduler>(this);
	localChannelManager = std::make_shared<LocalChannelManager>(this);
	localAttributeManager = std::make_shared<LocalAttributeManager>(id, deviceMessageDispatcher);

	//setEngineFactory(engineFactory);

	//EngineSchedulerMessage ListenerID
	schedulerMessageLID.name = getID().getID() + "::EventEngineScheduler";
	schedulerMessageLID.type = STI::Device::DeviceMessageType::EngineScheduler;

}

LocalDevice::~LocalDevice()
{
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

//LocalDeviceCollection event handler
void LocalDevice::DeviceCollectionListener::add(const DeviceID& id)
{
	// Listen to EngineScheduler messages from:
	// 1) Declared event targets and 2) any device that has this device as a target server.

	// if (localDevice->isTargetServerOf(id)) {
	// 	localDevice->addEventTarget(id);
	// }

	std::cout << "***** LocalDevice::DeviceCollectionListener::add? ";

	if( localDevice->isEventTarget(id) || localDevice->isTargetServerOf(id)) {

		std::cout << "yes\n";
		
		auto listener = std::static_pointer_cast<DeviceMessageListener<EngineSchedulerMessage>>(localDevice->eventEngineScheduler);
		
		localDevice->deviceMessageReceiver->addListener(id, localDevice->schedulerMessageLID, listener);	//listen to events on new device 'id'
	}
}

//LocalDeviceCollection event handler
void LocalDevice::DeviceCollectionListener::remove(const DeviceID& id)
{
	localDevice->deviceMessageReceiver->removeListener(id, localDevice->schedulerMessageLID);
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


void LocalDevice::addEventEngine(const STI::Engine::EngineID& engineID)
{
//	auto engine = eventEngineFactory->createEngine(getID(), localChannels, this, deviceMessageDispatcher, localCollection);
	eventEngineScheduler->addEngine(engineID);
}



LocalChannel& LocalDevice::addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName)
{
	auto channel = std::make_shared<LocalChannel>(channelNumber, type, inputType, outputType, defaultName);
	
	localChannelManager->addChannel(channel);

	return *channel;
}

LocalAttribute& LocalDevice::addAttribute(const std::string& key, const std::string& initialValue)
{
    auto attribute = std::make_shared<STI::Device::LocalAttribute>(key, initialValue);
	localAttributeManager->addAttribute(attribute);

	return *attribute;
}

LocalAttribute& LocalDevice::addAttribute(const std::string& key, const std::string& initialValue, std::vector<std::string> allowedValues)
{
    auto attribute = std::make_shared<STI::Device::LocalAttribute>(key, initialValue, allowedValues);
	localAttributeManager->addAttribute(attribute);

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

bool LocalDevice::isPartnerDevice(const DeviceID& id)
{
	auto it = partnerDevices.find(id);

	return it != partnerDevices.end();
}

bool DeviceCollectionPolicy::include(const STI::Device::DeviceID& key) const 
{
	return device->isPartnerDevice(key) || device->isTargetServerOf(key);
}
