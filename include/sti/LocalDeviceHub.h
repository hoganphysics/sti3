#ifndef STI_NETWORK_LOCALDEVICEHUB_H
#define STI_NETWORK_LOCALDEVICEHUB_H

#include <sti/device/Device.h>
#include <sti/network/HubID.h>
#include <sti/network/LocalHub.h>

#include <string>


namespace STI
{
namespace Network
{


class LocalDeviceHub : public STI::Network::LocalHub<STI::Device::DeviceID, STI::Device::Device>
{
public:
	
	LocalDeviceHub(const STI::Network::HubID& hubID)
	: STI::Network::LocalHub<STI::Device::DeviceID, STI::Device::Device>(hubID)
	{
	}

	bool addDevice(const typename std::shared_ptr<STI::Device::Device>& node)
	{
		if (node != 0) {
			return addNode(node->getID(), node);
		}
		return false;
	}
	bool removeDevice(const STI::Device::DeviceID& id)
	{
		return removeNode(id);
	}

};

} //Network
} //STI


#endif

