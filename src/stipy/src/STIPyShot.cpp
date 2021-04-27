
#include "STIPyShot.h"
#include "RawEvent.h"
#include "ParseTicket.h"
#include "MixedValuePy.h"
#include "STIPyChannel.h"

#include "ParseID.h"


using STI::Python::STIPyShot;
using STI::Python::ParseTicket;
using STI::Engine::RawEventType;
using STI::Python::STIPyChannel;

STIPyShot::STIPyShot(STIPyServer* server)
: server(server)
{
    eventNumber = 0;
    events = std::make_shared<std::vector<STI::Engine::RawEvent>>();

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
    std::unique_lock<std::mutex> evtLock(eventMutex);

    //RawEvent(const STI::Device::DeviceID& targetDeviceID, double time, unsigned short channel, const STI::Utils::MixedValue& value,
	//const std::string& description, unsigned eventNumber, const RawEventType& eventType)

    std::string description = "";
//    eventNumber++;
    STI::Python::MixedValuePy valuepy; //(value);

    valuepy.setValue_py(value);

    events->push_back(
        STI::Engine::RawEvent(channel.device()->id(), time, channel.channel(), 
                                valuepy, description, events->size(), RawEventType::Play)
            );
}

void STIPyShot::meas(const STIPyChannel& channel, double time, const pybind11::object& value)
{
    std::unique_lock<std::mutex> evtLock(eventMutex);
    
    std::string description = "";
    
    events->push_back(
        STI::Engine::RawEvent(channel.device()->id(), time, channel.channel(), 
                                MixedValuePy(value), description, events->size(), RawEventType::Measurement)
            );
}


ParseTicket STIPyShot::parse()
{
    STI::Engine::ParseID pid;
    pid.parseTimestamp.timestamp = 1.1;
    
    ParseTicket ticket(pid);

 //   ParseTicket ticket;
    return ticket;
}

ParseTicket STIPyShot::parse(const pybind11::dict& channels)
{

    STI::Engine::ParseID pid;
    pid.parseTimestamp.timestamp = 1.1;
    
    ParseTicket ticket(pid);
//    ParseTicket ticket;
    return ticket;
}


void STIPyShot::append(pybind11::object func)
{
}

void STIPyShot::append(const STI::Engine::RawEvent& evt)
{
}

// std::vector<STI::Engine::RawEvent> STIPyShot::getEvents()
// {
//     std::vector<STI::Engine::RawEvent> events;
//     return events;
// }


void STIPyShot::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts)
{
    evts = events;
}

