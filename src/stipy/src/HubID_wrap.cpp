
#include "HubID.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;


void init_HubID(py::module& m) 
{

//    m.def("add", [](int a, int b) { return a + b; });

    py::class_<STI::Network::HubID>(m, "HubID")
        .def(py::init<const std::string&, const std::string&, unsigned short>(), 
                        py::arg("name"), py::arg("address"), py::arg("module") )
        .def_readwrite("name", &STI::Network::HubID::name)
        .def_readwrite("address", &STI::Network::HubID::address)
        .def_readwrite("module", &STI::Network::HubID::module)
        .def("getID", &STI::Network::HubID::id)
        .def("__repr__",
            [](const STI::Network::HubID& id) {
                return id.id();
            })
        .def("__eq__",  // operator ==
            [](const STI::Network::HubID& self, const STI::Network::HubID& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const STI::Network::HubID& self, const STI::Network::HubID& rhs) {
                return self < rhs;
            })
        .def("__le__",  // operator <=
            [](const STI::Network::HubID& self, const STI::Network::HubID& rhs) {
                return (self < rhs) || (self == rhs);
            });

}

