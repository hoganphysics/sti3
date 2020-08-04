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
	LocalDeviceHub(const std::string& name)
	{
		id.name = name;
		id.address = "localhost";
	}

	const STI::Network::HubID& getID() { return id; }
	STI::Network::HubID id;
};

} //Network
} //STI


#endif

