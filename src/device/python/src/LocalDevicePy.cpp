
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
    // std::cout << "LocalDevicePy::write " << device->getID().getID() << std::endl;

    // bool result = false;
    // //auto mVal = MixedValuePy(value);


    // //auto tmp = MixedValuePy(value);
    // //
    // {
    //     //py::gil_scoped_release release;
    //     result = device->write(channel, MixedValuePy(value));
    // //return device->write(channel, 11.0);
    // }
    
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
    // MixedValue mVal = static_cast<MixedValue>(MixedValuePy(value));

    auto mValue = MixedValuePy(value);
    std::cout << "-----------------" << std::endl;
//    auto mVal = static_cast<MixedValue>(tmpVal);
//    MixedValue mVal = tmpVal;

    {
        //Need to run in separate thread; release python GIL
        py::gil_scoped_release release;
        result = device->writeChannelDefault(channel, mValue);
    }
    
    //py::gil_scoped_acquire acquire;

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

void LocalDevicePy::runTest2()
{
    std::vector<std::shared_ptr<STI::Python::A>> avec;
    // auto a = std::make_shared<STI::Python::A>(99);
    // avec.push_back(a);
    testVector2(avec);

    std::cout << "LocalDevicePy::runTest2() len=" << avec.size() << std::endl;
    for (auto& x : avec) {
        x->run();
        std::cout << std::hex << x.get() << std::dec << std::endl;
    }
}

void LocalDevicePy::testVector2(std::vector<std::shared_ptr<STI::Python::A>>& avec)
{

}

void LocalDevicePy::runTest()
{
    std::vector<int> testvec;
    testvec.push_back(33);

    testVector(testvec);

    std::cout << "LocalDevicePy::runTest() len=" << testvec.size() << std::endl;
    for (auto& x : testvec) {
        std::cout << "c++ val=" << x << std::endl;
    }

}

void LocalDevicePy::testVector(std::vector<int>& input)
{
}

void LocalDevicePy::LocalDeviceDelegate::parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
{
    try {

        auto manager = std::make_shared<STI::Python::SynchronousEventPyManager>();
        STI::Python::SynchronousEventPy::pyEventManager = manager;      //temporarily store manager in static member
        // STI::Python::SynchronousEventPy::pyEventManager = 0;
        // STI::Python::SynchronousEventPy::temp = 22;

        // py::gil_scoped_release release;
        std::cout << "LocalDeviceDelegate::parseEvents len=" << synchedEvents.size() << std::endl;
        localDevicePy->parseEvents(events, synchedEvents);
        std::cout << "LocalDeviceDelegate::parseEvents len=" << synchedEvents.size() << std::endl;

        std::cout << "++++++++++++++LocalDeviceDelegate::parseEvents manager=" << manager->pySynchronousEventsRefs.size() << std::endl;

        if (synchedEvents.size() > 0) {
            std::cout << "LocalDeviceDelegate::parseEvents time=" << synchedEvents.at(0)->getTime() << std::endl;
            // std::cout << "LocalDeviceDelegate::parseEvents count=" << synchedEvents.at(0).use_count() << std::endl;
            // synchedEvents.at(0)->playEvent();
        }

        double holderEventTime = 100;

        if (synchedEvents.size() > 0) {
            
            std::sort(synchedEvents.begin(), synchedEvents.end(), STI::Utils::compare_shared_ptr<STI::Engine::SynchronousEvent>);

            holderEventTime = synchedEvents.back()->getTime() + 100;
        }

        auto managerHolderEvent = std::make_shared<STI::Python::SynchronousEventPyManagerHolder>(holderEventTime, manager);


        // synchedEvents.insert(synchedEvents.begin(), managerHolderEvent);

        synchedEvents.push_back(managerHolderEvent);   

        // synchedEvents.clear();
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



// using STI::Python::LocalDevicePyTrampoline;

// void LocalDevicePyTrampoline::parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
// {
//     //pybind11::gil_scoped_acquire acquire;

//     //py::object dummy = py::cast(&synchedEvents);   // force re-use in the following call

//     PYBIND11_OVERRIDE(
//         void,     /* Return type */
//         LocalDevicePy,       /* Parent class */
//         parseEvents,          /* Name of function in C++ (must match Python name) */
//         events, synchedEvents        /* Argument(s) */
//     );
// }





// LocalDevicePy::LocalDevicePy(const std::string& name, const std::string& address, unsigned short module,
//     const std::string& targetServer)
// : LocalDevice(name, address, module, targetServer)
// //LocalDevicePy::LocalDevicePy()
// {

// }

// LocalDevicePy::~LocalDevicePy()
// {

// }

// std::shared_ptr<STI::Device::DeviceMessageDispatcher> LocalDevicePy::getMessageDispatcher()
// {
//     std::shared_ptr<STI::Device::DeviceMessageDispatcher> dispatcher;
    
//     LocalDevice::getMessageDispatcher(dispatcher);

//     return dispatcher;
// }

// std::shared_ptr<ChannelManagerPy> LocalDevicePy::getChannelManager()
// {
//     std::shared_ptr<ChannelManager> manager;
//     std::shared_ptr<ChannelManagerPy> wrapper;

//     LocalDevice::getChannelManager(manager);

//     if (manager != 0) {
//         wrapper = std::make_shared<ChannelManagerPy>(manager);
//     }

//     return wrapper;
// }

// STI::Device::DeviceID LocalDevicePy::getIDpy()
// {
//     //int y = getID().getModule();
//     STI::Device::DeviceID y = getID();
//     return y;
// }


// const STI::Device::DeviceID LocalDevicePy::getIDpy() const
// {
//     std::cout << "getID()" << std::endl;

//     return LocalDevice::getID();
// }

// std::shared_ptr<STI::Device::DeviceMessageDispatcher> LocalDevicePy::getMessageDispatcher()
// {
//     std::shared_ptr<STI::Device::DeviceMessageDispatcher> dispatcher;
    
//     LocalDevice::getMessageDispatcher(dispatcher);

//     return dispatcher;
// }

// //    std::shared_ptr<EventEngineSchedulerPy> getEngineScheduler();    

// std::shared_ptr<ChannelManagerPy> LocalDevicePy::getChannelManager()
// {
//     std::shared_ptr<ChannelManager> manager;
//     std::shared_ptr<ChannelManagerPy> wrapper;

//     LocalDevice::getChannelManager(manager);

//     if (manager != 0) {
//         wrapper = std::make_shared<ChannelManagerPy>(manager);
//     }

//     return wrapper;
// }



//Hooks to be implemented in python:
// bool LocalDevicePy::writeChannelPy(short channel, const STI::Python::MixedValuePy& value)
// {
//     return false;
// }

//pybind11::object LocalDevicePy::readChannelPy(short channel, const STI::Python::MixedValuePy& value)



// //Overrides for STI::Device::LocalDevice
// bool LocalDevicePy::writeChannel(short channel, const STI::Utils::MixedValue& value)
// {
//     return false;
// }

// bool LocalDevicePy::readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
// {
//     return false;
// }

