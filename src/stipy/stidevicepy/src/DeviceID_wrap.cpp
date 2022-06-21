
#include "DeviceID.h"

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

namespace py = pybind11;


void init_DeviceID(py::module& m) 
{
    py::class_<STI::Device::DeviceID>(m, "DeviceID")
        .def(py::init<const std::string&>(), 
                        py::arg("deviceID") )
        .def(py::init<const std::string&, const std::string&, unsigned short>(), 
                        py::arg("name"), py::arg("address"), py::arg("module") )
        .def(py::init<const std::string&, const std::string&, unsigned short, const std::string&>(), 
                        py::arg("name"), py::arg("address"), py::arg("module"), py::arg("targetServerID") )
        .def("getName", &STI::Device::DeviceID::getName)
        .def("getAddress", &STI::Device::DeviceID::getAddress)
        .def("getModule", &STI::Device::DeviceID::getModule)
        .def("getID", &STI::Device::DeviceID::getID)
        .def("getTargetServerID", &STI::Device::DeviceID::getTargetServerID)
        .def("__repr__",
            [](const STI::Device::DeviceID& id) {
                return id.getID();
            })
        // .def(py::hash(py::self))
        .def("__hash__",
            [](const STI::Device::DeviceID& id) {
                // return py::hash(id.getID());
                // return py::hash(py::self);
                std::hash<std::string> hasher;
                auto hash = hasher(id.getID());
                // int h = std::stoi(id.getID());
                return hash;
            })
        .def("__eq__",  // operator ==
            [](const STI::Device::DeviceID& self, const STI::Device::DeviceID& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const STI::Device::DeviceID& self, const STI::Device::DeviceID& rhs) {
                return self < rhs;
            })
        .def("__le__",  // operator <=
            [](const STI::Device::DeviceID& self, const STI::Device::DeviceID& rhs) {
                return (self < rhs) || (self == rhs);
            })
        ;

}

