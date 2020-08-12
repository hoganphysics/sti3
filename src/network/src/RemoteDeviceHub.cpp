
#include "RemoteDeviceHub.h"
#include "NetworkConvert.h"

#include "NetworkDeviceHubWrapper.h"
#include "TDeviceRefInterface.h"

#include "DeviceHub.h"
#include "HubID.h"
#include "HubTrace.h"

#include "orbTypes.h"

using STI::Network::HubID;
using STI::Network::DeviceHub;
using STI::Network::RemoteDeviceHub;
using STI::Network::HubID;
using STI::Network::HubTrace;
using STI::Network::convert;
using STI::Network::TDeviceRefInterface;
using STI::Network::NetworkDeviceHubWrapper;
using STI::TNetwork::TDeviceHubID;

RemoteDeviceHub::RemoteDeviceHub(::STI::TNetwork::TDeviceHub_ptr deviceHub)
	: tDeviceHub(STI::TNetwork::TDeviceHub::_duplicate(deviceHub))
{
}

//void RemoteDeviceHub::getNodeIDs(std::set<STI::Device::DeviceID>& ids) const
//{
//	try {
//		tDeviceHub->getNodeIDs(
//			
//		);	//remote call
//	}
//	catch (CORBA::TRANSIENT&) {
//	}
//	catch (CORBA::SystemException&) {
//	}
//	catch (CORBA::Exception&)
//	{
//	}
//}
//
//void RemoteDeviceHub::getHubIDs(std::set<HubID>& ids) const
//{
//	try {
//		tDeviceHub->getHubIDs(
//
//		);	//remote call
//	}
//	catch (CORBA::TRANSIENT&) {
//	}
//	catch (CORBA::SystemException&) {
//	}
//	catch (CORBA::Exception&)
//	{
//	}
//}

bool RemoteDeviceHub::addHub(const HubID& id, const std::shared_ptr<DeviceHub>& hub)
{
//	return false;

	STI::TNetwork::TDeviceHub_ptr tDeviceHubRef;

	if (!NetworkDeviceHubWrapper::getTDeviceHubReference(hub, tDeviceHubRef)) {
		return false;
	}

	bool success = false;

	try {
		success = tDeviceHub->addHub(convert<HubID, TDeviceHubID>(id), tDeviceHubRef);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException& ex) {
		std::string temp = ex._name();
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}


bool RemoteDeviceHub::removeHub(const HubID& id)
{
	bool success = false;

	try {
		success = tDeviceHub->removeHub(
			convert<STI::Network::HubID, STI::TNetwork::TDeviceHubID>(id)
		);	//remote call
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


bool RemoteDeviceHub::removeNode(const STI::Device::DeviceID& id, const HubTrace& trace)
{
	bool success = false;

	try {
		success = tDeviceHub->removeNode(
			convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(id),
			convert<STI::Network::HubTrace, STI::TNetwork::TDeviceHubTrace>(trace)
		);	//remote call
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


bool RemoteDeviceHub::refresh(const HubTrace& trace)
{
	bool success = false;

	try {
		success = tDeviceHub->refreshNetwork(
			convert<STI::Network::HubTrace, STI::TNetwork::TDeviceHubTrace>(trace)
		);	//remote call
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


//bool RemoteDeviceHub::getRemoteDevice(const typename std::shared_ptr<STI::Device::Device>& device, STI::TNetwork::TDevice_ptr& tDevice)
//{
//	std::shared_ptr<NetworkDeviceWrapper> networkDeviceWrapper;
//	networkDeviceWrapper = std::dynamic_pointer_cast<NetworkDeviceWrapper>(device);
//
//	if (networkDeviceWrapper) {		//check dynamic_pointer_cast
//	//	tDevice = networkDeviceWrapper->getTDeviceReference();
//	}
//	return (tDevice !=0 && !tDevice->_is_nil());
//}

bool RemoteDeviceHub::distribute(const STI::Device::DeviceID& id,
	const typename std::shared_ptr<STI::Device::Device>& node,
	const HubTrace& trace, const HubID& first)
{
	bool success = false;

	STI::TNetwork::TDevice_ptr tDevice;


	//if (!getRemoteDevice(node, tDevice)) {
	if (!TDeviceRefInterface::getTDeviceReference(node, tDevice)) {
			return false;
	}

	try {
		success = tDeviceHub->distribute(
			convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(id),
			tDevice,
			convert<STI::Network::HubTrace, STI::TNetwork::TDeviceHubTrace>(trace),
			convert<STI::Network::HubID, STI::TNetwork::TDeviceHubID>(first)

		);	//remote call
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

bool RemoteDeviceHub::distributeNodes(const HubID& targetHub)
{
	bool success = false;

	try {
		success = tDeviceHub->distributeNodes(
			convert<STI::Network::HubID, STI::TNetwork::TDeviceHubID>(targetHub)
		);	//remote call
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

bool RemoteDeviceHub::redistributeNodes(const HubTrace& trace)
{
	bool success = false;

	try {
		success = tDeviceHub->redistributeNodes(
			convert<STI::Network::HubTrace, STI::TNetwork::TDeviceHubTrace>(trace)
		);	//remote call
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


const HubID& RemoteDeviceHub::getID()
{
	using STI::TNetwork::TDeviceHubID_var;
	
	TDeviceHubID_var tHubID;

	bool success = false;

	try {
		tHubID = tDeviceHub->deviceHubID();	//remote call
		success = true;

		if (success) {
			hubID = convert<STI::TNetwork::TDeviceHubID, HubID>(tHubID);
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return hubID;
}
