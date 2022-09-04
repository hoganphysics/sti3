#include "STIPyShot.h"

#include <sti/engine/RawEventTarget.h>
#include <sti/engine/RawEvent.h>

#include "RawEventGroup.h"
#include "RawStackTrace.h"

#include <vector>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using STI::Engine::RawEventTarget;
using STI::Python::STIPyShot;
using STI::Engine::RawStackTrace;
using STI::Engine::RawEventGroup;


void init_STIPyShot(py::module& m) 
{

//    m.def("add", [](int a, int b) { return a + b; });

    py::class_<STIPyShot, std::shared_ptr<STIPyShot>>(m, "STIPyShot")

        .def("setvar", py::overload_cast<const std::string&, const pybind11::object&, 
                        const RawStackTrace&>(&STIPyShot::setvar),
                        py::arg("name"), py::arg("value"), py::arg("stackTrace"))
        .def("setvar", py::overload_cast<const std::string&, const pybind11::object&, 
                        const RawStackTrace&, const std::string&>(&STIPyShot::setvar),
                        py::arg("name"), py::arg("value"), py::arg("stackTrace"), py::arg("scope"))

        .def("event", py::overload_cast<const RawEventTarget&, double, const pybind11::object&, 
                        const RawStackTrace&>(&STIPyShot::event), 
                        py::arg("channel"), py::arg("time"), py::arg("value"), py::arg("stackTrace"))
        .def("event", py::overload_cast<const RawEventTarget&, double, const pybind11::object&, 
                        const RawStackTrace&, const std::string&>(&STIPyShot::event), 
                        py::arg("channel"), py::arg("time"), py::arg("value"), py::arg("stackTrace"), py::arg("scope"))

        .def("meas", py::overload_cast<const RawEventTarget&, double, const RawStackTrace&, 
                    const std::string&>(&STIPyShot::meas), 
                    py::arg("channel"), py::arg("time"), py::arg("stackTrace"), py::arg("scope"))
        .def("meas", py::overload_cast<const RawEventTarget&, double, const pybind11::object&, 
                        const RawStackTrace&, const std::string&>(&STIPyShot::meas), 
                        py::arg("channel"), py::arg("time"), py::arg("value"), py::arg("stackTrace"), py::arg("scope"))

        .def("group", py::overload_cast<>(&STIPyShot::group))
        .def("group", py::overload_cast<const std::string&>(&STIPyShot::group), py::arg("fullName"))

        .def("getEvents", 
            [](STIPyShot& self) {
                auto evts = self.getEvents();

                if (evts != 0) {
                    return *(evts);
                }

                STI::Engine::RawEventVector emptyEvents;
                return emptyEvents;
            }
        )
        .def("getVars", &STI::Python::STIPyShot::getVars)
        ;

}

