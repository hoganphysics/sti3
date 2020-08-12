
#include "RemoteDevice.h"
#include "RemoteDeviceCollection.h"

using STI::Network::RemoteDevice;
using STI::Network::RemoteDeviceCollection;


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



bool RemoteDevice::refresh()
{
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

void RemoteDevice::write(unsigned input)
{
	try {
		_tDevice->write(input);
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
		tDeviceCollection = _tDevice->getDeviceCollection();
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

