#ifndef STI_NETWORK_LOCALDEVICEHUB_H
#define STI_NETWORK_LOCALDEVICEHUB_H

#include "Device.h"
#include "LocalHub.h"
#include "HubID.h"

#include <string>


namespace STI
{
namespace Network
{


class LocalDeviceHub : public STI::Network::LocalHub<STI::Device::DeviceID, STI::Device::Device>
{
public:
	LocalDeviceHub(const std::string& name, const std::string& address = "localhost", unsigned short module = 0)
	{
		STI::Network::HubID id;

		id.name = name;
		id.address = address;
		id.module = module;

		setID(id);
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


private:

	
};

} //Network
} //STI


#endif

