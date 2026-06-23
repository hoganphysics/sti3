#include "DevicePy.h"

#include "AttributeManagerPy.h"
#include "ChannelManagerPy.h"
#include "MixedValuePy.h"
#include "MonitorManagerPy.h"
#include "PersistenceManagerPy.h"
#include "LogBrowser.h"
#include "SynchronousEventPy.h"

#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/LocalAttribute.h>
#include <sti/device/PostProcessingManager.h>
#include <sti/device/TaskManager.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/EngineID.h>

#include <memory>
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>
namespace py = pybind11;

using STI::Python::DevicePy;
using STI::Utils::MixedValueType;
using STI::Device::ChannelType;
using STI::Python::MixedValuePy;
using STI::Device::PostProcessingOptionInfo;
using STI::Device::PostProcessingTargetInfo;

void init_Device(py::module& m)
{
    py::class_<PostProcessingOptionInfo>(m, "PostProcessingOptionInfo")
        .def_readonly("name", &PostProcessingOptionInfo::name)
        .def_readonly("description", &PostProcessingOptionInfo::description)
        .def("__repr__",
            [](const PostProcessingOptionInfo& self) {
                std::stringstream s;
                s << "<PostProcessingOptionInfo " << self.name << ">";
                return s.str();
            });

    py::class_<PostProcessingTargetInfo>(m, "PostProcessingTargetInfo")
        .def_readonly("name", &PostProcessingTargetInfo::name)
        .def_readonly("description", &PostProcessingTargetInfo::description)
        .def_readonly("options", &PostProcessingTargetInfo::options)
        .def("__repr__",
            [](const PostProcessingTargetInfo& self) {
                std::stringstream s;
                s << "<PostProcessingTargetInfo " << self.name
                  << " | " << self.options.size() << " option(s)>";
                return s.str();
            });

    //Returned by LocalDevice.addPostProcessingTarget(); addOption() returns the same
    //builder so option declarations can be chained.
    py::class_<STI::Device::PostProcessingTargetBuilder>(m, "PostProcessingTargetBuilder")
        .def("addOption", &STI::Device::PostProcessingTargetBuilder::addOption,
            py::arg("name"), py::arg("description") = "",
            py::return_value_policy::reference);

    py::class_<DevicePy, std::shared_ptr<DevicePy>>(m, "Device")
        .def(py::init<>())
        .def("getID", &DevicePy::getID)
        .def("kill", &DevicePy::kill)
        .def("refresh", &DevicePy::refresh)
        .def("getDeviceCollection", &DevicePy::getDeviceCollection)
        .def("getMessageDispatcher", &DevicePy::getMessageDispatcher)
        .def("getEngineScheduler", &DevicePy::getEngineScheduler)
        .def("getChannelManager", &DevicePy::getChannelManager)
        .def("getAttributeManager", &DevicePy::getAttributeManager)
        .def("getMonitorManager", &DevicePy::getMonitorManager)
        .def("getPersistenceManager", &DevicePy::getPersistenceManager)
        .def("getProfileManager", &DevicePy::getProfileManager)
        .def("getTaskManager", &DevicePy::getTaskManager)        
        .def("getLogManager", &DevicePy::getLogManager)
        .def("getVersionManager", &DevicePy::getVersionManager)
        .def("getPostProcessingTargets", &DevicePy::getPostProcessingTargets)
        .def("metadata", py::overload_cast<>(&DevicePy::metadata, py::const_))
        .def("metadata", py::overload_cast<const std::string&>(&DevicePy::metadata, py::const_), py::arg("key"))
        .def("openLog",
            [](DevicePy& self, const STI::Device::LogID& logID, std::size_t tailLines) {
                py::gil_scoped_release release;
                return STI::Python::openLog(self.getDevice(), logID, tailLines);
            }, py::arg("logID"), py::arg("tail_lines") = 200)
        .def("write", py::overload_cast<short, const MixedValuePy&>(&DevicePy::write), py::arg("channelNumber"), py::arg("value"))
        .def("write", py::overload_cast<short, const pybind11::object&>(&DevicePy::write), py::arg("channelNumber"), py::arg("value"))
        .def("read", py::overload_cast<short>(&DevicePy::read), py::arg("channelNumber"))
        .def("read", py::overload_cast<short, const MixedValuePy&>(&DevicePy::read), py::arg("channelNumber"), py::arg("value"))
        .def("read", py::overload_cast<short, const pybind11::object&>(&DevicePy::read), py::arg("channelNumber"), py::arg("value"))
        .def("stopRW", &DevicePy::stopRW)
        .def("getAttribute",
            py::overload_cast<const std::string&>(&DevicePy::getAttribute), py::arg("key"))
        .def("setAttribute", &DevicePy::setAttribute, py::arg("key"), py::arg("value"))
        .def("refreshAttribute", &DevicePy::refreshAttribute, py::arg("key"))
        .def("refreshAttributes", &DevicePy::refreshAttributes)
        .def("__repr__", [](const DevicePy& self) {
                return self.getID().getID();
            })
        ;

}
