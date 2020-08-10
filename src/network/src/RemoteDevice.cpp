
#include "RemoteDevice.h"
#include "RemoteDeviceCollection.h"

using STI::Network::RemoteDevice;
using STI::Network::RemoteDeviceCollection;


RemoteDevice::RemoteDevice(::STI::TNetwork::TDevice_ptr device)
	: tDevice(device)
{
}

bool RemoteDevice::refresh()
{
	bool success = false;

	try {
		success = tDevice->refresh();
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

void RemoteDevice::write(unsigned input)
{
	try {
		tDevice->write(input);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteDevice::getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection)
{
	bool success = false;

	::STI::TNetwork::TDeviceCollection_ptr tDeviceCollection;	//remote reference
	std::shared_ptr<RemoteDeviceCollection> remoteCollection;	//wrapper

	try {
		tDeviceCollection = tDevice->getDeviceCollection();
		success = true;
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	if (success) {
		remoteCollection = std::make_shared<RemoteDeviceCollection>(tDeviceCollection);
		collection = remoteCollection;
	}
}

