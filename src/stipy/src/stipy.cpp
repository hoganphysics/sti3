
#include "stipy.h"
#include "STIPyServer.h"
#include <sti/NetworkDeviceHub.h>
#include "STIPyLibDevice.h"
#include "STIPyShot.h"
#include "STIPyGlobal.h"
#include "ORBManager.h"
#include "RawStackTrace.h"
#include "LocalShot.h"
#include "NetworkShotWrapper.h"
#include "STIPyShot.h"
#include "StackTraceData.h"
#include "NetworkFileHolder.h"
#include <sti/utils/LocalFileHolder.h>
#include <sti/engine/RawEventTarget.h>

#include <sti/utils/LocalFileHolder.h>

#include <pybind11/pybind11.h>

#include <iostream>

using STI::Network::ORBManager;

using STI::Python::STIPyServer;
using STI::Python::STIPyLibDevice;
using STI::Python::STIPyGlobal;
using STI::Engine::StackTrace;
using STI::Engine::RawEventGroup;
using STI::Engine::RawEventTarget;
using STI::Python::StackTracePy;
using STI::Python::STIPyShot;

using STI::Engine::RawStackTrace;

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

    std::shared_ptr<STI::Utils::FileHolderFactory> fileFactory;

    if (ORBManager::orbInstanceInitializd()) {
        fileFactory = std::make_shared<STI::Network::NetworkFileHolderFactory>();
    }
    else {
        fileFactory = std::make_shared<STI::Utils::LocalFileHolderFactory>();
    }

    auto stackTrace = std::make_shared<STI::Engine::StackTraceData>(fileFactory);
    auto eventGroup = std::make_shared<STI::Engine::RawEventGroup>("", "", stackTrace);

    std::shared_ptr<STI::Engine::Shot> shot;

    auto localShot = std::make_shared<STI::Engine::LocalShot>(shotConfig, eventGroup);

    if (ORBManager::orbInstanceInitializd()) {
        shot = std::make_shared<STI::Network::NetworkShotWrapper>(localShot);
    }
    else {
        shot = localShot;
    }

    auto pyShot = std::make_shared<STIPyShot>(shot);
    return pyShot;
}

std::shared_ptr<STIPyShot> STI::Python::makeShot(const std::string& name, const std::function<void(void)>& func)
{
    auto shot = makeShot(name);

    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->makeShot(shot, func);
    }

    return shot;
}

std::shared_ptr<STI::Engine::RawEventGroup> STI::Python::group(const std::string& name)
{
    auto stipy = STIPyGlobal::getInstance();
    std::shared_ptr<STI::Engine::RawEventGroup> g;

    if (stipy != 0) {
        g = stipy->group(name);
    }
    else {
        g = std::make_shared<STI::Engine::RawEventGroup>();
    }
    return g;
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


STI::Engine::ParsedVar STI::Python::var(const std::string& fullVarName, const STI::Engine::RawStackTrace& stackTrace)
{
    auto stipy = STIPyGlobal::getInstance();

    STI::Engine::ParsedVar v;

    if (stipy != 0) {
        v = stipy->var(fullVarName, stackTrace);
    }
    return v;
}

void STI::Python::setvar(const std::string& name, const pybind11::object& value, const STI::Engine::RawStackTrace& stackTrace, const std::string& scope)
{
    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->setvar(name, value, stackTrace, scope);
    }
}

void STI::Python::settag(const std::string& name, const STI::Engine::RawStackTrace& stackTrace, const std::string& scope)
{
    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->settag(name, stackTrace, scope);
    }
}


void STI::Python::event(const RawEventTarget& target, double time, const pybind11::object& value, 
                        const RawStackTrace& stackTrace, const std::string& scope)
{
    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->event(target, time, value, stackTrace, scope);
    }
}

void STI::Python::meas(const RawEventTarget& target, double time, const pybind11::object& value,
                        const RawStackTrace& stackTrace, const std::string& scope)
{
    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->meas(target, time, value, stackTrace, scope);
    }
}

void STI::Python::meas(const RawEventTarget& target, double time, const RawStackTrace& stackTrace, 
                        const std::string& scope)
{
    auto stipy = STIPyGlobal::getInstance();

    if (stipy != 0) {
        stipy->meas(target, time, stackTrace, scope);
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
