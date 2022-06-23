

#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/DeviceMessage.h>

#include <memory>

#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Device::DeviceMessageDispatcher;


void init_DeviceMessageDispatcher(py::module& m) 
{

    py::class_<DeviceMessageDispatcher, std::shared_ptr<DeviceMessageDispatcher>>(m, "DeviceMessageDispatcher")
        .def("addMessage", &DeviceMessageDispatcher::addMessage, 
                            "Add message to outgoing queue", py::arg("message") )
        .def("clearMessages", &DeviceMessageDispatcher::clearMessages);


}

