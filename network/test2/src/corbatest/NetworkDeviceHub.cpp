
#include "NetworkDeviceHub.h"

#include "NetworkDeviceHubWrapper.h"
#include "LocalDeviceHub.h"


using STI::Network::NetworkDeviceHub;
using STI::Device::DeviceID;
using STI::Network::LocalDeviceHub;

NetworkDeviceHub::NetworkDeviceHub(const std::string& name)
{
	localHub = std::make_shared<LocalDeviceHub>(name);
	deviceHubWrapper = std::make_shared<NetworkDeviceHubWrapper>(localHub);
}

bool NetworkDeviceHub::addNode(const DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node)
{
	if (localHub != 0) {
		return localHub->addNode(id, node);
	}
	return false;
}

bool NetworkDeviceHub::connect(const std::shared_ptr<LocalDeviceHub>& hub)
{
	return LocalDeviceHub::connect(localHub, hub);
}


