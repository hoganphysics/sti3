
#include "stipy.h"
#include "STIPyServer.h"
#include "NetworkDeviceHub.h"
#include "STIPyLibDevice.h"

using STI::Python::STIPyServer;

using STI::Python::STIPyLibDevice;

int add(int i, int j) {
    return i + j;
}

std::shared_ptr<STIPyServer> STI::Python::connect2(const std::string& localIP, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress)
{
    //Default is to assume the server is connected to a Hub with a HubID matching the server's DeviceID
    auto srv = std::make_shared<STIPyServer>();
    return srv;

}
void STI::Python::connect3(const std::string& localIP, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress)
{

}

void STI::Python::connect4(unsigned localIP, unsigned nameServerAddress)
{

}

std::shared_ptr<STIPyServer> STI::Python::connect(const std::string& localIP, const STI::Device::DeviceID& serverID, const std::string& nameServerAddress)
{
    //Default is to assume the server is connected to a Hub with a HubID matching the server's DeviceID
    STI::Network::HubID serverHubID(serverID.getName(), serverID.getAddress(), serverID.getModule());

    return connect(localIP, serverID, serverHubID, nameServerAddress);
}


std::shared_ptr<STIPyServer> STI::Python::connect(const std::string& localIP, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress)
{
    auto hub = std::make_shared<STI::Network::NetworkDeviceHub>(nameServerAddress);

    //Need to ensure that the DeviceID is unique.  Could have:
    //multiple computers connecting, multiple connections from each computer
    //Use local IP address.  Generate Name that is unique using time?
    //Could also generate a module this way, or could query the server to get list of connections

    std::string uniqueName = "STIPy";   //Add timestamp?  STIPy::<timestamp>

    auto stipydev = std::make_shared<STIPyLibDevice>(uniqueName, localIP, 0, serverID, serverHubID);
    hub->addDevice(stipydev);
    hub->run(false);

    auto server = std::make_shared<STIPyServer>(hub, stipydev);
    return server;
}


void STI::Python::disconnect()
{

}

