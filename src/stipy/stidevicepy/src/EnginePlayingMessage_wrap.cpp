#include <sti/engine/EnginePlayingMessage.h>
#include <sti/engine/EnginePlayingMessageCount.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/cast.h>

#include <sstream>


namespace py = pybind11;


void init_EnginePlayingMessage(py::module& m)
{
    py::enum_<STI::Engine::PlayingMessageType>(m, "PlayingMessageType")
        .value("PlayingError", STI::Engine::PlayingMessageType::Error)
        .value("PlayingWarning", STI::Engine::PlayingMessageType::Warning)
        .value("PlayingInformation", STI::Engine::PlayingMessageType::Information)
        ;

    py::class_<STI::Engine::EnginePlayingMessage>(m, "EnginePlayingMessage")
        .def(py::init<>())
        .def(py::init<const STI::Engine::PlayingMessageType&, unsigned, const std::string&>(),
            py::arg("type"), py::arg("id"), py::arg("name"))
        .def(py::init<const STI::Device::DeviceID&, const STI::Engine::PlayingMessageType&, unsigned, const std::string&>(),
            py::arg("source"), py::arg("type"), py::arg("id"), py::arg("name"))
        .def("getType", &STI::Engine::EnginePlayingMessage::getType)
        .def("getIDCode", &STI::Engine::EnginePlayingMessage::getIDCode)
        .def("getSourceID", &STI::Engine::EnginePlayingMessage::getSourceID)
        .def("getName", &STI::Engine::EnginePlayingMessage::getName)
        .def("getMessage", &STI::Engine::EnginePlayingMessage::getMessage)
        .def("appendMessage", &STI::Engine::EnginePlayingMessage::appendMessage)
        .def("__repr__",
            [](const STI::Engine::EnginePlayingMessage& message) {
                return "<" + message.getName() + "|" + message.getMessage() + ">";
            })
        ;

    py::class_<STI::Engine::EnginePlayingMessageCount>(m, "EnginePlayingMessageCount")
        .def(py::init<>())
        .def(py::init<const std::vector<STI::Engine::EnginePlayingMessage>&>(), py::arg("messages"))
        .def_readonly("errorCount", &STI::Engine::EnginePlayingMessageCount::errorCount)
        .def_readonly("warningCount", &STI::Engine::EnginePlayingMessageCount::warningCount)
        .def_readonly("infoCount", &STI::Engine::EnginePlayingMessageCount::infoCount)
        .def("__repr__",
            [](const STI::Engine::EnginePlayingMessageCount& count) {
                std::ostringstream oss;
                oss << "<EnginePlayingMessageCount | "
                    << "Errors: " << count.errorCount
                    << ", Warnings: " << count.warningCount
                    << ", Info: " << count.infoCount
                    << ">";
                return oss.str();
            })
        ;
}
