#include "DevicePy.h"

#include "AttributeManagerPy.h"
#include "ChannelManagerPy.h"
#include "MixedValuePy.h"
#include "PersistenceManagerPy.h"
#include "SynchronousEventPy.h"

#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/LocalAttribute.h>
#include <sti/device/TaskManager.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/EngineID.h>

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>
namespace py = pybind11;

using STI::Python::DevicePy;
using STI::Utils::MixedValueType;
using STI::Device::ChannelType;


void init_Device(py::module& m)
{
    py::class_<DevicePy, std::shared_ptr<DevicePy>>(m, "Device")
        .def(py::init<>())
        .def("getID", &DevicePy::getID)
        .def("kill", &DevicePy::kill)
        .def("getDeviceCollection", &DevicePy::getDeviceCollection)
        .def("getMessageDispatcher", &DevicePy::getMessageDispatcher)
        .def("getEngineScheduler", &DevicePy::getEngineScheduler)
        .def("getChannelManager", &DevicePy::getChannelManager)
        .def("getAttributeManager", &DevicePy::getAttributeManager)
        .def("getPersistenceManager", &DevicePy::getPersistenceManager)
        .def("getProfileManager", &DevicePy::getProfileManager)
        .def("getTaskManager", &DevicePy::getTaskManager)        
        .def("getLogManager", &DevicePy::getLogManager)
        .def("write", &DevicePy::write, py::arg("channelNumber"), py::arg("value"))
        .def("read", &DevicePy::read, py::arg("channelNumber"), py::arg("value"))
        .def("stopRW", &DevicePy::stopRW)
        .def("getAttribute",
            py::overload_cast<const std::string&>(&DevicePy::getAttribute), py::arg("key"))
        .def("setAttribute", &DevicePy::setAttribute, py::arg("key"), py::arg("value"))
        ;

}

