#include "stipy.h"

#include <sti/NetworkDeviceHub.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/StackTraceData.h>
#include <sti/utils/LocalFileHolder.h>

#include "LocalShot.h"
#include "RawStackTrace.h"
#include "STIPyGlobal.h"
#include "STIPyLibDevice.h"
#include "STIPyServer.h"
#include "STIPyShot.h"

#include <iostream>

#include <pybind11/pybind11.h>

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




std::shared_ptr<STIPyShot> STI::Python::makeShot()
{
    return makeShot("");
}

std::shared_ptr<STIPyShot> STI::Python::makeShot(const std::string& name)
{
    STI::Engine::ShotConfig shotConfig;

    std::shared_ptr<STI::Utils::FileHolderFactory> fileFactory;

    auto stackTrace = std::make_shared<STI::Engine::StackTraceData>();  //temp, needs to point to a local fileserver
    auto eventGroup = std::make_shared<STI::Engine::RawEventGroup>("", "", stackTrace);

    std::shared_ptr<STI::Engine::Shot> shot;

    auto localShot = std::make_shared<STI::Engine::LocalShot>(shotConfig, eventGroup);

    shot = localShot;

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

std::shared_ptr<STIPyServer> STI::Python::connect(const std::string& localAddress, const STI::Device::DeviceID& serverID, const std::string& nameServerAddress)
{
    //Default is to assume the server is connected to a Hub with a HubID matching the server's DeviceID
    STI::Network::HubID serverHubID(serverID.getName(), serverID.getAddress(), serverID.getModule());

    return connect(localAddress, serverID, serverHubID, nameServerAddress);
}

//Treat serverHubID as a guess. Check if it is live and hosts serverID
std::shared_ptr<STIPyServer> STI::Python::connect(const std::string& localAddress, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress)
{
    auto hub = std::make_shared<STI::Network::NetworkDeviceHub>(nameServerAddress);
    hub->getPersistenceOptions().bindToRootContext = false;
    hub->getPersistenceOptions().bindToTargetContexts = false;

    STI::Network::HubID verifiedServerHubID = serverHubID;  //guess

    if (!hub->findHub(serverID, verifiedServerHubID)) {
        // not found
        std::shared_ptr<STIPyServer> missing;
        std::cout << "Connect: Failed to find Hub for deviceID '" << serverID.getID() << "'" << std::endl;
        return missing;
    }

    //Need to ensure that the DeviceID is unique.  Could have:
    //multiple computers connecting, multiple connections from each computer
    //Use local IP address.  Generate Name that is unique using time?
    //Could also generate a module this way, or could query the server to get list of connections
    
    std::stringstream uniqueName;
    STI::Utils::TimeStamp connectTime;
       
    uniqueName << "STIPy";   //STIPy:localAddress:<data>:<time>
    uniqueName << ":" << localAddress;
    uniqueName << ":" << connectTime.date_YYYY_MM_DD("-") << ":" << connectTime.time_hh_mm_ss_mmmuuunnn();

    //Use a non-unique form of DeviceID to set STIPyLibDevice path structure (to avoid unwanted persistence directories).
    STI::Device::DeviceID subdirID("STIPy", localAddress, 0);
    STI::Utils::Configuration config;
    config.set<std::string>("PersistenceManager", "device subdirectory", subdirID.getID());

    auto stipydev = std::make_shared<STIPyLibDevice>(uniqueName.str(), localAddress, 0, serverID, verifiedServerHubID, config);

    hub->addDevice(stipydev, verifiedServerHubID);
    hub->run(false);    //don't block

    //stipydev->waitForConnection();

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

STI::Engine::RawEventTargetDevice STI::Python::dev(const std::string& deviceName)
{
    STI::Engine::RawEventTargetDevice device(deviceName);
    return device;
}

STI::Engine::RawEventTargetDevice STI::Python::dev(const std::string& name, const std::string& address, unsigned module)
{
    STI::Engine::RawEventTargetDevice device(name, address, module);
    return device;
}

STI::Engine::RawEventTargetDevice STI::Python::dev(const STI::Device::DeviceID& deviceID)
{
    STI::Engine::RawEventTargetDevice device(deviceID);
    return device;
}

// STI::Engine::RawEventTargetDevice STI::Python::dev(const std::string& name, const std::string& address, unsigned module, const std::string& targetServerID)
// {
//     auto device = std::make_shared<STI::Engine::RawEventTargetDevice>(name, address, module, targetServerID);
//     return device;
// }

STI::Engine::RawEventTarget STI::Python::ch(const STI::Engine::RawEventTargetDevice& device, unsigned channel)
{
    STI::Engine::RawEventTarget target(device, channel);
    return target;
}

STI::Engine::RawEventTarget STI::Python::ch(const STI::Engine::RawEventTargetDevice& device, const std::string& channelName)
{
    STI::Engine::RawEventTarget target(device, channelName);
    return target;
}

STI::Engine::RawEventTarget STI::Python::ch(const STI::Device::DeviceID& deviceID, unsigned channel)
{
    return ch(STI::Engine::RawEventTargetDevice(deviceID), channel);
}

STI::Engine::RawEventTarget STI::Python::ch(const STI::Device::DeviceID& deviceID, const std::string& channelName)
{
    return ch(STI::Engine::RawEventTargetDevice(deviceID), channelName);
}

STI::Engine::RawEventTarget STI::Python::ch(const std::string& channelName)
{
    STI::Engine::RawEventTarget target(channelName);
    return target;
}
