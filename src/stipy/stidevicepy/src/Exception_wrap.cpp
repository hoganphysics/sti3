#include <sti/engine/STI_Exception.h>

#include <sti/engine/EventParsingException.h>
#include <sti/engine/EventConflictException.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/cast.h>

#include <sstream>

using STI::Engine::STI_Exception;
using STI::Engine::EventParsingException;
using STI::Engine::EventConflictException;

namespace py = pybind11;


void init_Exception(py::module& m) 
{
    py::class_<STI_Exception>(m, "STI_Exception")
        .def(py::init<const std::string&>(), py::arg("message") )
        .def("printMessage", &STI_Exception::printMessage)
        .def("__repr__",
            [](const STI_Exception& self) {
                return "<STI_Exception: " + self.printMessage() + ">";
            })
        ;

    py::class_<EventParsingException, STI_Exception>(m, "EventParsingException")
        .def(py::init<const STI::Engine::RawEvent&, const std::string&>(), py::arg("evt"), py::arg("message") )
        .def("getEvent", &EventParsingException::getEvent)
        .def("__repr__",
            [](const EventParsingException& self) {
                return "<EventParsingException: " + self.printMessage() + ">";
            })
        ;
    
    py::register_exception<EventParsingException>(m, "EventParsingException");

    py::class_<EventConflictException, STI_Exception>(m, "EventConflictException")
        .def(py::init<const STI::Engine::RawEvent&, const std::string&>(), py::arg("evt"), py::arg("message") )
        .def(py::init<const STI::Engine::RawEvent&, const STI::Engine::RawEvent&, const std::string&>(), 
            py::arg("event1"), py::arg("event2"), py::arg("message") )
        .def("lastTime", &EventConflictException::lastTime)
        .def("getEvent1", &EventConflictException::getEvent1)
        .def("getEvent2", &EventConflictException::getEvent2)
        .def("__repr__",
            [](const EventConflictException& self) {
                return "<EventConflictException: " + self.printMessage() + ">";
            })
        ;

    py::register_exception<EventConflictException>(m, "EventConflictException");

}
