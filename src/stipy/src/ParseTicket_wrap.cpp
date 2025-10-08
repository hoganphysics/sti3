#include "PyParseTicket.h"

#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/RawEvent.h>

#include <sti/engine/RawEventGroup.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/cast.h>

#include <sstream>

using STI::Python::PyParseTicket;

using STI::Engine::Ticket;

namespace py = pybind11;


void init_ParseTicket(py::module& m) 
{

    py::enum_<Ticket::TicketStatus>(m, "TicketStatus")
        .value("Running", Ticket::TicketStatus::Running)
        .value("Complete", Ticket::TicketStatus::Complete)
        .value("Canceled", Ticket::TicketStatus::Canceled)
        .value("NotFound", Ticket::TicketStatus::NotFound)
        .value("Deferred", Ticket::TicketStatus::Deferred)
        ;

    // //ParsingMessageType { Error, Warning, Information };
    // py::enum_<STI::Engine::ParsingMessageType>(m, "ParsingMessageType")
    //     .value("Error", STI::Engine::ParsingMessageType::Error)
    //     .value("Warning", STI::Engine::ParsingMessageType::Warning)
    //     .value("Information", STI::Engine::ParsingMessageType::Information)
    //     ;

    // py::class_<STI::Engine::EngineParsingMessage>(m, "EngineParsingMessage")
    //     .def("getType", &STI::Engine::EngineParsingMessage::getType)
    //     .def("getIDCode", &STI::Engine::EngineParsingMessage::getIDCode)
    //     .def("getSourceID", &STI::Engine::EngineParsingMessage::getSourceID)
    //     .def("getName", &STI::Engine::EngineParsingMessage::getName)
    //     .def("getMessage", &STI::Engine::EngineParsingMessage::getMessage)
    //     .def("events", &STI::Engine::EngineParsingMessage::getEvents)
    //     .def("addEvent", &STI::Engine::EngineParsingMessage::addEvent)
    //     .def("appendMessage", &STI::Engine::EngineParsingMessage::appendMessage)
    //     .def("__repr__",
    //         [](const STI::Engine::EngineParsingMessage& message) {
    //             return "<" + message.getName() + "|" + message.getMessage() + ">";
    //         })
    //     ;


    py::class_<STI::Python::PyParseTicket, std::shared_ptr<STI::Python::PyParseTicket>>(m, "ParseTicket")

        .def("wait", py::overload_cast<>(&STI::Python::PyParseTicket::wait, py::const_))
        .def("wait", py::overload_cast<const std::function<bool()>&>(&STI::Python::PyParseTicket::wait, py::const_))
        .def("cancel", &STI::Python::PyParseTicket::cancel)
        .def("defer", &STI::Python::PyParseTicket::defer)
        .def("messages", &STI::Python::PyParseTicket::getMessages)
        .def("rootgroup", &STI::Python::PyParseTicket::getEvents)
        .def("status", &STI::Python::PyParseTicket::getStatus)
        .def("getParseResult", &STI::Python::PyParseTicket::getParseResult)
        .def("getParseID", &STI::Python::PyParseTicket::getParseID)
        .def("getEngineID", &STI::Python::PyParseTicket::getEngineID)
        .def("__repr__",
            [](const PyParseTicket& self) {
                std::stringstream s;
                // <ParseTicket | Complete | pid:user@machine#2022_05_12>
                s << "<ParseTicket | " << PyParseTicket::statusToString(self.getStatus()) << " | " << self.getParseID().print() << ">";
                return s.str();
            })
        ;

}
