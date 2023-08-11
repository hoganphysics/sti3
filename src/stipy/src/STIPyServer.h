
#ifndef STI_PYTHON_STIPYSERVER_H
#define STI_PYTHON_STIPYSERVER_H

#include <sti/engine/RawEventTargetDevice.h>
#include <sti/engine/ParseID.h>

#include <sti/NetworkDeviceHub.h>
#include "STIPyLibDevice.h"
#include "DevicePy.h"

#include <functional>
#include <string>
#include <set>

#include <pybind11/pybind11.h>


namespace STI
{
namespace Python
{

class STIPyShot;
class STIPySeq;
class PyParseTicket;
class PyResultTicket;


//maybe this should be the same as STIPyLibDevice, and should have a getServer() function to return the server device reference
//No, they are separate concepts:
//  1. Lib device connects to the server hub to get access to server reference
//  2. Server interface generates shots and tickets for parse/play
//Maybe STIPyServer and the server refernece should be the same thing (STIPyServer presents device reference?)

/*

class STIPyServer : public STIPyDevice

frame1=dev("frame 1")
frame2=dev("frame 2")

s1=frame1.makeshot(motFunc, vars1)
s2=frame2.makeshot(motFunc, vars2)

server = connect(...)

s = server.makeshot([s1, s2])
p = server.parse(s)
p = server.parse([s1, s2])


#makeshot always defines a top level group, by default "", or name
#when called on an existing device, the targetserverID of the top group is the device

s = makeshot(func)      #No targetServerID; could have abstract devices
devs1 = {frame1: dev("Frame1", "192.168.1.3",0), dds: dev("DDS", "192.168.1.6", 6)}
devs2 = {frame2: dev("Frame2", "192.168.1.7",0), dds: dev("DDS", "192.168.1.9", 6)}
s1=frame1.makeshot(s, devs1)    #speciallized shots for each frame
s2=frame2.makeshot(s, devs2)
p = server.parse([s1, s2])  #single, collective parse. Merge event groups by adding to top level and reindexing (ensure unique indices)

#alternative:
p1=frame1.parse(s, devs1)    #parse frame 1
p2=frame2.parse(s, devs2)    #parse frame 2
p = server.parse([p1, p2])  #joins parse results, if possible (parse trees must be independent). Pull RawEvents from subservers, add new top group and reindex groups to ensure unique.
server.play(p)

s = makeshot(func)
p = server.parse(s)     #sets targetServerID to its own ID?




Shots with names

s1 = makeshot("Frame 1", func, vars1, devs1)
s2 = makeshot("Frame 2", func, vars2, devs2)
p = server.parse([p1, p2])
server.play(p)


s = makeshot(func)  #default name ""

s1 = makeshot("Frame 1", s, devs1)
s2 = makeshot("Frame 1", s, devs1)


*/


class STIPyServer : public DevicePy
{
public:

    STIPyServer(const std::shared_ptr<STI::Network::NetworkDeviceHub>& libDeviceHub, 
                const std::shared_ptr<STIPyLibDevice>& libDevice, 
                const STI::Device::DeviceID& serverID);
    virtual ~STIPyServer();

    void setChannels(const pybind11::dict& channels);

    std::shared_ptr<STIPyShot> makeshot();
    std::shared_ptr<STIPyShot> makeshot(const std::function<void(void)>& func);
    std::shared_ptr<STIPyShot> makeshot(const std::function<void(void)>& func, const std::set<STI::Engine::ParsedVar>& vars);    //uses dictionary vars to override servars

    // std::shared_ptr<PySequenceTicket> addsequence(const std::shared_ptr<Sequence>& sequence);

    // addseq([{"x":5}, {"x":7}])
    
    
    // std::shared_ptr<STI::Engine::Sequence> addsequence(const pybind11::list& varsTable);

    std::shared_ptr<STI::Engine::Sequence> makesequence(const std::function<void(void)>& func);
    std::shared_ptr<STI::Engine::Sequence> makesequence(const pybind11::set& vars);
    std::shared_ptr<STI::Engine::Sequence> makesequence(const pybind11::dict& vars);
    // std::shared_ptr<STI::Engine::Sequence> makesequence(pybind11::object func);

    std::shared_ptr<PyParseTicket> parse(const std::shared_ptr<STIPyShot>& pyShot);
    std::shared_ptr<PyParseTicket> parse(const std::shared_ptr<STIPyShot>& pyShot, const STI::Engine::SequenceEntryID& sequenceEntryID);

    std::shared_ptr<PyParseTicket> parse(const std::shared_ptr<STIPyShot>& pyShot, const pybind11::dict& channels);
    std::shared_ptr<PyParseTicket> parse(const std::vector<PyParseTicket>& tickets);  //combining multiple servers

    std::shared_ptr<PyResultTicket> play(const std::shared_ptr<PyParseTicket>& ticket);
    std::shared_ptr<PyResultTicket> play(const std::shared_ptr<PyParseTicket>& ticket, unsigned repeats);
    std::shared_ptr<PyResultTicket> play(const STI::Engine::ParseID& parseID, unsigned repeats);

    STI::Engine::SequenceID parse(const std::shared_ptr<STI::Engine::Sequence>& seq);

    void cancelAll();

    std::string printNetwork();
    std::string printNetwork(const std::string& baseContext);

private:


    bool getScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler);
    bool getPersistenceManager(std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager);


    std::shared_ptr<STI::Network::NetworkDeviceHub> libDeviceHub;
    std::shared_ptr<STIPyLibDevice> libDevice;
    STI::Device::DeviceID serverID;

};


} //Python
} //STI

#endif

