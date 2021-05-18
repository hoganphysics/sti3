
#ifndef STI_PYTHON_STIPYSHOT_H
#define STI_PYTHON_STIPYSHOT_H

#include "fwd/RawEvent_fwd.h"

#include "Shot.h"
#include "DeviceID.h"
#include <pybind11/pybind11.h>

#include <vector>
#include <memory>
#include <mutex>
#include <map>

namespace STI
{
namespace Python
{

class STIPyChannel;
class STIPyServer;
class ParseTicket;


class STIPyShot
{
public:

    STIPyShot(const std::shared_ptr<STI::Engine::Shot>& shot, const STI::Device::DeviceID& serverID);

    void setvar(const std::string& name, const pybind11::object& value);
    void event(const STIPyChannel& channel, double time, const pybind11::object& value);
    void meas(const STIPyChannel& channel, double time, const pybind11::object& value);

//    void getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts);

    std::vector<STI::Engine::RawEvent> getEvents();


    //Not sure we need these; can be done using server->parse()
    // ParseTicket parse();
    // ParseTicket parse(const pybind11::dict& channels);
    void append(pybind11::object func);     //treat current list of setvars as overwritten vars
    void append(const STI::Engine::RawEvent& evt);

    const STI::Device::DeviceID& getServerID() { return serverID; }

    std::shared_ptr<STI::Engine::Shot> getShot() { return shot; }

private:

    void addEvent(const STIPyChannel& channel, double time, const pybind11::object& value, const STI::Engine::RawEventType& type);

    std::shared_ptr<STI::Engine::Shot> shot;
    
    mutable std::mutex eventMutex;
    unsigned eventNumber;
    std::shared_ptr<std::vector<STI::Engine::RawEvent>> events;

    std::shared_ptr<std::map<std::string, pybind11::object>> vars;

    STI::Device::DeviceID serverID;

};


} //Python
} //STI

#endif

