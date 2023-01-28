#include <sti/engine/ParseResult.h>

#include <sti/engine/ParsedDependencyTree.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/StackTraceResult.h>
#include <sti/engine/StackTraceData.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <sstream>
#include <memory>

using STI::Engine::ParseResult;

namespace py = pybind11;


void init_ParseResult(py::module& m)
{
    py::class_<ParseResult, std::shared_ptr<ParseResult>>(m, "ParseResult")
        .def_readonly("pid", &ParseResult::pid)
        .def_readonly("baseEventGroup", &ParseResult::baseEventGroup)
        .def_readonly("parsedDevices", &ParseResult::parsedDevices)
        .def_readonly("messages", &ParseResult::messages)
        .def_readonly("stackTraceResult", &ParseResult::stackTraceResult)       
        
        .def("__repr__",
            [](const ParseResult& self) {
                std::stringstream s;
                // <ParseResult | pid:user@machine#2022_05_12>
                s << "<ParseResult | " << self.pid.print() << ">";
                return s.str();
            })
        ;

}
