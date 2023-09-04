#include "RemoteDeviceHub.h"
#include "NetworkConvert.h"
#include "convert/Convert_HubNodeWalker.h"

#include "NetworkDeviceHubWrapper.h"
#include "TDeviceRefInterface.h"

#include <sti/network/DeviceHub.h>
#include <sti/network/HubID.h>
#include <sti/network/HubTrace.h>

#include "generated/orbTypes.h"

using STI::Network::HubID;
using STI::Network::DeviceHub;
using STI::Network::RemoteDeviceHub;
using STI::Network::HubID;
using STI::Network::HubTrace;
using STI::Network::convert;
using STI::Network::TDeviceRefInterface;
using STI::Network::NetworkDeviceHubWrapper;
using STI::TNetwork::TDeviceHubID;
using STI::Network::NodeWalker;


RemoteDeviceHub::RemoteDeviceHub(::STI::TNetwork::TDeviceHub_ptr deviceHub)
: STI::TNetwork::TReferenceHolder<STI::TNetwork::TDeviceHub>(deviceHub, hubMutex)
{
	_getHubID();	// network call to get HubID once and save locally
}


bool RemoteDeviceHub::addHub(const HubID& id, const std::shared_ptr<DeviceHub>& hub)
{
	std::unique_lock<std::mutex> hubLock(hubMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TDeviceHub_var tDeviceHubRef;

	if (!NetworkDeviceHubWrapper::getTDeviceHubReference(hub, tDeviceHubRef)) {
		return false;
	}

	STI::TNetwork::TDeviceHub_var tDeviceHubRefvar = tDeviceHubRef;

	bool success = false;

	try {
		success = getTRef()->addHub(convert<HubID, TDeviceHubID>(id), tDeviceHubRefvar);	//remote call
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
	std::unique_lock<std::mutex> hubLock(hubMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->removeHub(
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
	std::unique_lock<std::mutex> hubLock(hubMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->removeNode(
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
	std::unique_lock<std::mutex> hubLock(hubMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->refreshNetwork(
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


bool RemoteDeviceHub::distribute(const STI::Device::DeviceID& id,
	const typename std::shared_ptr<STI::Device::Device>& node,
	const HubTrace& trace, const HubID& first)
{
	std::unique_lock<std::mutex> hubLock(hubMutex);

	if (isDisabled()) return false;

	bool success = false;

	STI::TNetwork::TDevice_var tDevice;

	if (!TDeviceRefInterface::getTDeviceReference(node, tDevice)) {
			return false;
	}

	try {
		success = getTRef()->distribute(
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
	std::unique_lock<std::mutex> hubLock(hubMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->distributeNodes(
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
	std::unique_lock<std::mutex> hubLock(hubMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->redistributeNodes(
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

const HubID& RemoteDeviceHub::getID() const
{
	return hubID;
}

void RemoteDeviceHub::_getHubID()
{
	std::unique_lock<std::mutex> hubLock(hubMutex);

	if (isDisabled()) return;

	using STI::TNetwork::TDeviceHubID_var;
	
	TDeviceHubID_var tHubID;

	bool success = false;

	try {
		tHubID = getTRef()->deviceHubID();	//remote call
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
}

bool RemoteDeviceHub::hasNodeID(const STI::Device::DeviceID& id) const
{
	std::unique_lock<std::mutex> hubLock(hubMutex);

	if (isDisabled()) return false;

	bool found = false;

	try {
		found = getTRef()->hasNodeID(
			convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(id)
		);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return found;
}

void RemoteDeviceHub::walk(NodeWalker<STI::Device::DeviceID, STI::Device::Device>& root, const HubTrace& trace) const
{
	std::unique_lock<std::mutex> hubLock(hubMutex);

	if (isDisabled()) return;

	bool success = false;

	//convert in values
	STI::TNetwork::TNodeWalker_var tRoot(new STI::TNetwork::TNodeWalker);

	tRoot->connections.length(0);

	convert<STI::Network::DeviceHub::HubNodeWalker, STI::TNetwork::TNodeWalker>(root, tRoot);

	try {
		getTRef()->walk(
			tRoot,
			convert<STI::Network::HubTrace, STI::TNetwork::TDeviceHubTrace>(trace)
		);	//remote call

		success = true;
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	//convert out values
	if (success) {
		convert<STI::TNetwork::TNodeWalker, STI::Network::DeviceHub::HubNodeWalker>(tRoot, root);
	}
}

