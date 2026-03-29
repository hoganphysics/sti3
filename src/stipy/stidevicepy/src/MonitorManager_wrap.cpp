#include "MonitorManagerPy.h"

#include "MixedValuePy.h"

#include <sti/device/Monitor.h>
#include <sti/device/MonitorManager.h>

#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using STI::Python::MixedValuePy;
using STI::Python::MonitorManagerPy;


void init_MonitorManager(py::module& m)
{
    py::class_<MonitorManagerPy, std::shared_ptr<MonitorManagerPy>>(m, "MonitorManager")
        .def("getIDs", py::overload_cast<>(&MonitorManagerPy::getIDs))
        .def("getMonitor", py::overload_cast<const std::string&>(&MonitorManagerPy::getMonitor), py::arg("id"))
        .def("getMonitors", py::overload_cast<>(&MonitorManagerPy::getMonitors))
        .def("getStatus", &MonitorManagerPy::getStatus, py::arg("id"))
        .def("getValue", [](MonitorManagerPy& self, const std::string& id) {
                STI::Utils::MixedValue rawValue;
                {
                    py::gil_scoped_release release;
                    rawValue = self.getValue(id);
                }

                MixedValuePy value(rawValue);
                return value.getValue_py();
            }, py::arg("id"))
        .def("activate", &MonitorManagerPy::activate, py::arg("id"))
        .def("deactivate", &MonitorManagerPy::deactivate, py::arg("id"))
        .def("activateAll", &MonitorManagerPy::activateAll)
        .def("deactivateAll", &MonitorManagerPy::deactivateAll)
        ;
}
