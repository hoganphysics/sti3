


#include "STIPyDevice.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;


void init_STIPyDevice(py::module& m) 
{

//    m.def("add", [](int a, int b) { return a + b; });

    py::class_<STI::Python::STIPyDevice, std::shared_ptr<STI::Python::STIPyDevice>>(m, "STIPyDevice")
        .def(py::init<const std::string&, const std::string&, unsigned short>(), 
                        py::arg("name"), py::arg("address"), py::arg("module") )
        .def("getID", &STI::Python::STIPyDevice::id)
        .def("__repr__",
            [](const STI::Device::DeviceID& id) {
                return id.getID();
            })
        ;

}

