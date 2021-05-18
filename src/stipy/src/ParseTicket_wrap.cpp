
#include "ParseTicket.h"
#include "EngineParsingMessage.h"
#include "RawEvent.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


namespace py = pybind11;


void init_ParseTicket(py::module& m) 
{

    py::class_<STI::Engine::EngineParsingMessage>(m, "EngineParsingMessage")
        .def("getMessage", &STI::Engine::EngineParsingMessage::getMessage)
        .def("getEvents", &STI::Engine::EngineParsingMessage::getEvents)
        .def("__repr__",
            [](const STI::Engine::EngineParsingMessage& message) {
                return "<" + message.getName() + "|" + message.getMessage() + ">";
            })
        ;

    py::class_<STI::Python::ParseTicket, std::shared_ptr<STI::Python::ParseTicket>>(m, "ParseTicket")

        .def("wait", &STI::Python::ParseTicket::wait)
        .def("cancel", &STI::Python::ParseTicket::cancel)
        .def("getMessages", &STI::Python::ParseTicket::getMessages)
        .def("getEvents", &STI::Python::ParseTicket::getEvents)
        ;

}
