
#include "RemoteDevice.h"
#include "RemoteDeviceCollection.h"
#include "RemoteDeviceMessageDispatcher.h"
#include "NetworkConvert.h"
#include "RemoteEventEngineScheduler.h"
#include <sti/device/ChannelManager.h>
#include "RemoteChannelManager.h"
#include "RemoteAttributeManager.h"
#include "RemotePersistenceManager.h"

using STI::Network::RemoteDevice;
using STI::Network::RemoteDeviceCollection;
using STI::Network::RemoteDeviceMessageDispatcher;
using STI::Network::convert;
using STI::Network::RemoteEventEngineScheduler;
using STI::Device::ChannelManager;
using STI::Network::RemoteChannelManager;
using STI::Network::RemoteAttributeManager;
using STI::TNetwork::TReferenceHolder;

RemoteDevice::RemoteDevice(::STI::TNetwork::TDevice_ptr device)
	: TReferenceHolder<STI::TNetwork::TDevice>(device, deviceMutex)
{
	addDependent(remoteCollection);
	addDependent(remoteDispatcher);
	addDependent(remoteScheduler);
	addDependent(remoteChannelManager);
	addDependent(remoteAttributeManager);
}

RemoteDevice::~RemoteDevice()
{
}


bool RemoteDevice::getTDeviceRef(STI::TNetwork::TDevice_var& tDevice)
{
	tDevice = getTRef();		//implicit duplicate

	return !CORBA::is_nil(tDevice);
}

void RemoteDevice::attachMessageListenerForwarder(const std::shared_ptr<STI::Device::DeviceMessageListenerForwarder>& forwarder)
{
	std::unique_lock<std::mutex> deviceLock(deviceMutex);

	listenerForwarder = forwarder;
}


/// Check if the RemoteDevice reference is still live.  Attempts a call over the network
/// and returns false if it times out or there is some other error.
bool RemoteDevice::refresh()
{
	std::unique_lock<std::mutex> deviceLock(deviceMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->refresh();
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}

void RemoteDevice::kill()
{
	std::unique_lock<std::mutex> deviceLock(deviceMutex);

	if (isDisabled()) return;

	bool success = false;

	try {
		getTRef()->kill();
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteDevice::activate()
{
}

void RemoteDevice::disable()
{
	TReferenceHolder<STI::TNetwork::TDevice>::disable();
}

const STI::Device::DeviceID RemoteDevice::getID() const
{
	std::unique_lock<std::mutex> deviceLock(deviceMutex);

	if (cachedDeviceID.isCached()) return cachedDeviceID.get();

	::STI::TNetwork::TDeviceID_var tDeviceID;

	bool success = false;

	if (!isDisabled()) {
		try {
			tDeviceID = getTRef()->getID();
			success = true;
		}
		catch (CORBA::TRANSIENT&) {
		}
		catch (CORBA::SystemException&) {
		}
		catch (CORBA::Exception&)
		{
		}
	}

	STI::Device::DeviceID deviceID;

	if(success) {
		deviceID = convert<::STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tDeviceID);
		cachedDeviceID.set(deviceID);
	}

	return deviceID;
}



void RemoteDevice::getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection)
{
	std::unique_lock<std::mutex> deviceLock(deviceMutex);

	if (isLive(remoteCollection)) {
		collection = remoteCollection;
		return;
	}
	else if (remoteCollection != 0) {
		//non-null but not live for some reason; disable
		remoteCollection->disable();
	}

	if (isDisabled()) return;
	
	::STI::TNetwork::TDeviceCollection_var tDeviceCollection;	//remote reference

	try {
		tDeviceCollection = getTRef()->getDeviceCollection();	//remote call

		if (!CORBA::is_nil(tDeviceCollection)) {
			remoteCollection = std::make_shared<RemoteDeviceCollection>(tDeviceCollection);
			collection = remoteCollection;
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteDevice::getMessageDispatcher(std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher)
{
	std::unique_lock<std::mutex> deviceLock(deviceMutex);

	if (isLive(remoteDispatcher)) {
		dispatcher = remoteDispatcher;
		return;
	}
	else if (remoteDispatcher != 0) {
		//non-null but not live for some reason; disable
		remoteDispatcher->disable();
	}

	if (isDisabled()) return;

	bool success = false;

	::STI::TNetwork::TDeviceMessageDispatcher_var tMessageDispatcher;	//remote reference

	try {
		tMessageDispatcher = getTRef()->getMessageDispatcher();	//remote call
		success = true;
		
		if (success && !CORBA::is_nil(tMessageDispatcher)) {
			remoteDispatcher = std::make_shared<RemoteDeviceMessageDispatcher>(tMessageDispatcher);
			dispatcher = remoteDispatcher;
			success = (dispatcher != 0);
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

bool RemoteDevice::getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
{
	std::unique_lock<std::mutex> deviceLock(deviceMutex);
	
	if (isLive(remoteScheduler)) {
		scheduler = remoteScheduler;
		return true;
	}
	else if (remoteScheduler != 0) {
		//non-null but not live for some reason; disable
		remoteScheduler->disable();
	}

	if (isDisabled()) return false;

	bool success = false;

	::STI::TNetwork::TEventEngineScheduler_var tEngineScheduler;	//remote reference

	try {
		tEngineScheduler = getTRef()->getEngineScheduler();	//remote call
		success = true;
		
		if (success && !CORBA::is_nil(tEngineScheduler)) {
			remoteScheduler = std::make_shared<RemoteEventEngineScheduler>(tEngineScheduler);
			scheduler = remoteScheduler;
			success = (scheduler != 0);
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}

void RemoteDevice::getChannelManager(std::shared_ptr<STI::Device::ChannelManager>& manager)
{
	auto remoteID = getID();	//Need to get this first to avoid deadlock with getID()

	std::unique_lock<std::mutex> deviceLock(deviceMutex);
	
	if (isLive(remoteChannelManager)) {
		manager = remoteChannelManager;
		return;
	}
	else if (remoteChannelManager != 0) {
		//non-null but not live for some reason; disable
		remoteChannelManager->disable();
	}

	if (isDisabled()) return;

	bool success = false;

	::STI::TNetwork::TChannelManager_var tChannelManager;	//remote reference
	
	try {
		tChannelManager = getTRef()->getChannelManager();	//remote call

		if (!CORBA::is_nil(tChannelManager)) {
			remoteChannelManager = std::make_shared<RemoteChannelManager>(tChannelManager, listenerForwarder, remoteID);
			success = true;
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	if (success && isLive(remoteChannelManager)) {
		manager = remoteChannelManager;
	}
}

void RemoteDevice::getAttributeManager(std::shared_ptr<STI::Device::AttributeManager>& manager)
{
	auto remoteID = getID();	//Need to get this first to avoid deadlock with getID()

	std::unique_lock<std::mutex> deviceLock(deviceMutex);

	if (isLive(remoteAttributeManager)) {
		manager = remoteAttributeManager;
		return;
	}
	else if (remoteAttributeManager != 0) {
		//non-null but not live for some reason; disable
		remoteAttributeManager->disable();
	}

	if (isDisabled()) return;

	::STI::TNetwork::TAttributeManager_var tAttributelManager;	//remote reference
	
	try {
		tAttributelManager = getTRef()->getAttributeManager();	//remote call

		if (!CORBA::is_nil(tAttributelManager)) {
			remoteAttributeManager = std::make_shared<RemoteAttributeManager>(tAttributelManager, listenerForwarder, remoteID);
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	manager = remoteAttributeManager;
}

bool RemoteDevice::getPersistenceManager(std::shared_ptr<STI::Device::PersistenceManager>& manager)
{
	std::unique_lock<std::mutex> deviceLock(deviceMutex);

	if (isLive(remotePersistenceManager)) {
		manager = remotePersistenceManager;
		return (manager != 0);
	}
	else if (remotePersistenceManager != 0) {
		//non-null but not live for some reason; disable
		remotePersistenceManager->disable();
	}

	if (isDisabled()) return false;

	::STI::TNetwork::TPersistenceManager_var tPersistenceManager;	//remote reference
	
	try {
		tPersistenceManager = getTRef()->getPersistenceManager();	//remote call

		if (!CORBA::is_nil(tPersistenceManager)) {
			remotePersistenceManager = std::make_shared<RemotePersistenceManager>(tPersistenceManager);
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	manager = remotePersistenceManager;
	return (manager != 0);
}

bool RemoteDevice::write(short channel, const STI::Utils::MixedValue& value)
{
	std::shared_ptr<ChannelManager> manager;
	getChannelManager(manager);

	if (manager != 0) {
		return manager->writeChannel(channel, value);
	}
	return false;
}

bool RemoteDevice::read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
	std::shared_ptr<ChannelManager> manager;
	getChannelManager(manager);

	if (manager != 0) {
		return manager->readChannel(channel, value, data);
	}
	return false;
}

void RemoteDevice::stopRW()
{
	std::shared_ptr<ChannelManager> manager;
	getChannelManager(manager);

	if (manager != 0) {
		manager->stop();
	}
}

std::string RemoteDevice::getAttribute(const std::string& key)
{
	std::shared_ptr<STI::Device::AttributeManager> manager;
	getAttributeManager(manager);

	if (manager != 0) {
		return manager->getValue(key);
	}
	return "";
}


bool RemoteDevice::setAttribute(const std::string& key, const std::string& value)
{
	std::shared_ptr<STI::Device::AttributeManager> manager;
	getAttributeManager(manager);

	if (manager != 0) {
		return manager->setValue(key, value);
	}
	return false;
}

bool RemoteDevice::getAttribute(const std::string& key, std::shared_ptr<STI::Device::Attribute>& attribute)
{
	std::shared_ptr<STI::Device::AttributeManager> manager;
	getAttributeManager(manager);

	if (manager != 0) {
		return manager->getAttribute(key, attribute);
	}
	return false;
}

