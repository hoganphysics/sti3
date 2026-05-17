
#include <sti/device/PartnerDevice.h>
#include <sti/LocalDevice.h>
#include <sti/engine/RawEvent.h>
#include <sti/device/ProfileManager.h>
#include <sti/device/TaskManager.h>
#include <sti/device/VersionManager.h>

using STI::Device::PartnerDevice;
using STI::Device::LocalDevice;
using STI::Device::DeviceID;
using STI::Engine::RawEvent;

using STI::Device::DeviceMessageDispatcher;
using STI::Device::AttributeManager;
using STI::Device::ChannelManager;
using STI::Device::PersistenceManager;
using STI::Device::ProfileManager;


PartnerDevice::PartnerDevice(LocalDevice* localDevice, const std::shared_ptr<Device>& device)
: PartnerDevice(localDevice, device != 0 ? device->getID() : DeviceID(), device)
{
}

PartnerDevice::PartnerDevice(LocalDevice* localDevice, const DeviceID& partnerID, const std::shared_ptr<Device>& device)
: localDevice(localDevice), partnerID(partnerID), device(device)
{
}

const DeviceID PartnerDevice::getID() const
{
	if (device != 0) {
		return device->getID();
	}
	if (!partnerID.empty()) {
		return partnerID;
	}
	return DeviceID();
}

void PartnerDevice::kill()
{
	if (device != 0) {
		return device->kill();
	}
}

bool PartnerDevice::refresh()
{
	if (device != 0) {
		return device->refresh();
	}
	return false;
}

void PartnerDevice::getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection)
{
	if (device != 0) {
		device->getCollection(collection);
	}
}

void PartnerDevice::getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
{
	if (device != 0) {
		device->getMessageDispatcher(dispatcher);
	}
}

bool PartnerDevice::getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
{
	if (device != 0) {
		return device->getEngineScheduler(scheduler);
	}
	return false;
}

void PartnerDevice::getChannelManager(std::shared_ptr<ChannelManager>& manager)
{
	if (device != 0) {
		device->getChannelManager(manager);
	}
}

void PartnerDevice::getAttributeManager(std::shared_ptr<AttributeManager>& manager)
{
	if (device != 0) {
		device->getAttributeManager(manager);
	}
}

bool PartnerDevice::getPersistenceManager(std::shared_ptr<PersistenceManager>& manager)
{
	if (device != 0) {
		return device->getPersistenceManager(manager);
	}
	return false;
}

bool PartnerDevice::getMonitorManager(std::shared_ptr<STI::Device::MonitorManager>& manager)
{
	if (device != 0) {
		return device->getMonitorManager(manager);
	}
	return false;
}

bool PartnerDevice::getProfileManager(std::shared_ptr<ProfileManager>& manager)
{
	if (device != 0) {
		return device->getProfileManager(manager);
	}
	return false;
}

bool PartnerDevice::getTaskManager(std::shared_ptr<STI::Device::TaskManager>& manager)
{
	if (device != 0) {
		return device->getTaskManager(manager);
	}
	return false;
}

bool PartnerDevice::getLogManager(std::shared_ptr<STI::Device::LogManager>& manager)
{
	if (device != 0) {
		return device->getLogManager(manager);
	}
	return false;
}

bool PartnerDevice::getVersionManager(std::shared_ptr<STI::Device::VersionManager>& manager)
{
	if (device != 0) {
		return device->getVersionManager(manager);
	}
	return false;
}

void PartnerDevice::addEvent(const STI::Engine::RawEvent& evt, const STI::Engine::RawEvent& referenceEvent)
{
	if (localDevice == 0) return;

	localDevice->addEvent(evt, referenceEvent);
}

void PartnerDevice::addEvent(double time, const STI::Engine::RawEventTargetChannel& channel, const STI::Utils::MixedValue& value, const STI::Engine::RawEvent& referenceEvent)
{
	addEvent(time, channel, value, STI::Engine::RawEventType::Play, referenceEvent);
}

void PartnerDevice::addEvent(double time, const STI::Engine::RawEventTargetChannel& channel, const STI::Utils::MixedValue& value, 
	const STI::Engine::RawEventType& eventType, const STI::Engine::RawEvent& referenceEvent)
{
	STI::Engine::RawEventTargetDevice rawEventTargetDevice(getID());
	
	STI::Engine::RawEventTarget rawEventTarget(rawEventTargetDevice, channel);
	RawEvent evt(rawEventTarget, time, value, 0, eventType);	//event number is discarded and replaced with number from DeviceEventParser

	addEvent(evt, referenceEvent);
}


bool PartnerDevice::write(short channel, const STI::Utils::MixedValue& value)
{
	std::shared_ptr<ChannelManager> manager;
	getChannelManager(manager);

	if (manager != 0) {
		return manager->writeChannel(channel, value);
	}
	return false;
}

bool PartnerDevice::read(short channel, STI::Utils::MixedValue& data)
{
	return read(channel, STI::Utils::MixedValueType::Empty, data);
}

bool PartnerDevice::read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
	std::shared_ptr<ChannelManager> manager;
	getChannelManager(manager);

	if (manager != 0) {
		return manager->readChannel(channel, value, data);
	}
	return false;
}

void PartnerDevice::stopRW()
{
	std::shared_ptr<ChannelManager> manager;
	getChannelManager(manager);

	if (manager != 0) {
		return manager->stop();
	}
}

std::string PartnerDevice::getAttribute(const std::string& key)
{
	std::shared_ptr<AttributeManager> manager;
	getAttributeManager(manager);

	if (manager != 0) {
		return manager->getValue(key);
	}
	return "";
}

bool PartnerDevice::setAttribute(const std::string& key, const std::string& value)
{
	std::shared_ptr<AttributeManager> manager;
	getAttributeManager(manager);

	if (manager != 0) {
		return manager->setValue(key, value);
	}
	return false;
}

bool PartnerDevice::getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute)
{
	std::shared_ptr<AttributeManager> manager;
	getAttributeManager(manager);

	if (manager != 0) {
		return manager->getAttribute(key, attribute);
	}
	return false;
}
