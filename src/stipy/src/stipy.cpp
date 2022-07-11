
#include "stipy.h"
#include "STIPyServer.h"
#include <sti/NetworkDeviceHub.h>
#include "STIPyLibDevice.h"
#include "STIPyShot.h"
#include "STIPyGlobal.h"
//#include "ORBManager.h"
#include "StackTracePy.h"
#include "LocalShot.h"
#include "NetworkShotWrapper.h"
#include "STIPyShot.h"

#include <sti/engine/RawEventTarget.h>

#include <iostream>


using STI::Python::STIPyServer;
using STI::Python::STIPyLibDevice;
using STI::Python::STIPyGlobal;
using STI::Engine::StackTrace;
using STI::Engine::RawEventGroup;
using STI::Engine::RawEventTarget;
using STI::Python::StackTracePy;
using STI::Python::STIPyShot;

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


std::shared_ptr<STIPyShot> STI::Python::makeShot()
{
    return makeShot("");
}

std::shared_ptr<STIPyShot> STI::Python::makeShot(const std::string& name)
{
    STI::Engine::ShotConfig shotConfig;
    auto events = std::make_shared<STI::Engine::RawEventVector>();

    auto shot = std::make_shared<STI::Engine::LocalShot>(shotConfig);
	shot->setEvents(events);

    auto networkShot = std::make_shared<STI::Network::NetworkShotWrapper>(shot);

    auto pyShot = std::make_shared<STIPyShot>(shot, name);
    return pyShot;
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



void STI::Python::event(const RawEventTarget& target, double time, const pybind11::object& value, 
                        const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group)
{
    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->event(target, time, value, stackTrace, group);
    }
}

void STI::Python::meas(const RawEventTarget& target, double time, const pybind11::object& value,
                        const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group)
{
    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->meas(target, time, value, stackTrace, group);
    }
}

void STI::Python::meas(const RawEventTarget& target, double time, const StackTracePy& stackTrace, 
                        const STI::Engine::RawEventGroup& group)
{
    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->meas(target, time, stackTrace, group);
    }
}

STI::Engine::RawEventTargetDevice STI::Python::dev(const std::string& channelName)
{
    STI::Engine::RawEventTargetDevice device(channelName);
    return device;
}

STI::Engine::RawEventTargetDevice STI::Python::dev(const std::string& name, const std::string& address, unsigned module)
{
    // auto stipy = STIPyGlobal::getInstance();

    STI::Engine::RawEventTargetDevice device(name, address, module);
    return device;

    // if (stipy != 0) {
    //     STI::Engine::RawEventTargetDevice device;
    //     device = stipy->dev(name, address, module);
    // }
    // else {
    //     STI::Device::DeviceID id(name, address, module);
    //     device = std::make_shared<STI::Engine::RawEventTargetDevice>(id);
    // }
    // return device;
}

// STI::Engine::RawEventTargetDevice STI::Python::dev(const std::string& name, const std::string& address, unsigned module, const std::string& targetServerID)
// {
//     auto device = std::make_shared<STI::Engine::RawEventTargetDevice>(name, address, module, targetServerID);
//     return device;
// }


STI::Engine::RawEventTarget STI::Python::ch(const STI::Engine::RawEventTargetDevice& device, unsigned channel)
{
    // auto pychannel = std::make_shared<STI::Engine::RawEventTarget>(device, channel);
    // STI::Engine::RawEventTargetChannel pyChannel(channel);
    STI::Engine::RawEventTarget target(device, channel);

    return target;
}


STI::Engine::RawEventTarget STI::Python::ch(const STI::Engine::RawEventTargetDevice& device, const std::string& channelName)
{
    // STI::Engine::RawEventTargetChannel pyChannel(channelName);
    STI::Engine::RawEventTarget target(device, channelName);

    return target;
}

STI::Engine::RawEventTarget STI::Python::ch(const std::string& channelName)
{
    // STI::Engine::RawEventTargetChannel pyChannel(channel);
    STI::Engine::RawEventTarget target(channelName);

    return target;
}
