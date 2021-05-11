
#include "STIPyShot.h"
#include "RawEvent.h"
#include "ParseTicket.h"
#include "MixedValuePy.h"
#include "STIPyChannel.h"

#include "ParseID.h"


using STI::Python::STIPyShot;
using STI::Python::ParseTicket;
using STI::Engine::RawEvent;
using STI::Engine::RawEventType;
using STI::Python::STIPyChannel;


STIPyShot::STIPyShot(const std::shared_ptr<STI::Engine::Shot>& shot, const STI::Device::DeviceID& serverID)
: shot(shot), serverID(serverID)
{
    eventNumber = 0;
//    events = std::make_shared<std::vector<STI::Engine::RawEvent>>();

    if (shot != 0) {
        shot->getEvents(events);
    }

    vars = std::make_shared<std::map<std::string, pybind11::object>>();
}

// std::vector<STI::Engine::RawEvent> STIPyShot::getEvents()
// {
//     return (*events);
// }

// void STIPyShot::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts)
// {
//     evts = events;
// }

void STIPyShot::setvar(const std::string& name, const pybind11::object& value)
{
    std::unique_lock<std::mutex> evtLock(eventMutex);

    if (vars == 0) return;

    auto it = vars->find(name);

    if (it == vars->end()) {    //not found
        (*vars)[name] = value;
    }


}

void STIPyShot::event(const STIPyChannel& channel, double time, const pybind11::object& value)
{
    addEvent(channel, time, value, RawEventType::Play);
}

void STIPyShot::meas(const STIPyChannel& channel, double time, const pybind11::object& value)
{
    addEvent(channel, time, value, RawEventType::Measurement);
}

void STIPyShot::addEvent(const STIPyChannel& channel, double time, const pybind11::object& value, const RawEventType& type)
{
    std::unique_lock<std::mutex> evtLock(eventMutex);

    std::string description = "";
//    eventNumber++;
    STI::Python::MixedValuePy valuepy;

    valuepy.setValue_py(value);

    if (events != 0) {
        events->push_back( RawEvent(channel.device()->id(), time, channel.channel(), 
                                    valuepy, description, events->size(), type) );        
    }

}

// ParseTicket STIPyShot::parse()
// {
//     STI::Engine::ParseID pid;
//     pid.parseTimestamp.timestamp = 1.1;
    
//  //   ParseTicket ticket(pid);
//     ParseTicket ticket = libDevice->makeParseTicket(pid);

//  //   ParseTicket ticket;
//     return ticket;
// }

// ParseTicket STIPyShot::parse(const pybind11::dict& channels)
// {

//     STI::Engine::ParseID pid;
//     pid.parseTimestamp.timestamp = 1.1;
    
//     ParseTicket ticket(pid);
// //    ParseTicket ticket;
//     return ticket;
// }


void STIPyShot::append(pybind11::object func)
{
}

void STIPyShot::append(const STI::Engine::RawEvent& evt)
{
}

std::vector<STI::Engine::RawEvent> STIPyShot::getEvents()
{
    if (events != 0) {
        return *events;
    }
    else {
        auto nullEvts = std::make_shared<std::vector<STI::Engine::RawEvent>>();
        return *nullEvts;
    }
}


// void STIPyShot::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts)
// {
//     evts = events;
// }

