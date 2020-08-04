
#include "NetworkDeviceHubWrapper.h"
#include "DeviceID.h"
#include "HubID.h"
#include "HubTrace.h"

#include "orbTypes.h"

#include <memory>

using STI::Network::NetworkDeviceHubWrapper;
using STI::Network::DeviceHub;
using STI::Network::HubID;
using STI::Network::HubTrace;

NetworkDeviceHubWrapper::NetworkDeviceHubWrapper(const std::shared_ptr<DeviceHub>& hub)
	: localHub(hub), deviceHubServant(hub)
{
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
	return (tDeviceHub != 0 && !tDeviceHub->_is_nil());
}
