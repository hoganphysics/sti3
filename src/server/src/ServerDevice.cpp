
#include "ServerDevice.h"

using STI::Device::ServerDevice;


ServerDevice::ServerDevice(const std::string& name, const std::string& address, unsigned short module,
    const std::string& targetServer)
: STI::Device::LocalDevice(name, address, module, targetServer)
{
	STI::Engine::EngineID id(0);
	addEventEngine(id);

	addChannel(1, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Double, "testch");

}

ServerDevice::~ServerDevice()
{
}

