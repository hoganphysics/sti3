
#include "JNetworkDeviceHub.h"
#include <sti/NetworkDeviceHub.h>
#include "JNodeWalker.h"
#include <sti/LocalDeviceHub.h>

using STI::Network::JNetworkDeviceHub;
using STI::Network::JNodeWalker;


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

bool JNetworkDeviceHub::addNode(const typename std::shared_ptr<STI::Device::JDevice>& node)
{
    if(networkHub != 0) {
        return networkHub->addDevice(node);
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

JNodeWalker JNetworkDeviceHub::walk() const
{
    STI::Network::LocalDeviceHub::HubNodeWalker walker;

    if(networkHub != 0) {
        networkHub->walk(walker);
    }

//    JNodeWalker jWalker(walker);
 //   return jWalker;
    return JNodeWalker(walker);
}
