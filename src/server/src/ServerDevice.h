#ifndef STI_NETWORK_SERVERDEVICE_H
#define STI_NETWORK_SERVERDEVICE_H

#include "LocalDevice.h"


namespace STI
{
namespace Device
{

class ServerDevice : public STI::Device::LocalDevice	//LocalServer ?
{
public:
    ServerDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
	~ServerDevice();

};


} //Device
} //STI


#endif

