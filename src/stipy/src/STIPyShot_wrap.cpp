#include "STIPyShot.h"

#include <sti/engine/RawEventTarget.h>
#include <sti/engine/RawEvent.h>

#include <sti/engine/RawEventGroup.h>
#include "StackTrace.h"

#include <vector>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using STI::Engine::RawEventTarget;
using STI::Python::STIPyShot;
using STI::Engine::StackTrace;
using STI::Engine::RawEventGroup;


void init_STIPyShot(py::module& m) 
{

//    m.def("add", [](int a, int b) { return a + b; });

    py::class_<STIPyShot, std::shared_ptr<STIPyShot>>(m, "STIPyShot")
    
        .def("shotconfig", &STIPyShot::getShotConfig)
        .def("setvar", py::overload_cast<const std::string&, const pybind11::object&, 
                        const StackTrace&>(&STIPyShot::setvar),
                        py::arg("name"), py::arg("value"), py::arg("stackTrace"))
        .def("setvar", py::overload_cast<const std::string&, const pybind11::object&, 
                        const StackTrace&, const std::string&>(&STIPyShot::setvar),
                        py::arg("name"), py::arg("value"), py::arg("stackTrace"), py::arg("scope"))

        .def("event", py::overload_cast<const RawEventTarget&, double, const pybind11::object&, 
                        const StackTrace&>(&STIPyShot::event), 
                        py::arg("channel"), py::arg("time"), py::arg("value"), py::arg("stackTrace"))
        .def("event", py::overload_cast<const RawEventTarget&, double, const pybind11::object&, 
                        const StackTrace&, const std::string&>(&STIPyShot::event), 
                        py::arg("channel"), py::arg("time"), py::arg("value"), py::arg("stackTrace"), py::arg("scope"))

        .def("meas", py::overload_cast<const RawEventTarget&, double, const StackTrace&, 
                    const std::string&>(&STIPyShot::meas), 
                    py::arg("channel"), py::arg("time"), py::arg("stackTrace"), py::arg("scope"))
        .def("meas", py::overload_cast<const RawEventTarget&, double, const pybind11::object&, 
                        const StackTrace&, const std::string&>(&STIPyShot::meas), 
                        py::arg("channel"), py::arg("time"), py::arg("value"), py::arg("stackTrace"), py::arg("scope"))
        .def("set_trigger", &STIPyShot::set_trigger, py::arg("deviceID"), py::arg("stackTrace"))

        .def("rootgroup", py::overload_cast<>(&STIPyShot::group))
        .def("group", py::overload_cast<const std::string&>(&STIPyShot::group), py::arg("fullName"))

        .def("vars", &STI::Python::STIPyShot::getVars)
        ;

}

