

#include "DeviceMessageDispatcher.h"
#include "DeviceMessage.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;

using STI::Device::DeviceMessageDispatcher;


void init_DeviceMessageDispatcher(py::module& m) 
{

    py::class_<DeviceMessageDispatcher>(m, "DeviceMessageDispatcher")
        .def("addMessage", &DeviceMessageDispatcher::addMessage, 
                            "Overload docstring", py::arg("message") )
        .def("clearMessages", &DeviceMessageDispatcher::clearMessages);



}

