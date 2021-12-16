
#include "LocalDevicePy.h"
#include "DevicePy.h"
#include "ChannelManagerPy.h"
#include "DeviceMessageDispatcher.h"
#include "MixedValue.h"

#include "SynchronousEvent.h"

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
    return device->write(channel, valuepy.getMixedValue());
}

pybind11::object LocalDevicePy::read(short channel, const pybind11::object& value)
{
    MixedValue data;
    // MixedValuePy valuepy;
    MixedValuePy valuepy(value);
    // valuepy.addValue(33.0);

    // {
    //     pybind11::gil_scoped_acquire acquire;
    //     valuepy.setValue_py(value);
    // }

    // std::cout << "LocalDevicePy::read " << valuepy.print() << std::endl;
    
    bool success = device->read(channel, valuepy.getMixedValue(), data);
    // bool success = device->read(channel, MixedValuePy(value), data);

    if (success) {
        pybind11::gil_scoped_acquire acquire;
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
        // std::cout << "LocalDevicePy::writeChannel: " << mValue.print()  << std::endl;
        // result = device->writeChannelDefault(channel, static_cast<MixedValue>(mValue) );
        result = device->writeChannelDefault(channel, mValue.getMixedValue() );
    }

    return result;
}

//Can be overridden in python
pybind11::object LocalDevicePy::readChannel(short channel, const pybind11::object& value)
{
    // pybind11::object obj;

// std::cout << "LocalDevicePy::readChannel start" << std::endl;
    // py::gil_scoped_release release;

    // std::chrono::seconds dura( 5);
    // std::this_thread::sleep_for( dura );

    // pybind11::gil_scoped_acquire acquire;

    // return obj;

    // return py::none();


    MixedValue data;
    // auto mValue = MixedValuePy(value);
    MixedValuePy mValue(value);
    // mValue.setValue_py(value);
    // mValue.addValue(33.0);
    bool success = false;

    {
        //Need to run in separate thread; release python GIL
        py::gil_scoped_release release;
        success = device->readChannelDefault(channel, mValue.getMixedValue(), data);
    }
// std::cout << "LocalDevicePy::readChannel done" << std::endl;
    if (success) {
        // MixedValuePy pydata(data);
        // pybind11::gil_scoped_acquire acquire;
        // py::gil_scoped_release release;
        MixedValuePy pydata;
        pydata.setValue(data);
        // std::cout << "LocalDevicePy::readChannel value: " << pydata.print() << std::endl;
        // return pydata.getValue_py();
        pybind11::object obj = pydata.getValue_py();
        // obj.inc_ref();
        // std::cout << "LocalDevicePy::got object : " << obj.ref_count()  << std::endl;
        return obj;

        // try {
        //     return obj;
        // }
        // catch(py::error_already_set& e) {
        //     std::cout << "readChannel py exception: " << e.what() << std::endl;
        // }
        // catch(...) {
        //     std::cout << "readChannel unknown exception: " << std::endl;
        // }
        // return py::none();
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

