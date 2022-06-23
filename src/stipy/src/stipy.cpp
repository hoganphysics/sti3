
#include "stipy.h"
#include "STIPyServer.h"
#include <sti/NetworkDeviceHub.h>
#include "STIPyLibDevice.h"
#include "STIPyShot.h"
#include "STIPyGlobal.h"
//#include "ORBManager.h"

#include "STIPyChannel.h"

#include <iostream>


using STI::Python::STIPyServer;
using STI::Python::STIPyLibDevice;
using STI::Python::STIPyGlobal;


// int add(int i, int j) {
//     return i + j;
// }



// auto globalLib = STIPyGlobal::getInstance();

// std::string globalTest = "hello global";


// std::shared_ptr<STIPyServer> STI::Python::connect2(const std::string& localIP, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress)
// {
//     //Default is to assume the server is connected to a Hub with a HubID matching the server's DeviceID
//     auto srv = std::make_shared<STIPyServer>();
//     return srv;

// }
// void STI::Python::connect3(const std::string& localIP, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress)
// {

// }

// void STI::Python::connect4(unsigned localIP, unsigned nameServerAddress)
// {

// }

std::shared_ptr<STIPyServer> STI::Python::connect(const std::string& localIP, const STI::Device::DeviceID& serverID, const std::string& nameServerAddress)
{
    //Default is to assume the server is connected to a Hub with a HubID matching the server's DeviceID
    STI::Network::HubID serverHubID(serverID.getName(), serverID.getAddress(), serverID.getModule());

    return connect(localIP, serverID, serverHubID, nameServerAddress);
}


std::shared_ptr<STIPyServer> STI::Python::connect(const std::string& localIP, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress)
{
    auto hub = std::make_shared<STI::Network::NetworkDeviceHub>(nameServerAddress);
//    hub->getPersistenceOptions().bindToRootContext = false;
//    hub->getPersistenceOptions().bindToTargetContexts = false;

    //Need to ensure that the DeviceID is unique.  Could have:
    //multiple computers connecting, multiple connections from each computer
    //Use local IP address.  Generate Name that is unique using time?
    //Could also generate a module this way, or could query the server to get list of connections
    
    // std::cout << "Test global: " << globalTest << " : " << globalLib->get() << std::endl;

    std::string uniqueName = "STIPy";   //Add timestamp?  STIPy::<timestamp>

    auto stipydev = std::make_shared<STIPyLibDevice>(uniqueName, localIP, 0, serverID, serverHubID);

    hub->addDevice(stipydev);
    hub->run(false);

    stipydev->waitForConnection();

    auto server = std::make_shared<STIPyServer>(hub, stipydev, serverID);
    return server;
}


void STI::Python::disconnect()
{

}

std::string STI::Python::printNetwork(const std::string& nameServerAddress, const std::string& baseContext)
{
    return STI::Network::NetworkDeviceHub::printNetwork(nameServerAddress, baseContext);
}


void STI::Python::event(const STIPyChannel& channel, double time, const pybind11::object& value)
{
    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->event(channel, time, value);
    }
}

void STI::Python::meas(const STIPyChannel& channel, double time, const pybind11::object& value)
{
    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->meas(channel, time, value);
    }
}

void STI::Python::meas(const STIPyChannel& channel, double time)
{
    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->meas(channel, time);
    }
}

std::shared_ptr<STI::Python::STIPyDevice> 
STI::Python::dev(const std::string& name, const std::string& address, unsigned module)
{
    auto stipy = STIPyGlobal::getInstance();

    std::shared_ptr<STI::Python::STIPyDevice> device;

    if (stipy != 0) {
        device = stipy->dev(name, address, module);
    }
    else {
        STI::Device::DeviceID id(name, address, module);
        device = std::make_shared<STI::Python::STIPyDevice>(id);
    }
    return device;
}

std::shared_ptr<STI::Python::STIPyDevice> 
STI::Python::dev(const std::string& name, const std::string& address, unsigned module, const std::string& targetServerID)
{
    auto device = std::make_shared<STI::Python::STIPyDevice>(name, address, module, targetServerID);
    return device;
}

std::shared_ptr<STI::Python::STIPyChannel> 
STI::Python::ch(const std::shared_ptr<STI::Python::STIPyDevice>& device, unsigned channel)
{
    auto pychannel = std::make_shared<STI::Python::STIPyChannel>(device, channel);
    return pychannel;
}

