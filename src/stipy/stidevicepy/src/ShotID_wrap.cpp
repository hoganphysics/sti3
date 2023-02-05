
#include <sti/engine/ShotID.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

using STI::Engine::ShotID;


void init_ShotID(py::module& m)
{

    py::class_<STI::Engine::ShotID>(m, "ShotID")
        .def(py::init<>())
        .def_readonly("submissionTime", &ShotID::submissionTime)
        .def_readonly("parseID", &ShotID::parseID)
        .def_readonly("jobSourceID", &ShotID::jobSourceID)
        .def("__repr__",
            [](const ShotID& self) {
                return self.print();
            })
        .def("__eq__",  // operator ==
            [](const ShotID& self, const ShotID& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const ShotID& self, const ShotID& other) {
                return self < other;
            })
        ;

}
