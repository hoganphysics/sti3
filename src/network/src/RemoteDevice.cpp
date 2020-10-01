
#include "RemoteDevice.h"
#include "RemoteDeviceCollection.h"
#include "RemoteDeviceEventDispatcher.h"
#include "NetworkConvert.h"

using STI::Network::RemoteDevice;
using STI::Network::RemoteDeviceCollection;
using STI::Network::RemoteDeviceEventDispatcher;
using STI::Network::convert;

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


/// Check if the RemoteDevice reference is still live.  Attempts a call over the network
/// and returns false if it times out or there is some other error.
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

STI::Device::DeviceID RemoteDevice::getID()
{
	::STI::TNetwork::TDeviceID_var tDeviceID;

	bool success = false;

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

	// if(!success) {
	// 	tDeviceID = new ::STI::TNetwork::TDeviceID();
	// }

	STI::Device::DeviceID deviceID;

	if(success) {
		deviceID = convert<::STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tDeviceID);
	}

	return deviceID;
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

void RemoteDevice::getEventDispatcher(std::shared_ptr<STI::Device::DeviceEventDispatcher>& dispatcher)
{
	bool success = false;

	::STI::TNetwork::TDeviceEventDispatcher_ptr tEventDispatcher;	//remote reference
	std::shared_ptr<RemoteDeviceEventDispatcher> remoteDispatcher;		//wrapper

	try {
		tEventDispatcher = _tDevice->getEventDispatcher();
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
		remoteDispatcher = std::make_shared<RemoteDeviceEventDispatcher>(tEventDispatcher);
		dispatcher = remoteDispatcher;
	}
}

