
#ifndef STI_PYTHON_STIPYSERVER_H
#define STI_PYTHON_STIPYSERVER_H

#include "STIPyDevice.h"
#include "ParseID.h"

#include "NetworkDeviceHub.h"
#include "STIPyLibDevice.h"

#include <pybind11/pybind11.h>
#include <string>

namespace STI
{
namespace Python
{

class STIPyShot;
class STIPySeq;
class ParseTicket;
class ResultTicket;


//maybe this should be the same as STIPyLibDevice, and should have a getServer() function to return the server device reference
//No, they are separate concepts:
//  1. Lib device connects to the server hub to get access to server reference
//  2. Server interface generates shots and tickets for parse/play
//Maybe STIPyServer and the server refernece should be the same thing (STIPyServer presents device reference?)

class STIPyServer
{
public:

    STIPyServer();//tmp
    STIPyServer(const std::shared_ptr<STI::Network::NetworkDeviceHub>& libDeviceHub, const std::shared_ptr<STIPyLibDevice>& libDevice);

    void setChannels(const pybind11::dict& channels);

    std::shared_ptr<STIPyShot> makeshot();
    std::shared_ptr<STIPyShot> makeshot(pybind11::object func);
    std::shared_ptr<STIPyShot> makeshot(pybind11::object func, const pybind11::dict& vars);    //uses dictionary vars to override servars

    std::shared_ptr<STIPySeq> makesequence(pybind11::object func);

    std::shared_ptr<ParseTicket> parse(const std::shared_ptr<STIPyShot>& shot);
    std::shared_ptr<ParseTicket> parse(const std::shared_ptr<STIPyShot>& shot, const pybind11::dict& channels);
    std::shared_ptr<ParseTicket> parse(const std::vector<ParseTicket>& tickets);  //combining multiple servers

    ResultTicket play(const ParseTicket& ticket);
    ResultTicket play(const ParseTicket& ticket, unsigned repeats);
    ResultTicket play(const STI::Engine::ParseID& parseID, unsigned repeats);

private:

    std::shared_ptr<STI::Network::NetworkDeviceHub> libDeviceHub;
    std::shared_ptr<STIPyLibDevice> libDevice;

};


} //Python
} //STI

#endif

