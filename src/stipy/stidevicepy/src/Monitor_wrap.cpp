#include <sti/device/AutoMonitor.h>
#include <sti/device/Monitor.h>
#include <sti/device/LocalMonitor.h>

#include "MixedValuePy.h"

#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using STI::Device::AutoMonitor;
using STI::Device::LocalMonitor;
using STI::Device::Monitor;
using STI::Device::MonitorStatus;
using STI::Python::MixedValuePy;

namespace
{

const char* monitorStatusToString(MonitorStatus status)
{
    switch (status) {
        case MonitorStatus::Active:
            return "Active";
        case MonitorStatus::Inactive:
            return "Inactive";
        case MonitorStatus::Missing:
            return "Missing";
        default:
            return "Unknown";
    }
}

py::dict monitorMetadataToDict(const Monitor& monitor)
{
    py::dict values;

    for (auto& tuple : monitor.getMetaData().getVector()) {
        MixedValuePy pyval(tuple.getVector().at(1));
        values[tuple.getVector().at(0).getString().c_str()] = pyval.getValue_py();
    }

    return values;
}

} // namespace


void init_Monitor(py::module& m)
{
    py::enum_<MonitorStatus>(m, "MonitorStatus")
        .value("Active", MonitorStatus::Active)
        .value("Inactive", MonitorStatus::Inactive)
        .value("Missing", MonitorStatus::Missing)
        ;

    py::class_<Monitor, std::shared_ptr<Monitor>>(m, "Monitor")
        .def("id", &Monitor::getID)
        .def("group", &Monitor::getGroup)
        .def("status", &Monitor::getStatus)
        .def("activate", &Monitor::activate)
        .def("deactivate", &Monitor::deactivate)
        .def("value", [](Monitor& self) {
                MixedValuePy value(self.getValue());
                return value.getValue_py();
            })
        .def("metadata", [](Monitor& self) {
                return monitorMetadataToDict(self);
            })
        .def("metadata", [](Monitor& self, const std::string& key) {
                MixedValuePy value(self.getMetaData(key));
                return value.getValue_py();
            }, py::arg("key"))
        .def("__repr__",
            [](Monitor& monitor) {
                return "<id=" + monitor.getID()
                    + ", group=" + monitor.getGroup()
                    + ", status=" + monitorStatusToString(monitor.getStatus())
                    + ">";
            })
        ;

    py::class_<LocalMonitor, Monitor, std::shared_ptr<LocalMonitor>>(m, "LocalMonitor")
        .def(py::init<const std::string&>(), py::arg("id"))
        .def("setValue",
            [](std::shared_ptr<LocalMonitor>& self, const py::object& value) {
                MixedValuePy mixedValue(value);
                self->setValue(mixedValue.getMixedValue());
                return self;
            }, py::arg("value"))
        .def("addMetadata",
            [](std::shared_ptr<LocalMonitor>& self, const std::string& key, const py::object& value) {
                MixedValuePy mixedValue(value);
                self->addMetaData(key, mixedValue.getMixedValue());
                return self;
            }, py::arg("key"), py::arg("value"))
        ;

    py::class_<AutoMonitor, LocalMonitor, std::shared_ptr<AutoMonitor>>(m, "AutoMonitor");
}
