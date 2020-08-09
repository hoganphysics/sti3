
#include "TDevice_i.h"
#include "ORBManager.h"

using STI::TNetwork::TDevice_i;
using STI::TNetwork::TDeviceCollection_ptr;

//Device is a DeviceCollector, so we can simply pass the device pointer to both
TDevice_i::TDevice_i(const std::shared_ptr<STI::Device::Device>& device)
	: localDevice(device), deviceCollectionServant(device)
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

void TDevice_i::write(::CORBA::ULong input) 
{ 
	localDevice->write(input);
}

