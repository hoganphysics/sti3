

#include "ParsedVarPy.h"

#include <vector>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

using STI::Python::ParsedVarPy;


void init_ParsedVarPy(py::module& m) 
{

//    m.def("add", [](int a, int b) { return a + b; });

    py::class_<ParsedVarPy>(m, "ParsedVar")

        .def("name", &ParsedVarPy::name)
        .def("value", &ParsedVarPy::value)
        .def("trace", &ParsedVarPy::trace)
        .def("scope", &ParsedVarPy::scope)
        .def("__repr__",
            [](const ParsedVarPy& self) {
                std::stringstream s;
                s << "ParsedVar(" << self.name() << " = " << self.value() << ")";
                return s.str();
            })
        ;

}

