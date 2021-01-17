
#include "RawEvent.h"

#include "DeviceID.h"
#include "MixedValue.h"
#include "MixedValuePy.h"

#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Engine::RawEventType;
using STI::Python::MixedValuePy;

void init_RawEvent(py::module& m) 
{

    //RawEventType { Play, Measurement, Waveform, Pause, Jump };

    py::enum_<RawEventType>(m, "RawEventType")
        .value("Play", RawEventType::Play)
        .value("Measurement", RawEventType::Measurement)
        .value("Waveform", RawEventType::Waveform)
        .value("Pause", RawEventType::Pause)
        .value("Jump", RawEventType::Jump)
        .export_values()
        ;


    py::class_<STI::Engine::RawEvent>(m, "RawEvent")
        .def(py::init<>())
        .def("__init__",
            [](STI::Engine::RawEvent& instance, const STI::Device::DeviceID& targetDeviceID,
                double time, unsigned short channel, const pybind11::object& value,
                const std::string& description, unsigned eventNumber, const RawEventType& eventType) 
            {
                new (&instance) STI::Engine::RawEvent(targetDeviceID, time, channel, MixedValuePy(value), description, eventNumber, eventType);
            })
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
                return pyval.getValue_py();
            })
        .def("description", &STI::Engine::RawEvent::description)
        .def("type", &STI::Engine::RawEvent::type)
        .def("targetDevice", &STI::Engine::RawEvent::targetDevice)
    //    .def("getStackTrace", &STI::Engine::RawEvent::getStackTrace)
        .def("getEventGraphPath", &STI::Engine::RawEvent::getEventGraphPath)
        .def("__repr__",
            [](const STI::Engine::RawEvent& self) {
                return self.print();
            })
        .def("__eq__",  // operator ==
            [](const STI::Engine::RawEvent& self, const STI::Engine::RawEvent& other) {
                return self == other;
            })
        ;

}

