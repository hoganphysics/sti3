
#include "RemoteDevice.h"
#include "RemoteDeviceCollection.h"
#include "RemoteDeviceMessageDispatcher.h"
#include "NetworkConvert.h"
#include "RemoteEventEngineScheduler.h"
#include "ChannelManager.h"
#include "RemoteChannelManager.h"
#include "RemoteAttributeManager.h"


using STI::Network::RemoteDevice;
using STI::Network::RemoteDeviceCollection;
using STI::Network::RemoteDeviceMessageDispatcher;
using STI::Network::convert;
using STI::Network::RemoteEventEngineScheduler;
using STI::Device::ChannelManager;
using STI::Network::RemoteChannelManager;
using STI::Network::RemoteAttributeManager;


RemoteDevice::RemoteDevice(::STI::TNetwork::TDevice_ptr device)
	: _tDevice(STI::TNetwork::TDevice::_duplicate(device))
{
}



bool RemoteDevice::getTDeviceRef(STI::TNetwork::TDevice_ptr& tDevice)
{
	STI::TNetwork::TDevice_var newDev;
	newDev = _tDevice;		//implicit duplicate

	tDevice = newDev.out();

	//tDevice = STI::TNetwork::TDeviceHub::_duplicate();
	//tDevice = _tDevice;
	//tDevice = _tDevice->_duplicate(_tDevice);
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

	if (CORBA::is_nil(_tDevice)) return false;

	bool success = false;

	try {
		success = _tDevice->refresh();
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

const STI::Device::DeviceID RemoteDevice::getID() const
{
	std::unique_lock<std::mutex> deviceLock(deviceMutex);

	::STI::TNetwork::TDeviceID_var tDeviceID;

	bool success = false;

	if (!CORBA::is_nil(_tDevice)) {
		try {
			tDeviceID = _tDevice->getID();
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

	if (CORBA::is_nil(_tDevice)) return;
	
	bool success = false;

	::STI::TNetwork::TDeviceCollection_ptr tDeviceCollection;	//remote reference
	// std::shared_ptr<RemoteDeviceCollection> remoteCollection;	//wrapper

	try {
		tDeviceCollection = _tDevice->getDeviceCollection();	//remote call
		success = true;

		if (success && !CORBA::is_nil(tDeviceCollection)) {
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

	if (CORBA::is_nil(_tDevice)) return;

	bool success = false;

	::STI::TNetwork::TDeviceMessageDispatcher_ptr tMessageDispatcher;	//remote reference
	// std::shared_ptr<RemoteDeviceMessageDispatcher> remoteDispatcher;		//wrapper

	try {
		tMessageDispatcher = _tDevice->getMessageDispatcher();	//remote call
		success = true;
		
		if (success && !CORBA::is_nil(tMessageDispatcher)) {
			remoteDispatcher = std::make_shared<RemoteDeviceMessageDispatcher>(tMessageDispatcher);
			dispatcher = remoteDispatcher;
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

	if (CORBA::is_nil(_tDevice)) return false;

	bool success = false;

	::STI::TNetwork::TEventEngineScheduler_ptr tEngineScheduler;	//remote reference
	// std::shared_ptr<RemoteEventEngineScheduler> remoteScheduler;	//wrapper

	try {
		tEngineScheduler = _tDevice->getEngineScheduler();	//remote call
		success = true;
		
		if (success && !CORBA::is_nil(tEngineScheduler)) {
			remoteScheduler = std::make_shared<RemoteEventEngineScheduler>(tEngineScheduler);
			scheduler = remoteScheduler;
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

	if (CORBA::is_nil(_tDevice)) return;

	if (listenerForwarder == 0) return;

	bool success = false;

	::STI::TNetwork::TChannelManager_ptr tChannelManager;	//remote reference
	
	try {
		tChannelManager = _tDevice->getChannelManager();	//remote call

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

	if (CORBA::is_nil(_tDevice)) return;

	if (listenerForwarder == 0) return;

	::STI::TNetwork::TAttributeManager_ptr tAttributelManager;	//remote reference
	
	try {
		tAttributelManager = _tDevice->getAttributeManager();	//remote call

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

