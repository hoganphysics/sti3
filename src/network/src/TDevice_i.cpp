#include "TDevice_i.h"
// #include "ORBManager.h"
#include <sti/device/DeviceID.h>
#include "NetworkConvert.h"

using STI::TNetwork::TDevice_i;
using STI::TNetwork::TDeviceCollection_ptr;
using STI::TNetwork::TDeviceMessageDispatcher_ptr;
using STI::TNetwork::TEventEngineScheduler_ptr;
using STI::TNetwork::TChannelManager_ptr;
using STI::TNetwork::TAttributeManager_ptr;
using STI::TNetwork::TPersistenceManager_ptr;
using STI::TNetwork::TProfileManager_ptr;
using STI::TNetwork::TTaskManager_ptr;
using STI::TNetwork::TLogManager_ptr;
using STI::TNetwork::TDeviceID;
using STI::Device::DeviceID;
using STI::Network::convert;


//Device is a DeviceCollector, so we can simply pass the device pointer to both
TDevice_i::TDevice_i(const std::shared_ptr<STI::Device::Device>& device)
: localDevice(device), 
deviceCollectionServantHolder(new STI::TNetwork::TDeviceCollection_i(device)), 
messageDispatcherServantHolder(new STI::TNetwork::TDeviceMessageDispatcher_i(device)), 
eventSchedulerServantHolder(new STI::TNetwork::TEventEngineScheduler_i(device)), 
channelManagerServantHolder(new STI::TNetwork::TChannelManager_i(device)), 
attributeManagerServantHolder(new STI::TNetwork::TAttributeManager_i(device)), 
persistenceManagerServantHolder(new STI::TNetwork::TPersistenceManager_i(device)), 
profileManagerServantHolder(new STI::TNetwork::TProfileManager_i(device)), 
taskManagerServantHolder(new STI::TNetwork::TTaskManager_i(device)), 
logManagerServantHolder(new STI::TNetwork::TLogManager_i(device))
{
	// STI::Network::ORBManager::ORBManager::activateServant(attributeManagerServant);
	// STI::Network::ORBManager::ORBManager::activateServant(channelManagerServant);
	// STI::Network::ORBManager::ORBManager::activateServant(deviceCollectionServant);
	// STI::Network::ORBManager::ORBManager::activateServant(messageDispatcherServant);
	// STI::Network::ORBManager::ORBManager::activateServant(eventSchedulerServant);
	// STI::Network::ORBManager::ORBManager::activateServant(persistenceManagerServant);
	// STI::Network::ORBManager::ORBManager::activateServant(profileManagerServant);
	// STI::Network::ORBManager::ORBManager::activateServant(taskManagerServant);
	// STI::Network::ORBManager::ORBManager::activateServant(logManagerServant);
}

TDevice_i::~TDevice_i()
{
	// STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

::CORBA::Boolean TDevice_i::refresh()
{
	return localDevice->refresh();
}

void TDevice_i::disable()
{
	return localDevice->disable();
}

void TDevice_i::kill()
{
	return localDevice->kill();
}

TDeviceCollection_ptr TDevice_i::getDeviceCollection()
{
	return deviceCollectionServantHolder.getRefPtr();
}

TDeviceMessageDispatcher_ptr TDevice_i::getMessageDispatcher()
{
	return messageDispatcherServantHolder.getRefPtr();
}

TEventEngineScheduler_ptr TDevice_i::getEngineScheduler()
{
	return eventSchedulerServantHolder.getRefPtr();
}

TChannelManager_ptr TDevice_i::getChannelManager()
{
	return channelManagerServantHolder.getRefPtr();
}

TAttributeManager_ptr TDevice_i::getAttributeManager()
{
	return attributeManagerServantHolder.getRefPtr();	
}

TPersistenceManager_ptr TDevice_i::getPersistenceManager()
{
	return persistenceManagerServantHolder.getRefPtr();
}

TProfileManager_ptr TDevice_i::getProfileManager()
{
	return profileManagerServantHolder.getRefPtr();
}

TTaskManager_ptr TDevice_i::getTaskManager()
{
	return taskManagerServantHolder.getRefPtr();
}

TLogManager_ptr TDevice_i::getLogManager()
{
	return logManagerServantHolder.getRefPtr();
}

TDeviceID* TDevice_i::getID()
{
	STI::TNetwork::TDeviceID_var tDevice(new STI::TNetwork::TDeviceID);

	if(localDevice != 0) {
		convert<DeviceID, TDeviceID>(localDevice->getID(), tDevice);
	}

	return tDevice._retn();
}

