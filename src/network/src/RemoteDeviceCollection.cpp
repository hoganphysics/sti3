
#include "RemoteDeviceCollection.h"
#include "NetworkConvert.h"
#include "TDeviceRefInterface.h"

#include "RemoteDevice.h"
#include "DeviceID.h"

#include "orbTypes.h"


using STI::Network::TDeviceRefInterface;
using STI::Network::RemoteDevice;
using STI::Network::RemoteDeviceCollection;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;
using STI::Network::convert;


RemoteDeviceCollection::RemoteDeviceCollection(::STI::TNetwork::TDeviceCollection_ptr deviceCollection)
	: tDeviceCollection(STI::TNetwork::TDeviceCollection::_duplicate(deviceCollection))
{
}

bool RemoteDeviceCollection::add(const STI::Device::DeviceID& id, const std::shared_ptr<STI::Device::Device>& node)
{
	STI::TNetwork::TDevice_ptr tDevice;

	if (!TDeviceRefInterface::getTDeviceReference(node, tDevice)) {
		return false;
	}

//	STI::TNetwork::TDevice_var tDevicevar = tDevice;

	bool success = false;

	//std::cout << "RemoteDeviceCollection::add( " << CORBA::is_nil(tDevice) << " )" << std::endl;

	try {
		success = tDeviceCollection->add(convert<DeviceID, TDeviceID>(id), tDevice);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	//std::cout << "after tDeviceCollection->add" << std::endl;

	return success;
}

bool RemoteDeviceCollection::remove(const STI::Device::DeviceID& id)
{
	bool success = false;

	try {
		success = tDeviceCollection->remove(convert<DeviceID, TDeviceID>(id));	//remote call
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

bool RemoteDeviceCollection::contains(const STI::Device::DeviceID& id) const
{
	bool success = false;

	try {
		success = tDeviceCollection->contains(convert<DeviceID, TDeviceID>(id));	//remote call
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

unsigned RemoteDeviceCollection::size() const
{
	unsigned result = 0;
	try {
		result = tDeviceCollection->size();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return result;
}

bool RemoteDeviceCollection::get(const STI::Device::DeviceID& id, std::shared_ptr<STI::Device::Device>& node) const
{
	bool success = false;

	STI::TNetwork::TDevice_var tDevice;

	try {
		success = tDeviceCollection->get(convert<DeviceID, TDeviceID>(id), tDevice);	//remote call

		if (success && tDevice != 0 && !tDevice->_is_nil()) {
			node = std::make_shared<RemoteDevice>( STI::TNetwork::TDevice::_duplicate(tDevice) );
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

void RemoteDeviceCollection::getIDs(std::set<STI::Device::DeviceID>& ids) const
{
	STI::TNetwork::TDeviceIDSeq_var tIDs(new STI::TNetwork::TDeviceIDSeq);

	try {
		tDeviceCollection->getIDs(tIDs);	//remote call

		convert<TDeviceID, DeviceID>(tIDs, ids);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteDeviceCollection::cleanup()
{
	try {
		tDeviceCollection->cleanup();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteDeviceCollection::clear()
{
	try {
		tDeviceCollection->clear();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

