
#include "TDevice_i.h"
#include "ORBManager.h"
#include "DeviceID.h"
#include "NetworkConvert.h"

using STI::TNetwork::TDevice_i;
using STI::TNetwork::TDeviceCollection_ptr;
using STI::TNetwork::TDeviceEventDispatcher_ptr;
using STI::TNetwork::TEventEngineScheduler_ptr;
using STI::TNetwork::TDeviceID;
using STI::Device::DeviceID;
using STI::Network::convert;

//Device is a DeviceCollector, so we can simply pass the device pointer to both
TDevice_i::TDevice_i(const std::shared_ptr<STI::Device::Device>& device)
	: localDevice(device), deviceCollectionServant(device), eventDispatcherServant(device), eventSchedulerServant(device)
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

TDeviceCollection_ptr TDevice_i::getDeviceCollection()
{
	return deviceCollectionServant._this();
}

TDeviceEventDispatcher_ptr TDevice_i::getEventDispatcher()
{
	return eventDispatcherServant._this();
}

TEventEngineScheduler_ptr TDevice_i::getEngineScheduler()
{
	return eventSchedulerServant._this();
}

TDeviceID* TDevice_i::getID()
{
	STI::TNetwork::TDeviceID_var tDevice(new STI::TNetwork::TDeviceID);

	if(localDevice != 0) {
		convert<DeviceID, TDeviceID>(localDevice->getID(), tDevice);
	}

	return tDevice._retn();
}

void TDevice_i::write(::CORBA::ULong input) 
{ 
	localDevice->write(input);
}

