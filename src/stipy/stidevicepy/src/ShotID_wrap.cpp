
#include <sti/engine/ShotID.h>
#include <sti/engine/ParseID.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

using STI::Engine::ShotID;


void init_ShotID(py::module& m)
{

    py::class_<STI::Engine::ShotID>(m, "ShotID")
        .def(py::init<>())
        .def(py::init<const STI::Engine::ParseID&, const STI::Engine::EngineJobSourceID&>(), py::arg("pid"), py::arg("jobSourceID"))
        .def(py::init<const STI::Engine::ParseID&, const STI::Engine::EngineJobSourceID&, const STI::Utils::TimeStamp&>(), 
                py::arg("pid"), py::arg("jobSourceID"), py::arg("submissionTime"))
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
