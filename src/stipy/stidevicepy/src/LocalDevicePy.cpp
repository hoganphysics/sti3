
#include "LocalDevicePy.h"
#include "DevicePy.h"
#include "ChannelManagerPy.h"
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/utils/MixedValue.h>

#include <sti/engine/SynchronousEvent.h>

#include "SynchronousEventPy.h"
#include "SynchronousEventPyManager.h"

#include <iostream>

#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Python::LocalDevicePy;
using STI::Device::LocalDevice;
using STI::Python::DevicePy;
using STI::Python::ChannelManagerPy;
using STI::Device::ChannelManager;
using STI::Utils::MixedValue;


LocalDevicePy::LocalDevicePy(const std::string& name, const std::string& address, unsigned short module,
    const std::string& targetServer)
: DevicePy()
{
    device = std::make_shared<LocalDevicePy::LocalDeviceDelegate>(this, name, address, module, targetServer);
    setDevice(device);
}

LocalDevicePy::~LocalDevicePy()
{
}


bool LocalDevicePy::write(short channel, const pybind11::object& value)
{  
    MixedValuePy valuepy(value);
    return (device != 0) && device->write(channel, valuepy.getMixedValue());
}

pybind11::object LocalDevicePy::read(short channel, const pybind11::object& value)
{
    MixedValue data;
    MixedValuePy valuepy(value);
   
    bool success = (device != 0) && device->read(channel, valuepy.getMixedValue(), data);

    if (success) {
        pybind11::gil_scoped_acquire acquire;
        MixedValuePy pydata(data);
        return pydata.getValue_py();
    }
    return py::none();
}

void LocalDevicePy::stopRW()
{
    if (device != 0) {
        device->stopRW();
    }
}


//Can be overridden in python
bool LocalDevicePy::writeChannel(short channel, const pybind11::object& value)
{
    bool result = false;
    auto mValue = MixedValuePy(value);

    {
        //Need to run in separate thread; release python GIL
        py::gil_scoped_release release;
        result = (device != 0) && device->writeChannelDefault(channel, mValue.getMixedValue() );
    }

    return result;
}

//Can be overridden in python
pybind11::object LocalDevicePy::readChannel(short channel, const pybind11::object& value)
{
    MixedValue data;
    MixedValuePy mValue(value);

    bool success = false;

    {
        //Need to run in separate thread; release python GIL
        py::gil_scoped_release release;
        success = (device != 0) && device->readChannelDefault(channel, mValue.getMixedValue(), data);
    }

    if (success) {

        MixedValuePy pydata;
        pydata.setValue(data);

        pybind11::object obj = pydata.getValue_py();

        return obj;
    }
    return py::none();
}


void LocalDevicePy::LocalDeviceDelegate::parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
{
    try {

        auto manager = std::make_shared<STI::Python::SynchronousEventPyManager>();
        STI::Python::SynchronousEventPy::pyEventManager = manager;      //temporarily store manager in static member

        if (localDevicePy != 0) {
            localDevicePy->parseEvents(events, synchedEvents);
        }

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

}

