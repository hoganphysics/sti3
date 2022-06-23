
#include "TDevice_i.h"
#include "ORBManager.h"
#include <sti/device/DeviceID.h>
#include "NetworkConvert.h"

using STI::TNetwork::TDevice_i;
using STI::TNetwork::TDeviceCollection_ptr;
using STI::TNetwork::TDeviceMessageDispatcher_ptr;
using STI::TNetwork::TEventEngineScheduler_ptr;
using STI::TNetwork::TChannelManager_ptr;
using STI::TNetwork::TAttributeManager_ptr;
using STI::TNetwork::TPersistenceManager_ptr;
using STI::TNetwork::TDeviceID;
using STI::Device::DeviceID;
using STI::Network::convert;


//Device is a DeviceCollector, so we can simply pass the device pointer to both
TDevice_i::TDevice_i(const std::shared_ptr<STI::Device::Device>& device)
	: localDevice(device), deviceCollectionServant(device), messageDispatcherServant(device), 
		eventSchedulerServant(device), channelManagerServant(device), attributeManagerServant(device), 
		persistenceManagerServant(device)
{
}

TDevice_i::~TDevice_i()
{
	STI::Network::ORBManager::ORBManager::deactivateServant(this);
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
	return deviceCollectionServant._this();
}

TDeviceMessageDispatcher_ptr TDevice_i::getMessageDispatcher()
{
	return messageDispatcherServant._this();
}

TEventEngineScheduler_ptr TDevice_i::getEngineScheduler()
{
	return eventSchedulerServant._this();
}

TChannelManager_ptr TDevice_i::getChannelManager()
{
	return channelManagerServant._this();
}

TAttributeManager_ptr TDevice_i::getAttributeManager()
{
	return attributeManagerServant._this();	
}

TPersistenceManager_ptr TDevice_i::getPersistenceManager()
{
	return persistenceManagerServant._this();
}

TDeviceID* TDevice_i::getID()
{
	STI::TNetwork::TDeviceID_var tDevice(new STI::TNetwork::TDeviceID);

	if(localDevice != 0) {
		convert<DeviceID, TDeviceID>(localDevice->getID(), tDevice);
	}

	return tDevice._retn();
}

