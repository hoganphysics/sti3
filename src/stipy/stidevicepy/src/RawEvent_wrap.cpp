
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventTarget.h>

#include <sti/device/DeviceID.h>
#include <sti/utils/MixedValue.h>
#include <sti/utils/utils.h>
#include "MixedValuePy.h"
#include <sti/engine/RawEventGroup.h>
#include "StackTrace.h"

#include <sstream>

#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Engine::RawEventType;
using STI::Python::MixedValuePy;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventTargetChannel;
using STI::Engine::RawEventTargetDevice;


void init_RawEvent(py::module& m) 
{

    py::class_<RawEventTargetChannel>(m, "RawEventTargetChannel")
        .def(py::init<const std::string&>(), py::arg("name"))
        .def(py::init<unsigned short>(), py::arg("channel"))

        .def("isAbstract", &RawEventTargetChannel::isAbstract)
        .def("name", &RawEventTargetChannel::name)
        .def("channel", &RawEventTargetChannel::channel)
        .def("setChannel", &RawEventTargetChannel::setChannel)
        .def("__repr__",
            [](const STI::Engine::RawEventTargetChannel& self) {
                std::stringstream s;
                
                s << "ch(";

                if (self.isAbstract()) {
                    s << self.name() << ")" << " <Abstract>";
                }
                else {
                    s << self.channel() << ")";
                }
                return s.str();
            })
        ;

    py::class_<RawEventTargetDevice>(m, "RawEventTargetDevice")
        .def(py::init<const std::string&>(), py::arg("name"))
        .def(py::init<const STI::Device::DeviceID&>(), py::arg("targetDevice"))

        .def("isAbstract", &RawEventTargetDevice::isAbstract)
        .def("name", &RawEventTargetDevice::name)
        .def("deviceID", &RawEventTargetDevice::deviceID)
        .def("setTargetDeviceID", &RawEventTargetDevice::setTargetDeviceID)
        .def("__repr__",
            [](const STI::Engine::RawEventTargetDevice& self) {
                std::stringstream s;
                
                s << "dev(";

                if (self.isAbstract()) {
                    s << self.name() << ")" << " <Abstract>";
                }
                else {
                    s << self.deviceID().getID() << ")";
                }
                return s.str();
            })
        ;

    py::class_<RawEventTarget>(m, "RawEventTarget")
        .def(py::init<const RawEventTargetDevice&, const RawEventTargetChannel&>(), py::arg("dev"), py::arg("ch"))
        .def(py::init<const STI::Device::DeviceID&, unsigned short>(), py::arg("targetDevice"), py::arg("channel"))
        .def(py::init<const STI::Device::DeviceID&, const std::string&>(), py::arg("targetDevice"), py::arg("channelName"))
        .def(py::init<const std::string&, unsigned short>(), py::arg("deviceName"), py::arg("channel"))
        .def(py::init<const std::string&, const std::string&>(), py::arg("deviceName"), py::arg("channelName"))
        .def(py::init<const std::string&>(), py::arg("channelName"))

        .def("isAbstract", &RawEventTarget::isAbstract)
        .def("device", &RawEventTarget::getDevice)
        .def("channel", &RawEventTarget::getChannel)
        .def("__repr__",
            [](const STI::Engine::RawEventTarget& self) {
                std::stringstream s;
                
                s << "dev(";
                if (self.device().isAbstract()) {
                    s << self.device().name();
                }
                else {
                    s << self.device().deviceID().getID();
                }
                s << ").ch(";
                if (self.channel().isAbstract()) {
                    s << self.channel().name();
                }
                else {
                    s << self.channel().channel();
                }
                s << ")";
                if (self.isAbstract()) {
                    s << " <Abstract>";
                }
                return s.str();
            })
        ;


    //RawEventType { Play, Measurement, Waveform, Pause, Jump };

    py::enum_<RawEventType>(m, "RawEventType")
        .value("Play", RawEventType::Play)
        .value("Measurement", RawEventType::Measurement)
        .value("Waveform", RawEventType::Waveform)
        .value("Pause", RawEventType::Pause)
        .value("Jump", RawEventType::Jump)
        //.export_values()
        ;




    py::class_<STI::Engine::RawEvent>(m, "RawEvent")
        .def(py::init<>())
        .def(py::init(
            [](const STI::Engine::RawEventTarget& eventTarget,
                double time, const pybind11::object& value, unsigned eventNumber, const RawEventType& eventType) 
                {
                    return new STI::Engine::RawEvent(eventTarget, time, MixedValuePy(value), eventNumber, eventType);
                }), 
                py::arg("eventTarget"), py::arg("time"), py::arg("value"),
                py::arg("eventNumber"), py::arg("eventType") )
        // .def(py::init(
        //     [](const STI::Engine::RawEventTarget& eventTarget,
        //         double time, const pybind11::object& value, unsigned eventNumber, const RawEventType& eventType,
        //         const STI::Engine::StackTrace& trace, const STI::Engine::RawEventGroup& group) 
        //         {
        //             return new STI::Engine::RawEvent(eventTarget, time, MixedValuePy(value), eventNumber, eventType, trace, group);
        //         }), 
        //         py::arg("eventTarget"), py::arg("time"), py::arg("value"),
        //         py::arg("eventNumber"), py::arg("eventType"), py::arg("stackTrace"),py::arg("group") )
        // .def("__init__",
        //     [](STI::Engine::RawEvent& instance, const STI::Device::DeviceID& targetDeviceID,
        //         double time, unsigned short channel, const pybind11::object& value,
        //         const std::string& description, unsigned eventNumber, const RawEventType& eventType) 
        //     {
        //         new (&instance) STI::Engine::RawEvent(targetDeviceID, time, channel, MixedValuePy(value), description, eventNumber, eventType);
        //     })
        // .def(py::init<const STI::Device::DeviceID&, double, unsigned short, 
        //             const STI::Utils::MixedValue&, const std::string&, unsigned, const RawEventType&>(), 
        //                 py::arg("targetDeviceID"), py::arg("time"), py::arg("channel"), py::arg("value"), 
        //                 py::arg("description"), py::arg("eventNumber"), py::arg("eventType") )
        .def("time", &STI::Engine::RawEvent::time)
        .def("channel", &STI::Engine::RawEvent::channel)
        //.def("value", &STI::Engine::RawEvent::value)
        .def("value",
            [](const STI::Engine::RawEvent& self) {
                MixedValuePy pyval(self.value());
                // return pyval.getValue_py();
                return pyval;
            })
        .def("description", &STI::Engine::RawEvent::description)
        .def("type", &STI::Engine::RawEvent::type)
        .def("target", &STI::Engine::RawEvent::target)
        .def("getGroupName", &STI::Engine::RawEvent::getGroupName)
        .def("trace", &STI::Engine::RawEvent::getStackTrace)
        // .def("compressedTrace", &STI::Engine::RawEvent::getCompressedStackTrace)
        .def("compressedTrace",
            [](const STI::Engine::RawEvent& self) {
                return self.getCompressedStackTrace();
            })
        .def("getEventGraphPath", &STI::Engine::RawEvent::getEventGraphPath)
        .def("isMeasurementEvent", &STI::Engine::RawEvent::isMeasurementEvent)
        .def("printTime", 
            [](const STI::Engine::RawEvent& self) {
                return STI::Utils::printTimeFormated(self.time());
            })
        .def("__repr__",
            [](const STI::Engine::RawEvent& self) {
                return self.print();
            })
        .def("__eq__",  // operator ==
            [](const STI::Engine::RawEvent& self, const STI::Engine::RawEvent& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const STI::Engine::RawEvent& self, const STI::Engine::RawEvent& other) {
                return self < other;
            })
            
        ;

}

