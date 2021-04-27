
#include "STIPyLibDevice.h"
#include "DeviceID.h"
#include "DeviceCollection.h"

#include <memory>
#include <iostream>

using STI::Python::STIPyLibDevice;
using STI::Device::LocalDevice;


STIPyLibDevice::STIPyLibDevice(const std::string& name, const std::string& address, unsigned short module,
		const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID)
: LocalDevice(name, address, module, serverID.getID()), serverID(serverID), serverHubID(serverHubID)
{
    addPartner(serverID);
}

bool STIPyLibDevice::addto(const STI::Network::HubID& target)
{
    return serverHubID == target;
}

bool STIPyLibDevice::getServer(std::shared_ptr<Device>& server)
{
    std::shared_ptr<STI::Device::DeviceCollection> deviceCollection;
    getCollection(deviceCollection);
    
    std::cout << "Collection: " << deviceCollection->size() << std::endl;

    return (deviceCollection != 0) && deviceCollection->get(serverID, server);
}
