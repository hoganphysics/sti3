
#include "TDevice_i.h"
#include "ORBManager.h"

using STI::TNetwork::TDevice_i;
using STI::TNetwork::TDeviceCollection_ptr;
using STI::TNetwork::TDeviceEventDispatcher_ptr;


//Device is a DeviceCollector, so we can simply pass the device pointer to both
TDevice_i::TDevice_i(const std::shared_ptr<STI::Device::Device>& device)
	: localDevice(device), deviceCollectionServant(device), eventDispatcherServant(device)
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


void TDevice_i::write(::CORBA::ULong input) 
{ 
	localDevice->write(input);
}

