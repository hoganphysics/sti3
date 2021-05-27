
#include "TDeviceHub_i.h"
#include "HubID.h"
#include "DeviceID.h"
#include "RemoteDevice.h"
#include "RemoteDeviceHub.h"

#include "ORBManager.h"
#include "NetworkConvert.h"
#include "Convert_HubNodeWalker.h"
#include "orbTypes.h"


using STI::Network::RemoteDeviceHub;
using STI::Network::RemoteDevice;
using STI::TNetwork::TDeviceHub_i;
using STI::TNetwork::TDeviceHubID;
using STI::TNetwork::TDeviceHubTrace;
using STI::TNetwork::TDeviceID;
using STI::Network::convert;

TDeviceHub_i::TDeviceHub_i(const std::shared_ptr<STI::Network::LocalDeviceHub>& hub)
	: localHub(hub)
{
}

TDeviceHub_i::~TDeviceHub_i()
{
	STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

::CORBA::Boolean TDeviceHub_i::addHub(const TDeviceHubID& hubID, ::STI::TNetwork::TDeviceHub_ptr hub)
{
	bool success = false;
	std::shared_ptr<RemoteDeviceHub> remoteDeviceHub;

	//wrap the received TDeviceHub reference in RemoteDevice
	if (hub != 0) {
		remoteDeviceHub = std::make_shared<RemoteDeviceHub>(hub);
	}

	if (localHub != 0 && remoteDeviceHub != 0) {

		success = localHub->addHub(
			convert<TDeviceHubID, STI::Network::HubID>(hubID),
			remoteDeviceHub			
		);
	}
	return success;
}

::CORBA::Boolean TDeviceHub_i::removeHub(const TDeviceHubID& hubID)
{
	bool success = false;
	if (localHub != 0) {

		//If this is a RemoteHub stored locally, then this TDeviceHub_i is its owner.
		//Attempt to resolve the RemoteDeviceHub and disable it to free references.
		std::shared_ptr<STI::Network::DeviceHub> hub;
		if (localHub->getHub(convert<TDeviceHubID, STI::Network::HubID>(hubID), hub) && hub != 0) {
			auto remoteHub = std::dynamic_pointer_cast<RemoteDeviceHub>(hub);

			if (remoteHub != 0) {
				remoteHub->disable();
			}
		}

		success = localHub->removeHub(
			convert<TDeviceHubID, STI::Network::HubID>(hubID)
		);
	}
	return success;
}

::CORBA::Boolean TDeviceHub_i::refreshNetwork(const TDeviceHubTrace& trace)
{
	bool success = false;
	if (localHub != 0) {

		success = localHub->refresh(
			convert<TDeviceHubTrace, STI::Network::HubTrace>(trace)
		);
	}
	return success;
}

::CORBA::Boolean TDeviceHub_i::distribute(const TDeviceID& devID, ::STI::TNetwork::TDevice_ptr node, 
	const TDeviceHubTrace& trace, const TDeviceHubID& first)
{
	bool success = false;
	std::shared_ptr<RemoteDevice> remoteDevice;

	//wrap the received TDevice reference in RemoteDevice
	if (node != 0) {
		remoteDevice = std::make_shared<RemoteDevice>(node);
	}

	if (localHub != 0 && remoteDevice != 0) {

		success = localHub->distribute(
			convert<TDeviceID, STI::Device::DeviceID>(devID),
			remoteDevice,
			convert<TDeviceHubTrace, STI::Network::HubTrace>(trace),
			convert<TDeviceHubID, STI::Network::HubID>(first)
		);
	}
	return success;
}

::CORBA::Boolean TDeviceHub_i::distributeNodes(const TDeviceHubID& targetHub)
{
	bool success = false;
	if (localHub != 0) {

		success = localHub->distributeNodes(
			convert<TDeviceHubID, STI::Network::HubID>(targetHub)
		);
	}
	return success;
}

::CORBA::Boolean TDeviceHub_i::redistributeNodes(const TDeviceHubTrace& trace)
{
	bool success = false;
	if (localHub != 0) {

		success = localHub->redistributeNodes(
			convert<TDeviceHubTrace, STI::Network::HubTrace>(trace)
		);
	}
	return success;
}

::CORBA::Boolean TDeviceHub_i::removeNode(const TDeviceID& devID, const TDeviceHubTrace& trace)
{
	bool success = false;
	if (localHub != 0) {
		
		success = localHub->removeNode(
			convert<TDeviceID, STI::Device::DeviceID>(devID),
			convert<TDeviceHubTrace, STI::Network::HubTrace>(trace)
			);
	}
	return success;
}

TDeviceHubID* TDeviceHub_i::deviceHubID()
{
	STI::TNetwork::TDeviceHubID_var tDeviceHub(new STI::TNetwork::TDeviceHubID);
	
	if (localHub != 0) {
		convert<STI::Network::HubID, TDeviceHubID>(localHub->getID(), tDeviceHub);
	}

	return tDeviceHub._retn();
}

void TDeviceHub_i::walk(::STI::TNetwork::TNodeWalker& root, const ::STI::TNetwork::TDeviceHubTrace& trace)
{
	if (localHub != 0) {

		//convert in values
		STI::Network::DeviceHub::HubNodeWalker nodeWalker;
		convert<STI::TNetwork::TNodeWalker, STI::Network::DeviceHub::HubNodeWalker>(root, nodeWalker);

		localHub->walk(
			nodeWalker,
			convert<TDeviceHubTrace, STI::Network::HubTrace>(trace)
		);
		
		//convert out values
		//STI::TNetwork::TNodeWalker_var tNodeWalker(new STI::TNetwork::TNodeWalker);
		convert<STI::Network::DeviceHub::HubNodeWalker, STI::TNetwork::TNodeWalker>(nodeWalker, root);

	//	root = tNodeWalker.out();
	}
}


//distribute(...)
/*
This is a bit of a compromise.  Dynamic downcasting here because we need access
to the derived object's TDevice_i servant.  This is safe because any Device pointer
stored by this Hub is guaranteed to be a NetworkDeviceWrapper (since it was added by
the NetworkDeviceHub, which always wraps adds Nodes in a NetworkDeviceWrapper).

An alternative would have been to store and maintain a separate list of TDevice_i servant 
references somewhere (say in NetworkDeviceHub), indexed by the common DeviceID, and
then retreive the TDevice_i we need when a distribute() call arrives.  This is much
more fragile (lists must be synched...), and strains encapsulation.  Instead, we store each
TDevice_i inside a NetworkDeviceWrapper that wraps the same Device being served by the 
TDevice_i, and the NetworkDeviceWrapper is simply managed by the usual DeviceCollection.

So we sacrifice some OOP purity for practicality.
*/


