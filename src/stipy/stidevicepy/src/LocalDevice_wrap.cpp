#include <sti/LocalDevice.h>

#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/LocalAttribute.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EventEngineScheduler.h>

#include "AttributeManagerPy.h"
#include "ChannelManagerPy.h"
#include "DevicePy.h"
#include "LocalDevicePy.h"
#include "MixedValuePy.h"
#include "PersistenceManagerPy.h"
#include "SynchronousEventPy.h"

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>
namespace py = pybind11;

using STI::Python::DevicePy;
using STI::Python::LocalDevicePy;
using STI::Python::LocalDevicePyTrampoline;
using STI::Utils::MixedValueType;
using STI::Device::ChannelType;


void init_LocalDevice(py::module& m) 
{
    py::class_<LocalDevicePy, DevicePy, LocalDevicePyTrampoline, std::shared_ptr<LocalDevicePy>>(m, "LocalDevice") 
        .def(py::init<const std::string&, const std::string&, unsigned short, const std::string&>(), 
                     py::arg("name"), py::arg("address"), py::arg("module"), py::arg("targetServerID") )
        .def("writeChannel", &LocalDevicePy::writeChannel, py::arg("channelNumber"), py::arg("value"))
        .def("readChannel", &LocalDevicePy::readChannel, py::arg("channelNumber"), py::arg("value"))
        .def("parseEvents", &LocalDevicePy::parseEvents, py::arg("eventsIn"), py::arg("synchedEvents")) //py::call_guard<py::gil_scoped_release>() , py::keep_alive<1, 2>() py::return_value_policy::reference
        .def("addChannel", 
            py::overload_cast<unsigned short, ChannelType, MixedValueType, MixedValueType, const std::string&>(&LocalDevicePy::addChannel), 
            py::arg("channelNumber"), py::arg("type"), py::arg("inputType"), py::arg("outputType"), py::arg("defaultName"))
        .def("addPartner", py::overload_cast<const STI::Device::DeviceID&>(&LocalDevicePy::addPartner), 
                py::arg("deviceID"))
        .def("addPartner", py::overload_cast<const STI::Device::DeviceID&, const std::string&>(&LocalDevicePy::addPartner), 
                py::arg("deviceID"), py::arg("alias"))
        .def("addEventTarget", &LocalDevicePy::addEventTarget, py::arg("deviceID"))
        .def("addEventEngine", py::overload_cast<const STI::Engine::EngineID&>(&LocalDevicePy::addEventEngine), py::arg("engineID"))
        //  .def("addAttribute", &LocalDevicePy::addAttribute)
        .def("addAttribute", 
                py::overload_cast<const std::string&, const std::string&>(&LocalDevicePy::addAttribute), 
                py::return_value_policy::reference, py::arg("key"), py::arg("initialValue"))
        //  .def("addAttribute", py::overload_cast<const std::string&, const std::string&, const pybind11::list&>(&LocalDevicePy::addAttribute))
        .def("addAttribute", 
                py::overload_cast<const std::string&, const std::string&, const std::vector<std::string>&>(&LocalDevicePy::addAttribute), 
                py::return_value_policy::reference, py::arg("key"), py::arg("initialValue"), py::arg("allowedValues"))
        .def("addTask", &LocalDevicePy::addTask, py::arg("task"))
        .def("log", py::overload_cast<>(&LocalDevicePy::log))
        .def("log", py::overload_cast<const std::string&>(&LocalDevicePy::log), py::arg("name"))
        .def("partner", py::overload_cast<const STI::Device::DeviceID&>(&LocalDevicePy::partner), py::arg("deviceID"))
        .def("partner", py::overload_cast<const std::string&>(&LocalDevicePy::partner), py::arg("alias"))

        ;

}

