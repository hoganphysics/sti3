#ifndef STI_NETWORK_NETWORKDEVICEHUBWRAPPER_H
#define STI_NETWORK_NETWORKDEVICEHUBWRAPPER_H

#include <sti/LocalDeviceHub.h>
#include <sti/network/DeviceHub.h>
#include <sti/device/DeviceID.h>
#include <sti/device/Device.h>

#include "TDeviceHub_i.h"
#include "generated/orbTypes.h"
#include "ServantHolder.h"

#include <memory>

namespace STI
{
namespace Network
{


//thin wrapper around LocalHub that also holds the TDeviceHub_i servant of the same Hub
class NetworkDeviceHubWrapper : public STI::Network::Hub<STI::Device::DeviceID, STI::Device::Device>
{
public:

	NetworkDeviceHubWrapper(const std::shared_ptr<LocalDeviceHub>& hub);
	~NetworkDeviceHubWrapper();

	bool addNode(const STI::Device::DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node);
	bool removeNode(const STI::Device::DeviceID& id);
	bool removeNode(const STI::Device::DeviceID& id, const HubTrace& trace);

	bool addHub(const HubID& id, const typename std::shared_ptr<Hub<STI::Device::DeviceID, STI::Device::Device>>& hub);
	bool removeHub(const HubID& id);

	bool refresh(const HubTrace& trace);

	bool distribute(const STI::Device::DeviceID& id, const typename std::shared_ptr<STI::Device::Device>& node,
		const HubTrace& trace, const HubID& first);
	bool distributeNodes(const HubID& targetHub);
	bool redistributeNodes(const HubTrace& trace);

	const HubID& getID() const;
	bool hasNodeID(const STI::Device::DeviceID& id) const;

	bool ping() const;
	bool isConnectedTo(const HubID& id) const;

	void walk(NodeWalker<STI::Device::DeviceID, STI::Device::Device>& root, const HubTrace& trace) const;

	static bool getTDeviceHubReference(const typename std::shared_ptr<DeviceHub>& deviceHub,
		STI::TNetwork::TDeviceHub_var& tDeviceHub);

private:

	std::shared_ptr<LocalDeviceHub> localHub;
	
	// STI::TNetwork::TDeviceHub_i deviceHubServant;
	ServantHolder<STI::TNetwork::TDeviceHub_i, STI::TNetwork::TDeviceHub> deviceHubServantHolder;
};

} //Network
} //STI


#endif

