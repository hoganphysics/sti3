


#include "STIPyDevice.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;


void init_STIPyDevice(py::module& m) 
{

    py::class_<STI::Python::STIPyDevice, std::shared_ptr<STI::Python::STIPyDevice>>(m, "STIPyDevice")
        .def(py::init<const std::string&, const std::string&, unsigned short, const std::string&>(), 
                        py::arg("name"), py::arg("address"), py::arg("module"), py::arg("targetServerID") )
        .def("getID", &STI::Python::STIPyDevice::id)
        .def("__repr__",
            [](const STI::Device::DeviceID& id) {
                return id.getID();
            })
        ;

}

