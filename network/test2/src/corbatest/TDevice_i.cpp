
#include "TDevice_i.h"

using STI::TNetwork::TDevice_i;
using STI::TNetwork::TDeviceCollection_ptr;

//Device is a DeviceCollector, so we can simply pass the device pointer to both
TDevice_i::TDevice_i(const std::shared_ptr<STI::Device::Device>& device)
	: localDevice(device), deviceCollectionServant(device) {}

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

