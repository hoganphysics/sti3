
#ifndef STI_PYTHON_STIPY_H
#define STI_PYTHON_STIPY_H

#include <sti/engine/StackTrace.h>
#include <sti/engine/RawEventTarget.h>
#include "RawEventGroup.h"
#include <sti/device/DeviceID.h>
#include <sti/network/HubID.h>
#include "StackTracePy.h"

#include <memory>
#include <string>

#include <pybind11/pybind11.h>




// Could rename this file globals.h


// int add(int i, int j);


namespace STI
{
namespace Python
{



// class ParseTicket;
// class ResultTicket;
// class STIPySeq;


class STIPyServer;
class STIPyShot;

// std::shared_ptr<STIPyServer> connect2(const std::string& localIP, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress);
// void connect3(const std::string& localIP, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress);
// void connect4(unsigned localIP, unsigned nameServerAddress);

std::shared_ptr<STIPyServer> connect(const std::string& localIP, const STI::Device::DeviceID& serverID, const std::string& nameServerAddress);
std::shared_ptr<STIPyServer> connect(const std::string& localIP, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress);

void disconnect();

std::string printNetwork(const std::string& nameServerAddress, const std::string& baseContext);



// //redirect to currently selected shot
// void setvar(const std::string& name, const pybind11::object& value);

std::shared_ptr<STIPyShot> makeShot();
std::shared_ptr<STIPyShot> makeShot(const std::string& name);

void event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group);
void meas(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group);
void meas(const STI::Engine::RawEventTarget& target, double time, const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group);

// STI::Engine::RawEventTargetDevice 
// dev(const std::string& name);
STI::Engine::RawEventTargetDevice dev(const std::string& channelName);  //abstract device
STI::Engine::RawEventTargetDevice dev(const std::string& name, const std::string& address, unsigned module);
// STI::Engine::RawEventTargetDevice dev(const std::string& name, const std::string& address, unsigned module, const std::string& targetServerID);

// std::shared_ptr<RawEventTarget> 
// ch(const std::string& name);

STI::Engine::RawEventTarget ch(const STI::Engine::RawEventTargetDevice& device, unsigned channel);
STI::Engine::RawEventTarget ch(const STI::Engine::RawEventTargetDevice& device, const std::string& channelName);    //abstract channel
STI::Engine::RawEventTarget ch(const std::string& channelName);    //abstract channel


} //Python
} //STI

#endif

