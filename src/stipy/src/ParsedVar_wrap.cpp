

#include "ParsedVar.h"
#include "StackTraceData.h"
#include "RawStackTrace.h"

#include <vector>
#include <memory>
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


namespace py = pybind11;

// using STI::Python::ParsedVarPy;
using STI::Engine::ParsedVar;
using STI::Engine::RawStackTrace;


void init_ParsedVar(py::module& m) 
{

//    m.def("add", [](int a, int b) { return a + b; });

    py::class_<ParsedVar>(m, "ParsedVar")

        
        .def_readonly("name", &ParsedVar::name)
        .def_readonly("fullGroupName", &ParsedVar::fullGroupName)
        .def_readonly("value", &ParsedVar::value)
        .def("trace",
            [](const ParsedVar& self) {
                if (self.stackTraceData != 0) {
                    return self.stackTraceData->getStackTrace(self.trace);
                }
                RawStackTrace emptyTrace;
                return emptyTrace;
            })
        .def("isBound", &ParsedVar::isBound)
        .def("__repr__",
            [](const ParsedVar& self) {
                std::stringstream s;
                s << "ParsedVar(" << self.name << " = " << self.value.print() << ")";
                return s.str();
            })
        ;

}

