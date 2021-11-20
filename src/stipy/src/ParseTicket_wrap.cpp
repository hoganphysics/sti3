
#include "PyParseTicket.h"
#include "EngineParsingMessage.h"
#include "RawEvent.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using STI::Engine::Ticket;

namespace py = pybind11;


void init_ParseTicket(py::module& m) 
{

    py::enum_<Ticket::TicketStatus>(m, "TicketStatus")
        .value("Running", Ticket::TicketStatus::Running)
        .value("Complete", Ticket::TicketStatus::Complete)
        .value("Canceled", Ticket::TicketStatus::Canceled)
        .value("NotFound", Ticket::TicketStatus::NotFound)
        .export_values();

    py::class_<STI::Engine::EngineParsingMessage>(m, "EngineParsingMessage")
        .def("getMessage", &STI::Engine::EngineParsingMessage::getMessage)
        .def("getEvents", &STI::Engine::EngineParsingMessage::getEvents)
        .def("__repr__",
            [](const STI::Engine::EngineParsingMessage& message) {
                return "<" + message.getName() + "|" + message.getMessage() + ">";
            })
        ;

    py::class_<STI::Python::PyParseTicket, std::shared_ptr<STI::Python::PyParseTicket>>(m, "ParseTicket")

        .def("wait", &STI::Python::PyParseTicket::wait)
        .def("cancel", &STI::Python::PyParseTicket::cancel)
        .def("getMessages", &STI::Python::PyParseTicket::getMessages)
        .def("getEvents", &STI::Python::PyParseTicket::getEvents)
        .def("getStatus", &STI::Python::PyParseTicket::getStatus)
        ;

}
