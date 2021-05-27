
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
using STI::TNetwork::TDeviceCollection;
using STI::TNetwork::TReferenceHolder;


RemoteDeviceCollection::RemoteDeviceCollection(::STI::TNetwork::TDeviceCollection_ptr deviceCollection)
: TReferenceHolder<TDeviceCollection>(deviceCollection, collectionMutex)
//: tDeviceCollection(STI::TNetwork::TDeviceCollection::_duplicate(deviceCollection))
{
}

bool RemoteDeviceCollection::add(const STI::Device::DeviceID& id, const std::shared_ptr<STI::Device::Device>& node)
{
	std::unique_lock<std::mutex> collectionLock(collectionMutex);

	if (isDisabled()) return false;

	// STI::TNetwork::TDevice_ptr tDevice;

	// if (!TDeviceRefInterface::getTDeviceReference(node, tDevice)) {
	// 	return false;
	// }

	STI::TNetwork::TDevice_var tDevice;

	if (!TDeviceRefInterface::getTDeviceReference(node, tDevice)) {
		return false;
	}


//	STI::TNetwork::TDevice_var tDevicevar = tDevice;

	bool success = false;

	//std::cout << "RemoteDeviceCollection::add( " << CORBA::is_nil(tDevice) << " )" << std::endl;

	try {
		success = getTRef()->add(convert<DeviceID, TDeviceID>(id), tDevice);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	//std::cout << "after getTRef()->add" << std::endl;

	return success;
}

bool RemoteDeviceCollection::remove(const STI::Device::DeviceID& id)
{
	std::unique_lock<std::mutex> collectionLock(collectionMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->remove(convert<DeviceID, TDeviceID>(id));	//remote call
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
	std::unique_lock<std::mutex> collectionLock(collectionMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->contains(convert<DeviceID, TDeviceID>(id));	//remote call
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
	std::unique_lock<std::mutex> collectionLock(collectionMutex);

	unsigned result = 0;

	if (isDisabled()) return result;

	try {
		result = getTRef()->size();	//remote call
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
	std::unique_lock<std::mutex> collectionLock(collectionMutex);

	if (isDisabled()) return false;

	bool success = false;

	STI::TNetwork::TDevice_var tDevice;

	try {
		success = getTRef()->get(convert<DeviceID, TDeviceID>(id), tDevice);	//remote call

		if (success && tDevice != 0 && !tDevice->_is_nil()) {
			//node = std::make_shared<RemoteDevice>( STI::TNetwork::TDevice::_duplicate(tDevice) );
			node = std::make_shared<RemoteDevice>(tDevice);
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
	std::unique_lock<std::mutex> collectionLock(collectionMutex);

	if (isDisabled()) return;
	
	STI::TNetwork::TDeviceIDSeq_var tIDs(new STI::TNetwork::TDeviceIDSeq);

	try {
		getTRef()->getIDs(tIDs);	//remote call

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
	std::unique_lock<std::mutex> collectionLock(collectionMutex);

	if (isDisabled()) return;

	try {
		getTRef()->cleanup();	//remote call
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
	std::unique_lock<std::mutex> collectionLock(collectionMutex);

	if (isDisabled()) return;

	try {
		getTRef()->clear();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

bool RemoteDeviceCollection::ping() const
{
	std::unique_lock<std::mutex> collectionLock(collectionMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->ping();	//remote call
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

