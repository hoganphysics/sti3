#ifndef STI_PYTHON_STIPY_H
#define STI_PYTHON_STIPY_H

#include <sti/engine/CompressedStackTrace.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/device/DeviceID.h>
#include <sti/network/HubID.h>
#include "StackTrace.h"

#include <memory>
#include <string>

#include <pybind11/pybind11.h>


// Could rename this file globals.h


namespace STI
{
namespace Python
{


class STIPyServer;
class STIPyShot;


std::shared_ptr<STIPyServer> connect(const std::string& localAddress, const STI::Device::DeviceID& serverID, const std::string& nameServerAddress);
std::shared_ptr<STIPyServer> connect(const std::string& localAddress, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress);

void disconnect();

std::string printNetwork(const std::string& nameServerAddress, const std::string& baseContext);


std::shared_ptr<STIPyShot> makeShot();
std::shared_ptr<STIPyShot> makeShot(const std::string& name);
std::shared_ptr<STIPyShot> makeShot(const std::string& name, const std::function<void(void)>& func);

STI::Engine::ParsedVar var(const std::string& fullVarName, const STI::Engine::StackTrace& stackTrace);

std::shared_ptr<STI::Engine::RawEventGroup> group(const std::string& name);

void setvar(const std::string& name, const pybind11::object& value, const STI::Engine::StackTrace& stackTrace, const std::string& scope);
void settag(const std::string& name, const STI::Engine::StackTrace& stackTrace, const std::string& scope);

void event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, const STI::Engine::StackTrace& stackTrace, const std::string& scope);
void meas(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, const STI::Engine::StackTrace& stackTrace, const std::string& scope);
void meas(const STI::Engine::RawEventTarget& target, double time, const STI::Engine::StackTrace& stackTrace, const std::string& scope);

void set_trigger(const STI::Device::DeviceID& deviceID, const STI::Engine::StackTrace& stackTrace);
void set_trigger(const STI::Engine::RawEventTargetDevice& device, const STI::Engine::StackTrace& stackTrace);

STI::Engine::RawEventTargetDevice dev(const std::string& deviceName);  //abstract device
STI::Engine::RawEventTargetDevice dev(const std::string& name, const std::string& address, unsigned module);
STI::Engine::RawEventTargetDevice dev(const STI::Device::DeviceID& deviceID);

STI::Engine::RawEventTarget ch(const STI::Engine::RawEventTargetDevice& device, unsigned channel);
STI::Engine::RawEventTarget ch(const STI::Engine::RawEventTargetDevice& device, const std::string& channelName);    //abstract channel
STI::Engine::RawEventTarget ch(const STI::Device::DeviceID& deviceID, unsigned channel);
STI::Engine::RawEventTarget ch(const STI::Device::DeviceID& deviceID, const std::string& channelName);    //abstract channel
STI::Engine::RawEventTarget ch(const std::string& channelName);    //abstract channel


} //Python
} //STI

#endif

