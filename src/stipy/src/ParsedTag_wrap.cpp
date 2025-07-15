

#include <sti/engine/ParsedTag.h>
#include <sti/engine/StackTraceData.h>

#include "StackTrace.h"
#include "MixedValuePy.h"

#include <vector>
#include <memory>
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


namespace py = pybind11;

// using STI::Python::ParsedVarPy;
using STI::Engine::ParsedTag;
using STI::Engine::StackTrace;
using STI::Python::MixedValuePy;

void init_ParsedTag(py::module& m) 
{

    py::class_<ParsedTag>(m, "ParsedTag")
        .def_readonly("name", &ParsedTag::name)
        // .def_readonly("fullGroupName", &ParsedVar::fullGroupName)
        .def("trace",
            [](const ParsedTag& self) {
                if (self.stackTraceData != 0) {
                    return self.stackTraceData->getStackTrace(self.trace);
                }
                StackTrace emptyTrace;
                return emptyTrace;
            })
        .def("compressedTrace",
            [](const ParsedTag& self) {
                return self.trace;
            })
        .def("getGroupName", &ParsedTag::getGroupName)  
        .def("__repr__",
            [](const ParsedTag& self) {
                std::stringstream s;
                s << "tag(" << self.name << ")";
                return s.str();
            })
        ;

}

