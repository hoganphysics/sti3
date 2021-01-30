
#include "LocalDevicePy.h"
#include "DevicePy.h"
#include "ChannelManagerPy.h"
#include "DeviceMessageDispatcher.h"

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



LocalDevicePy::LocalDevicePy(const std::string& name, const std::string& address, unsigned short module,
    const std::string& targetServer)
: DevicePy()
//: DevicePy2( device = std::make_shared<LocalDevice>(name, address, module, targetServer) )
{
    device = std::make_shared<LocalDevicePy::PyLocalDevice>(this, name, address, module, targetServer);
    setDevice(device);
}

LocalDevicePy::~LocalDevicePy()
{
}

bool LocalDevicePy::writeChannel(short channel, const pybind11::object& value)
{
    return false;
}

pybind11::object LocalDevicePy::readChannel(short channel, const pybind11::object& value)
{
    return py::none();
}





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

