
#ifndef STI_PYTHON_STIPY_H
#define STI_PYTHON_STIPY_H


#include "DeviceID.h"
#include "HubID.h"

#include <memory>
#include <string>

#include <pybind11/pybind11.h>




// Could rename this file globals.h


// int add(int i, int j);


namespace STI
{
namespace Python
{

class STIPyChannel;



// class ParseTicket;
// class ResultTicket;
// class STIPySeq;


class STIPyDevice;
class STIPyChannel;
class STIPyServer;
// class STIPyShot;

// std::shared_ptr<STIPyServer> connect2(const std::string& localIP, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress);
// void connect3(const std::string& localIP, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress);
// void connect4(unsigned localIP, unsigned nameServerAddress);

std::shared_ptr<STIPyServer> connect(const std::string& localIP, const STI::Device::DeviceID& serverID, const std::string& nameServerAddress);
std::shared_ptr<STIPyServer> connect(const std::string& localIP, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress);

void disconnect();

std::string printNetwork(const std::string& nameServerAddress, const std::string& baseContext);



// //redirect to currently selected shot
// void setvar(const std::string& name, const pybind11::object& value);

void event(const STIPyChannel& channel, double time, const pybind11::object& value);
void meas(const STIPyChannel& channel, double time, const pybind11::object& value);
void meas(const STIPyChannel& channel, double time);

std::shared_ptr<STIPyDevice> 
dev(const std::string& name, const std::string& address, unsigned module);

std::shared_ptr<STIPyDevice> 
dev(const std::string& name, const std::string& address, unsigned module, const std::string& targetServerID);

std::shared_ptr<STIPyChannel> 
ch(const std::shared_ptr<STI::Python::STIPyDevice>& device, unsigned channel);


} //Python
} //STI

#endif

