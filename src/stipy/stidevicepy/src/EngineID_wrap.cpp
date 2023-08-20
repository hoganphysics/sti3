
#include <sti/engine/EngineID.h>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

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
                return id.getNumber();
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

