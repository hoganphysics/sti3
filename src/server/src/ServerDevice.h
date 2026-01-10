#ifndef STI_NETWORK_SERVERDEVICE_H
#define STI_NETWORK_SERVERDEVICE_H

#include <sti/LocalDevice.h>


namespace STI
{
namespace Device
{

class ServerDevice : public STI::Device::LocalDevice
{
public:
	ServerDevice(const STI::Utils::Configuration& config);
    // ServerDevice(const std::string& name, const std::string& address, unsigned short module,
	// 	const std::string& targetServer);
	~ServerDevice();

	bool writeChannel(short channel, const STI::Utils::MixedValue& value);

};


} //Device
} //STI


#endif

