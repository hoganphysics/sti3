
#include <sti/network/HubID.h>

#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Network::HubID;


void init_HubID(py::module& m) 
{

    py::class_<HubID>(m, "HubID")
        .def(py::init<const std::string&, const std::string&, unsigned short>(), 
                        py::arg("name"), py::arg("address"), py::arg("module") )
        .def_readwrite("name", &HubID::name)
        .def_readwrite("address", &HubID::address)
        .def_readwrite("module", &HubID::module)
        .def("getID", &HubID::getID)
        .def("toString", &STI::Network::HubID::getID)
        .def("__repr__",
            [](const HubID& id) {
                return id.getID();
            })
        .def("__eq__",  // operator ==
            [](const HubID& self, const HubID& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const HubID& self, const HubID& rhs) {
                return self < rhs;
            })
        .def("__le__",  // operator <=
            [](const HubID& self, const HubID& rhs) {
                return (self < rhs) || (self == rhs);
            })
        ;

}

