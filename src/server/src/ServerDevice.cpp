
#include "ServerDevice.h"

using STI::Device::ServerDevice;


ServerDevice::ServerDevice(const std::string& name, const std::string& address, unsigned short module,
    const std::string& targetServer)
: STI::Device::LocalDevice(name, address, module, targetServer)
{
}

ServerDevice::~ServerDevice()
{
}

