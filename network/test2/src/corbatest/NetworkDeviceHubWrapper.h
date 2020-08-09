#ifndef STI_NETWORK_NETWORKDEVICEHUBWRAPPER_H
#define STI_NETWORK_NETWORKDEVICEHUBWRAPPER_H

#include "Hub.h"
#include "TDeviceHub_i.h"
#include "DeviceHub.h"
#include "orbTypes.h"
#include "DeviceID.h"
#include "Device.h"

#include <memory>

namespace STI
{
namespace Network
{

//class LocalDeviceHub : public STI::Network::LocalHub<STI::Device::DeviceID, STI::Device::Device>

//thin wrapper around LocalHub that also holds the TDeviceHub_i servant of the same Hub
class NetworkDeviceHubWrapper : public STI::Network::Hub<STI::Device::DeviceID, STI::Device::Device>
{
public:

	NetworkDeviceHubWrapper(const std::shared_ptr<DeviceHub>& hub);
	~NetworkDeviceHubWrapper();

	//bool addHub(const HubID& id, const typename std::shared_ptr<Hub<ID, T>>& hub)

	bool addHub(const HubID& id, const typename std::shared_ptr<Hub<STI::Device::DeviceID, STI::Device::Device>>& hub);
	bool removeHub(const HubID& id);

	bool removeNode(const STI::Device::DeviceID& id, const HubTrace& trace);
	bool refresh(const HubTrace& trace);

	bool distribute(const STI::Device::DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node,
		const HubTrace& trace, const HubID& first);
	bool distributeNodes(const HubID& targetHub);
	bool redistributeNodes(const HubTrace& trace);

	const HubID& getID();

	static bool getTDeviceHubReference(const typename std::shared_ptr<DeviceHub>& deviceHub,
		STI::TNetwork::TDeviceHub_ptr& tDeviceHub);

private:

	std::shared_ptr<DeviceHub> localHub;


public:
	STI::TNetwork::TDeviceHub_i deviceHubServant;
};

} //Network
} //STI


#endif

