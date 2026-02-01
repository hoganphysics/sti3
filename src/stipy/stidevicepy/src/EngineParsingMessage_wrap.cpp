#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/EngineParsingMessageCount.h>

#include <sti/engine/RawEvent.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/cast.h>

#include <sstream>


namespace py = pybind11;


void init_EngineParsingMessage(py::module& m) 
{
    py::enum_<STI::Engine::ParsingMessageType>(m, "ParsingMessageType")
        .value("ParsingError", STI::Engine::ParsingMessageType::Error)
        .value("ParsingWarning", STI::Engine::ParsingMessageType::Warning)
        .value("ParsingInformation", STI::Engine::ParsingMessageType::Information)
        ;

    py::class_<STI::Engine::EngineParsingMessage>(m, "EngineParsingMessage")
        .def(py::init<>() )
        .def(py::init<const STI::Device::DeviceID&, const STI::Engine::ParsingMessageType&, 
                      unsigned, const std::string&>(), py::arg("source"), py::arg("type"), py::arg("id"), py::arg("name") )
        .def("getType", &STI::Engine::EngineParsingMessage::getType)
        .def("getIDCode", &STI::Engine::EngineParsingMessage::getIDCode)
        .def("getSourceID", &STI::Engine::EngineParsingMessage::getSourceID)
        .def("getName", &STI::Engine::EngineParsingMessage::getName)
        .def("getMessage", &STI::Engine::EngineParsingMessage::getMessage)
        .def("events", &STI::Engine::EngineParsingMessage::getEvents)
        .def("addEvent", &STI::Engine::EngineParsingMessage::addEvent)
        .def("appendMessage", &STI::Engine::EngineParsingMessage::appendMessage)
        .def("__repr__",
            [](const STI::Engine::EngineParsingMessage& message) {
                return "<" + message.getName() + "|" + message.getMessage() + ">";
            })
        ;

    py::class_<STI::Engine::EngineParsingMessageCount>(m, "EngineParsingMessageCount")
        .def(py::init<>() )
        .def(py::init<const std::vector<STI::Engine::EngineParsingMessage>&>(), py::arg("messages"))
        .def_readonly("errorCount", &STI::Engine::EngineParsingMessageCount::errorCount)
        .def_readonly("warningCount", &STI::Engine::EngineParsingMessageCount::warningCount)
        .def_readonly("infoCount", &STI::Engine::EngineParsingMessageCount::infoCount)
        .def("__repr__",
            [](const STI::Engine::EngineParsingMessageCount& count) {
                std::ostringstream oss;
                oss << "<EngineParsingMessageCount | "
                    << "Errors: " << count.errorCount
                    << ", Warnings: " << count.warningCount
                    << ", Info: " << count.infoCount
                    << ">";
                return oss.str();
            })
        ;
}
