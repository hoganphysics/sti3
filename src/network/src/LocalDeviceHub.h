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
		id.name = name;
		id.address = address;
		id.module = module;
	}

	const STI::Network::HubID& getID() { return id; }

	void setID(const STI::Network::HubID hubID) { id = hubID; }

private:

	STI::Network::HubID id;
};

} //Network
} //STI


#endif

