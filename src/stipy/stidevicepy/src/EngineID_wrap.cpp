
#include <sti/engine/EngineID.h>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
// #include <sti/utils/utils.h>

#include <sstream>
using STI::Engine::EngineID;

namespace py = pybind11;


void init_EngineID(py::module& m) 
{
    py::class_<EngineID>(m, "EngineID")
        .def(py::init<>())
        .def(py::init<int>(), py::arg("number"))

        .def("number", &EngineID::getNumber)
        .def("set_number", &EngineID::setNumber, py::arg("number"))

        .def("__repr__",
            [](const EngineID& id) {
                std::stringstream s;
                s << "EngineID (" << id.getNumber() << ")";
                return s.str();
                // return STI::Utils::valueToString(id.getNumber());
            })
        .def("__hash__",
            [](const EngineID& id) {
                std::hash<int> hasher;
                auto hash = hasher(id.getNumber());
                return hash;
            })
        .def("__eq__",  // operator ==
            [](const EngineID& self, const EngineID& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const EngineID& self, const EngineID& rhs) {
                return self < rhs;
            })
        .def("__le__",  // operator <=
            [](const EngineID& self, const EngineID& rhs) {
                return (self < rhs) || (self == rhs);
            })
        ;
}

