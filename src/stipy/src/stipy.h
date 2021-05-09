
#ifndef STI_PYTHON_STIPY_H
#define STI_PYTHON_STIPY_H




#include "DeviceID.h"
#include "HubID.h"


#include <memory>
#include <string>
#include <vector>
#include <pybind11/pybind11.h>


int add(int i, int j);


namespace STI
{
namespace Python
{


// class ParseTicket;
// class ResultTicket;
// class STIPySeq;


// class STIPyDevice;
// class STIPyChannel;
class STIPyServer;
// class STIPyShot;

std::shared_ptr<STIPyServer> connect2(const std::string& localIP, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress);
void connect3(const std::string& localIP, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress);
void connect4(unsigned localIP, unsigned nameServerAddress);

std::shared_ptr<STIPyServer> connect(const std::string& localIP, const STI::Device::DeviceID& serverID, const std::string& nameServerAddress);
std::shared_ptr<STIPyServer> connect(const std::string& localIP, const STI::Device::DeviceID& serverID, const STI::Network::HubID& serverHubID, const std::string& nameServerAddress);

void disconnect();

//in case HubID is not default!

std::string printNetwork(const std::string& nameServerAddress, const std::string& baseContext);

//std::shared_ptr<STIPyShot> makeshot();


//global locks
void setShot();
void releaseShot();

// //redirect to currently selected shot
// void setvar(const std::string& name, const pybind11::object& value);
// void event(const STIPyChannel& channel, double time, const pybind11::object& value);
// void meas(const STIPyChannel& channel, double time, const pybind11::object& value);


// STIPyDevice dev(const std::string& name, const std::string& address, unsigned module);
// STIPyChannel ch(const STIPyDevice& device, unsigned channel);

} //Python
} //STI

#endif

