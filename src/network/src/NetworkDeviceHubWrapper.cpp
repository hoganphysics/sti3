#include "NetworkDeviceHubWrapper.h"

#include <sti/device/DeviceID.h>
#include <sti/network/HubID.h>
#include <sti/network/HubTrace.h>

#include "NetworkDevice.h"
#include "generated/orbTypes.h"
#include "ORBManager.h"

#include <memory>

using STI::Network::NetworkDevice;
using STI::Network::NetworkDeviceHubWrapper;
using STI::Network::DeviceHub;
using STI::Network::HubID;
using STI::Network::HubTrace;
using STI::Network::LocalHub;
using STI::Network::NodeWalker;


NetworkDeviceHubWrapper::NetworkDeviceHubWrapper(const std::shared_ptr<LocalDeviceHub>& hub)
	: localHub(hub), deviceHubServant(hub)
{
	STI::Network::ORBManager::ORBManager::activateServant(deviceHubServant);

	//The LocalHub might have (local) nodes and hubs already attached that must be wrapped.
	//For all hubs currently stored by localHub, replace with NetworkDeviceHubWrapper (this is recursive)

	std::set<STI::Network::HubID> hubIDs;
	localHub->getHubIDs(hubIDs);
	std::shared_ptr<DeviceHub> attachedHub;
	std::shared_ptr<LocalDeviceHub> attachedLocalHub;
	std::shared_ptr<DeviceHub> wrappedHub;

	for (auto& hubID : hubIDs) {
		if (localHub->getHub(hubID, attachedHub) && attachedHub != 0) {

			//Check if the attached hub is local
			attachedLocalHub = std::dynamic_pointer_cast<LocalDeviceHub>(attachedHub);
			
			if (attachedLocalHub != 0) {
				//this is a LocalHub; wrap it.
				wrappedHub = std::make_shared<NetworkDeviceHubWrapper>(attachedLocalHub);	//recursive call
				addHub(hubID, wrappedHub);	//replaces with new wrapped version
			}

		}
	}

	//For all stored nodes, re-add (replace) with NetworkDevice

	std::set<STI::Device::DeviceID> nodeIDs;
	localHub->getNodeIDs(nodeIDs);
	std::shared_ptr<STI::Device::Device> node;

	for (auto& id : nodeIDs) {
		if (localHub->getNode(id, node) && node != 0) {
			addNode(id, node);		//replaces with NetworkDevice version
		}
	}
}

NetworkDeviceHubWrapper::~NetworkDeviceHubWrapper()
{
	//STI::TNetwork::TDeviceHub_i deviceHubServant;
//	deviceHubServant._remove_ref();
}

bool NetworkDeviceHubWrapper::addNode(const STI::Device::DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node)
{
	if (localHub != 0) {

		std::shared_ptr<NetworkDevice> wrappedNode = std::make_shared<NetworkDevice>(node);
		return localHub->addNode(id, wrappedNode);
	}
	return false;
}


bool NetworkDeviceHubWrapper::addHub(const HubID& id, const typename std::shared_ptr<DeviceHub>& hub)
{
	if (localHub != 0) {
		return localHub->addHub(id, hub);
	}
	return false;
}

bool NetworkDeviceHubWrapper::removeHub(const HubID& id)
{
	if (localHub != 0) {
		return localHub->removeHub(id);
	}
	return false;
}

bool NetworkDeviceHubWrapper::removeNode(const STI::Device::DeviceID& id)
{
	HubTrace trace;
	return removeNode(id, trace);
}

bool NetworkDeviceHubWrapper::removeNode(const STI::Device::DeviceID& id, const HubTrace& trace)
{
	if (localHub != 0) {
		return localHub->removeNode(id, trace);
	}
	return false;
}

bool NetworkDeviceHubWrapper::refresh(const HubTrace& trace)
{
	if (localHub != 0) {
		return localHub->refresh(trace);
	}
	return false;
}

bool NetworkDeviceHubWrapper::distribute(const STI::Device::DeviceID& id, 
	const typename std::shared_ptr<STI::Device::Device>& node,
	const HubTrace& trace, const HubID& first)
{
	if (localHub != 0) {
		return localHub->distribute(id, node, trace, first);
	}
	return false;
}

bool NetworkDeviceHubWrapper::distributeNodes(const HubID& targetHub)
{
	if (localHub != 0) {
		return localHub->distributeNodes(targetHub);
	}
	return false;
}


bool NetworkDeviceHubWrapper::redistributeNodes(const HubTrace& trace)
{
	if (localHub != 0) {
		return localHub->redistributeNodes(trace);
	}
	return false;
}

const HubID& NetworkDeviceHubWrapper::getID() const
{
	return localHub->getID();
}

bool NetworkDeviceHubWrapper::hasNodeID(const STI::Device::DeviceID& id) const
{
	return localHub->hasNodeID(id);
}

void NetworkDeviceHubWrapper::walk(NodeWalker<STI::Device::DeviceID, STI::Device::Device>& root, const HubTrace& trace) const
{
	localHub->walk(root, trace);
}


bool NetworkDeviceHubWrapper::getTDeviceHubReference(const typename std::shared_ptr<DeviceHub>& deviceHub,
	STI::TNetwork::TDeviceHub_var& tDeviceHub)
{
	if (deviceHub == 0) return false;

	bool success = false;
	std::shared_ptr<NetworkDeviceHubWrapper> networkDeviceHubWrapper;
	networkDeviceHubWrapper = std::dynamic_pointer_cast<NetworkDeviceHubWrapper>(deviceHub);

	if (networkDeviceHubWrapper) {		//check dynamic_pointer_cast

		tDeviceHub = networkDeviceHubWrapper->deviceHubServant._this();
		success = true;
	}

	return success && !CORBA::is_nil(tDeviceHub);
}
