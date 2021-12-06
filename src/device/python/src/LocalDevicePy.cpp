
#include "LocalDevicePy.h"
#include "DevicePy.h"
#include "ChannelManagerPy.h"
#include "DeviceMessageDispatcher.h"
#include "MixedValue.h"

#include "SynchronousEvent.h"

#include "SynchronousEventPy.h"

#include <iostream>


#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Python::LocalDevicePy;

//using STI::Python::LocalDevicePy;
using STI::Device::LocalDevice;
using STI::Python::DevicePy;
// using STI::Python::DevicePy2;

using STI::Python::ChannelManagerPy;
using STI::Device::ChannelManager;
using STI::Utils::MixedValue;


LocalDevicePy::LocalDevicePy(const std::string& name, const std::string& address, unsigned short module,
    const std::string& targetServer)
: DevicePy()
//: DevicePy2( device = std::make_shared<LocalDevice>(name, address, module, targetServer) )
{
    device = std::make_shared<LocalDevicePy::LocalDeviceDelegate>(this, name, address, module, targetServer);
    setDevice(device);
}

LocalDevicePy::~LocalDevicePy()
{
}


bool LocalDevicePy::write(short channel, const pybind11::object& value)
{  
    return device->write(channel, MixedValuePy(value));
}

pybind11::object LocalDevicePy::read(short channel, const pybind11::object& value)
{
    MixedValue data;
    
    bool success = device->read(channel, MixedValuePy(value), data);

    if (success) {
        MixedValuePy pydata(data);
        return pydata.getValue_py();
    }
    return py::none();
}

void LocalDevicePy::stopRW()
{
    device->stopRW();
}


//Can be overridden in python
bool LocalDevicePy::writeChannel(short channel, const pybind11::object& value)
{
    bool result = false;
    auto mValue = MixedValuePy(value);

    {
        //Need to run in separate thread; release python GIL
        py::gil_scoped_release release;
        result = device->writeChannelDefault(channel, mValue);
    }

    return result;
}

//Can be overridden in python
pybind11::object LocalDevicePy::readChannel(short channel, const pybind11::object& value)
{
    MixedValue data;
    auto mValue = MixedValuePy(value);
    bool success = false;

    {
        //Need to run in separate thread; release python GIL
        py::gil_scoped_release release;
        success = device->readChannelDefault(channel, mValue, data);
    }

    if (success) {
        MixedValuePy pydata(data);
        return pydata.getValue_py();
    }
    return py::none();
}


void LocalDevicePy::LocalDeviceDelegate::parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
{
    try {

        auto manager = std::make_shared<STI::Python::SynchronousEventPyManager>();
        STI::Python::SynchronousEventPy::pyEventManager = manager;      //temporarily store manager in static member

        localDevicePy->parseEvents(events, synchedEvents);

        double holderEventTime = 100;
        if (synchedEvents.size() > 0) {
            std::sort(synchedEvents.begin(), synchedEvents.end(), STI::Utils::compare_shared_ptr<STI::Engine::SynchronousEvent>);
            holderEventTime = synchedEvents.back()->getTime() + 100;
        }

        auto managerHolderEvent = std::make_shared<STI::Python::SynchronousEventPyManagerHolder>(holderEventTime, manager);
        synchedEvents.push_back(managerHolderEvent);   

        STI::Python::SynchronousEventPy::pyEventManager = 0;    //clear static member reference

    }
    catch (py::error_already_set& e) {
        std::cout << "parseEvents exception: " << e.what() << std::endl;
    }


    //synchedEvents.clear();

    // try {
    //     localDevicePy->parseEvents(events, synchedEvents);
    // } catch (py::error_already_set &e) {
    //     //py::print(e.what());
    //     throw;

    //     // if (e.matches(PyExc_FileNotFoundError)) {
    //     //     py::print("missing.txt not found");
    //     // } else if (e.matches(PyExc_PermissionError)) {
    //     //     py::print("missing.txt found but not accessible");
    //     // } else {
    //     //     throw;
    //     // }
    // }
}

