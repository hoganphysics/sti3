
#include "JNetworkDeviceHub.h"
#include "NetworkDeviceHub.h"

using STI::Network::JNetworkDeviceHub;


JNetworkDeviceHub::JNetworkDeviceHub(const std::string& nameServiceAddress)
{
    networkHub = std::make_shared<NetworkDeviceHub>(nameServiceAddress);
}

JNetworkDeviceHub::JNetworkDeviceHub(const std::string& name, const std::string& address, unsigned short module, const std::string& nameServiceAddress)
{
    networkHub = std::make_shared<NetworkDeviceHub>(name, address, module, nameServiceAddress);
}

JNetworkDeviceHub::~JNetworkDeviceHub()
{
}

bool JNetworkDeviceHub::addNode(const STI::Device::DeviceID& id, const typename std::shared_ptr<STI::Device::JDevice>& node)
{
    if(networkHub != 0) {
        return networkHub->addNode(id, node);
    }
    return false;
}

void JNetworkDeviceHub::run()
{
    if(networkHub != 0) {
        networkHub->run();
    }
}

void JNetworkDeviceHub::run(bool block)
{
    if(networkHub != 0) {
        networkHub->run(block);
    }
}
