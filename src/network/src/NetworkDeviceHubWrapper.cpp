
#include "NetworkDeviceHubWrapper.h"
#include "DeviceID.h"
#include "HubID.h"
#include "HubTrace.h"
#include "NetworkDeviceWrapper.h"
#include "orbTypes.h"

#include <memory>

#include <iostream>

using STI::Network::NetworkDeviceWrapper;
using STI::Network::NetworkDeviceHubWrapper;
using STI::Network::DeviceHub;
using STI::Network::HubID;
using STI::Network::HubTrace;
using STI::Network::LocalHub;

NetworkDeviceHubWrapper::NetworkDeviceHubWrapper(const std::shared_ptr<LocalHub<STI::Device::DeviceID, STI::Device::Device>>& hub)
	: localHub(hub), deviceHubServant(hub)
{
	//The LocalHub might have (local) nodes and hubs already attached that must be wrapped.
	//For all hubs currently stored by localHub, replace with NetworkDeviceHubWrapper (this is recursive)

	std::set<STI::Network::HubID> hubIDs;
	localHub->getHubIDs(hubIDs);
	std::shared_ptr<DeviceHub> attachedHub;
	std::shared_ptr<LocalHub<STI::Device::DeviceID, STI::Device::Device>> attachedLocalHub;
	std::shared_ptr<DeviceHub> wrappedHub;

	for (auto& hubID : hubIDs) {
		if (localHub->getHub(hubID, attachedHub) && attachedHub != 0) {

			//Check if the attached hub is local
			attachedLocalHub = std::dynamic_pointer_cast<LocalHub<STI::Device::DeviceID, STI::Device::Device>>(attachedHub);
			
			if (attachedLocalHub != 0) {
				//this is a LocalHub; wrap it.
				wrappedHub = std::make_shared<NetworkDeviceHubWrapper>(attachedLocalHub);	//recursive call
				addHub(hubID, wrappedHub);	//replaces with new wrapped version
			}

		}
	}

	//For all stored nodes, re-add (replace) with NetworkDeviceWrapper

	std::set<STI::Device::DeviceID> nodeIDs;
	localHub->getNodeIDs(nodeIDs);
	std::shared_ptr<STI::Device::Device> node;

	for (auto& id : nodeIDs) {
		if (localHub->getNode(id, node) && node != 0) {
			addNode(id, node);		//replaces with NetworkDeviceWrapper version
		}
	}
}

NetworkDeviceHubWrapper::~NetworkDeviceHubWrapper()
{
	std::cerr << "Destroying NetworkDeviceHubWrapper" << std::endl;
	//STI::TNetwork::TDeviceHub_i deviceHubServant;
//	deviceHubServant._remove_ref();
}

bool NetworkDeviceHubWrapper::addNode(const STI::Device::DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node)
{
	if (localHub != 0) {

		std::shared_ptr<NetworkDeviceWrapper> wrappedNode = std::make_shared<NetworkDeviceWrapper>(node);
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

const HubID& NetworkDeviceHubWrapper::getID()
{
	return localHub->getID();
}


bool NetworkDeviceHubWrapper::getTDeviceHubReference(const typename std::shared_ptr<DeviceHub>& deviceHub,
	STI::TNetwork::TDeviceHub_ptr& tDeviceHub)
{
	std::shared_ptr<NetworkDeviceHubWrapper> networkDeviceHubWrapper;
	networkDeviceHubWrapper = std::dynamic_pointer_cast<NetworkDeviceHubWrapper>(deviceHub);

	if (networkDeviceHubWrapper) {		//check dynamic_pointer_cast

		tDeviceHub = networkDeviceHubWrapper->deviceHubServant._this();
	}

	return !CORBA::is_nil(tDeviceHub);
}
