
#include "RemoteDevice.h"
#include "RemoteDeviceCollection.h"
#include "RemoteDeviceMessageDispatcher.h"
#include "NetworkConvert.h"
#include "RemoteEventEngineScheduler.h"
#include "ChannelManager.h"
#include "RemoteChannelManager.h"
#include "RemoteAttributeManager.h"
#include "RemotePersistenceManager.h"

#include <iostream>

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
//	_tDevice(STI::TNetwork::TDevice::_duplicate(device))
{
	addDependent(remoteCollection);
	addDependent(remoteDispatcher);
	addDependent(remoteScheduler);
	addDependent(remoteChannelManager);
	addDependent(remoteAttributeManager);
}

RemoteDevice::~RemoteDevice()
{
	std::cout << "~RemoteDevice() " << getID().getID() << std::endl;
}

// bool RemoteDevice::getTDeviceRef(STI::TNetwork::TDevice_ptr& tDevice)
// {
// 	STI::TNetwork::TDevice_var newDev;
// 	newDev = _tDevice;		//implicit duplicate

// 	tDevice = newDev.out();

// 	//tDevice = STI::TNetwork::TDeviceHub::_duplicate();
// 	//tDevice = _tDevice;
// 	//tDevice = _tDevice->_duplicate(_tDevice);
// 	return !CORBA::is_nil(tDevice);
// }

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

void RemoteDevice::disable()
{
	TReferenceHolder<STI::TNetwork::TDevice>::disable();
	
	// std::unique_lock<std::mutex> deviceLock(deviceMutex);

	
	// ::STI::TNetwork::TDevice_var nilDevice = ::STI::TNetwork::TDevice::_nil();
	// _tDevice = nilDevice;	//release reference; reference is now nil

	// if (remoteAttributeManager != 0) {
	// 	remoteAttributeManager->disable();
	// }
}

const STI::Device::DeviceID RemoteDevice::getID() const
{
	std::unique_lock<std::mutex> deviceLock(deviceMutex);

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
	
//	bool success = false;

	::STI::TNetwork::TDeviceCollection_var tDeviceCollection;	//remote reference

	try {
		tDeviceCollection = getTRef()->getDeviceCollection();	//remote call
//		success = true;

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
	// std::shared_ptr<RemoteDeviceMessageDispatcher> remoteDispatcher;		//wrapper

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
	// std::shared_ptr<RemoteEventEngineScheduler> remoteScheduler;	//wrapper

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

	if (listenerForwarder == 0) return;

	bool success = false;

	::STI::TNetwork::TChannelManager_var tChannelManager;	//remote reference
	
	try {
		tChannelManager = getTRef()->getChannelManager();	//remote call

		if (!CORBA::is_nil(tChannelManager)) {
			remoteChannelManager = std::make_shared<RemoteChannelManager>(tChannelManager, listenerForwarder, getID());
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

	if (listenerForwarder == 0) return;

	::STI::TNetwork::TAttributeManager_var tAttributelManager;	//remote reference
	
	try {
		tAttributelManager = getTRef()->getAttributeManager();	//remote call

		if (!CORBA::is_nil(tAttributelManager)) {
			remoteAttributeManager = std::make_shared<RemoteAttributeManager>(tAttributelManager, listenerForwarder, getID());
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

