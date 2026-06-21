#include <sti/LocalDevice.h>

#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/LocalAttribute.h>
#include <sti/device/VersionInfo.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EventEngineScheduler.h>

#include "AttributeManagerPy.h"
#include "ChannelManagerPy.h"
#include "DevicePy.h"
#include "LocalDevicePy.h"
#include "MixedValuePy.h"
#include "PersistenceManagerPy.h"
#include "SynchronousEventPy.h"
#include "TaskPy.h"

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>
namespace py = pybind11;

using STI::Python::DevicePy;
using STI::Python::LocalDevicePy;
using STI::Python::LocalDevicePyTrampoline;
using STI::Utils::MixedValueType;
using STI::Device::ChannelType;
using STI::Python::TaskPy;
using STI::Utils::Task;


void init_LocalDevice(py::module& m) 
{

    py::class_<LocalDevicePy, DevicePy, LocalDevicePyTrampoline, std::shared_ptr<LocalDevicePy>>(m, "LocalDevice") 
        .def(py::init<const std::map<std::string, std::string>&>(), py::arg("config") )
        .def(py::init<const STI::Utils::Configuration&>(), py::arg("config") )
        .def(py::init<const STI::Utils::Configuration&, const std::string&>(), py::arg("config"), py::arg("section") )
        .def(py::init<const std::string&, const std::string&, unsigned short, const std::string&>(), 
                     py::arg("name"), py::arg("address"), py::arg("module"), py::arg("targetServerID") )
        .def(py::init<const std::string&, const std::string&, unsigned short, const std::string&, const STI::Utils::Configuration&>(), 
                     py::arg("name"), py::arg("address"), py::arg("module"), py::arg("targetServerID"), py::arg("config") )

        .def("writeChannel", &LocalDevicePy::writeChannel, py::arg("channelNumber"), py::arg("value"))
        .def("readChannel", &LocalDevicePy::readChannel, py::arg("channelNumber"), py::arg("value"))
        // .def("parseEvents", &LocalDevicePy::parseEvents, py::arg("eventsIn"), py::arg("synchedEvents")) //py::call_guard<py::gil_scoped_release>() , py::keep_alive<1, 2>() py::return_value_policy::reference
        .def("parseEventsWrapper", &LocalDevicePy::parseEventsWrapper, py::arg("eventsIn"), py::arg("synchedEvents"))
        .def("parseEventsDefault", &LocalDevicePy::parseEventsDefault, py::arg("eventsIn"), py::arg("synchedEvents"))
        .def("addInfo", &LocalDevicePy::addInfo, py::arg("id"), py::arg("name"))
        .def("addWarning", &LocalDevicePy::addWarning, py::arg("id"), py::arg("name"))
        .def("throwConflictException", 
            py::overload_cast<const STI::Engine::RawEvent&, const std::string&>(&LocalDevicePy::throwConflictException), 
            py::arg("evt"), py::arg("message"))
        .def("throwConflictException", 
            py::overload_cast<const STI::Engine::RawEvent&, const STI::Engine::RawEvent&, const std::string&>(&LocalDevicePy::throwConflictException), 
            py::arg("event1"), py::arg("event2"), py::arg("message"))
        
        .def("throwParsingException", 
            py::overload_cast<const STI::Engine::RawEvent&, const std::string&>(&LocalDevicePy::throwParsingException), 
            py::arg("evt"), py::arg("message"))
        .def("throwPythonException", &LocalDevicePy::throwPythonException, py::arg("message"))
        .def("addChannel", 
            py::overload_cast<unsigned short, ChannelType, MixedValueType, MixedValueType, const std::string&>(&LocalDevicePy::addChannel), 
            py::arg("channelNumber"), py::arg("type"), py::arg("inputType"), py::arg("outputType"), py::arg("defaultName"))
        .def("addInputChannel", 
            py::overload_cast<unsigned short, STI::Utils::MixedValueType, const std::string&>(&LocalDevicePy::addInputChannel), 
            py::arg("channelNumber"), py::arg("inputType"), py::arg("defaultName"))
        .def("addInputChannel", 
            py::overload_cast<unsigned short, STI::Utils::MixedValueType, STI::Utils::MixedValueType, const std::string&>(&LocalDevicePy::addInputChannel), 
            py::arg("channelNumber"), py::arg("inputType"), py::arg("outputType"), py::arg("defaultName"))
        .def("addOutputChannel", 
            py::overload_cast<unsigned short, STI::Utils::MixedValueType, const std::string&>(&LocalDevicePy::addOutputChannel), 
            py::arg("channelNumber"), py::arg("outputType"), py::arg("defaultName"))

        .def("addPartner", py::overload_cast<const STI::Device::DeviceID&>(&LocalDevicePy::addPartner), 
                py::arg("deviceID"))
        .def("addPartner", py::overload_cast<const STI::Device::DeviceID&, const std::string&>(&LocalDevicePy::addPartner), 
                py::arg("deviceID"), py::arg("alias"))
        .def("addEventTarget", py::overload_cast<const STI::Device::DeviceID&>(&LocalDevicePy::addEventTarget), 
                py::arg("deviceID"))
        .def("addEventTarget", py::overload_cast<const STI::Device::DeviceID&, const std::string&>(&LocalDevicePy::addEventTarget), 
                py::arg("deviceID"), py::arg("alias"))
        .def("addEventEngine", py::overload_cast<const STI::Engine::EngineID&>(&LocalDevicePy::addEventEngine), py::arg("engineID"))
        //  .def("addAttribute", &LocalDevicePy::addAttribute)
        .def("addAttribute", 
                py::overload_cast<const std::string&, const std::string&>(&LocalDevicePy::addAttribute), 
                py::return_value_policy::reference, py::arg("key"), py::arg("initialValue"))
        //  .def("addAttribute", py::overload_cast<const std::string&, const std::string&, const pybind11::list&>(&LocalDevicePy::addAttribute))
        .def("addAttribute", 
                py::overload_cast<const std::string&, const std::string&, const std::vector<std::string>&>(&LocalDevicePy::addAttribute), 
                py::return_value_policy::reference, py::arg("key"), py::arg("initialValue"), py::arg("allowedValues"))
        .def("addMonitor",
                py::overload_cast<const std::string&>(&LocalDevicePy::addMonitor),
                py::return_value_policy::reference, py::arg("id"))
        .def("addMonitor",
                py::overload_cast<const std::shared_ptr<STI::Device::LocalMonitor>&>(&LocalDevicePy::addMonitor),
                py::return_value_policy::reference, py::arg("monitor"))
        .def("addAutoMonitor",
                &LocalDevicePy::addAutoMonitor,
                py::return_value_policy::reference,
                py::arg("id"), py::arg("updateInterval_s"), py::arg("updater"))
        
        .def("__addTask_Base", py::overload_cast<const std::shared_ptr<Task>&>(&LocalDevicePy::addTask), py::arg("task"))
        // .def("addTask", py::overload_cast<const std::shared_ptr<TaskPy>&>(&LocalDevicePy::addTask), py::arg("task"))
        .def("__addTask", py::overload_cast<const std::shared_ptr<STI::Python::TaskPy>&, const pybind11::object&>(&LocalDevicePy::addTask), py::arg("task"), py::arg("object"))
        .def("addPostProcessingTarget", &LocalDevicePy::addPostProcessingTarget,
                py::arg("name"), py::arg("function"), py::arg("description") = "")
        .def("addMetadata",
            [](std::shared_ptr<LocalDevicePy>& self, const std::string& key, const pybind11::object& value) {
                self->addMetadata(key, value);
                return self;
            }, py::arg("key"), py::arg("value"))
        .def("setColor",
            [](std::shared_ptr<LocalDevicePy>& self, const std::string& color) {
                self->setColor(color);
                return self;
            }, py::arg("color"))
        .def("setDescription",
            [](std::shared_ptr<LocalDevicePy>& self, const std::string& description) {
                self->setDescription(description);
                return self;
            }, py::arg("description"))
        .def("setHelp",
            [](std::shared_ptr<LocalDevicePy>& self, const std::string& help) {
                self->setHelp(help);
                return self;
            }, py::arg("help"))
        .def("addVersionInfo",
                py::overload_cast<const STI::Device::VersionInfo&>(&LocalDevicePy::addVersionInfo),
                py::arg("version"))
        .def("addVersionInfo",
                py::overload_cast<const std::string&, const std::string&>(&LocalDevicePy::addVersionInfo),
                py::arg("component"), py::arg("version"))

        .def("log", py::overload_cast<>(&LocalDevicePy::log))
        .def("log", py::overload_cast<const std::string&>(&LocalDevicePy::log), py::arg("name"))
        .def("partner", py::overload_cast<const STI::Device::DeviceID&>(&LocalDevicePy::partner), py::arg("deviceID"))
        .def("partner", py::overload_cast<const std::string&>(&LocalDevicePy::partner), py::arg("alias"))
        .def("makeVirtualFileHolder", &LocalDevicePy::makeVirtualFileHolder, py::arg("path"), py::arg("filename"))

        .def("getMessageReceiver", &LocalDevicePy::getMessageReceiver)
        ;

}
