
#include "STIPyShot.h"
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/RawEvent.h>
#include "RawEventGroup.h"
#include "StackTracePy.h"
#include "ParsedVarPy.h"

#include <vector>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using STI::Engine::RawEventTarget;
using STI::Python::STIPyShot;
using STI::Python::StackTracePy;


void init_STIPyShot(py::module& m) 
{

//    m.def("add", [](int a, int b) { return a + b; });

    py::class_<STI::Python::STIPyShot, std::shared_ptr<STI::Python::STIPyShot>>(m, "STIPyShot")

        .def("setvar", py::overload_cast<const std::string&, const pybind11::object&, 
                        const StackTracePy&>(&STIPyShot::setvar),
                        py::arg("name"), py::arg("value"), py::arg("stackTrace"))
        .def("setvar", py::overload_cast<const std::string&, const pybind11::object&, 
                        const StackTracePy&, const STI::Engine::RawEventGroup&>(&STIPyShot::setvar),
                        py::arg("name"), py::arg("value"), py::arg("stackTrace"), py::arg("scope"))
        .def("getvar", py::overload_cast<const std::string&>(&STIPyShot::getvar), py::arg("name"))
        .def("getvar", py::overload_cast<const std::string&, const STI::Engine::RawEventGroup&>(&STIPyShot::getvar),
                        py::arg("name"), py::arg("scope"))


        .def("event", &STI::Python::STIPyShot::event, py::arg("channel"), 
                        py::arg("time"), py::arg("value"), py::arg("stackTrace"), py::arg("group"))
        .def("meas", py::overload_cast<const RawEventTarget&, double, const pybind11::object&, 
                        const StackTracePy&, const STI::Engine::RawEventGroup&>(&STIPyShot::meas),
                        py::arg("channel"), py::arg("time"), py::arg("value"), py::arg("stackTrace"), py::arg("group"))
        .def("meas", py::overload_cast<const RawEventTarget&, double, const StackTracePy&, 
                        const STI::Engine::RawEventGroup&>(&STIPyShot::meas),
                        py::arg("channel"), py::arg("time"), py::arg("stackTrace"), py::arg("group"))
        .def("getEvents", &STI::Python::STIPyShot::getEvents)
        .def("getVars", &STI::Python::STIPyShot::getVars)


        .def("group", py::overload_cast<const std::string&>(&STIPyShot::group),
                        py::arg("name"))
        .def("group", py::overload_cast<const std::string&, const STI::Engine::RawEventGroup&>(&STIPyShot::group),
                        py::arg("name"), py::arg("parentGroup"))

        // .def("getEvents",
        //     [](STI::Python::STIPyShot& self) {
        //         std::shared_ptr<std::vector<STI::Engine::RawEvent>> evts;
        //         self.getEvents(evts);
        //         return (*evts);
        //     }
        // )
        ;

}

