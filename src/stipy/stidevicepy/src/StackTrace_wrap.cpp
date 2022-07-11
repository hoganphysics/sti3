
// #include <sti/engine/StackTrace.h>
#include "StackTracePy.h"

#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using STI::Python::StackTracePy;
using STI::Python::StackFramePy;

namespace py = pybind11;

void init_StackTrace(py::module& m)
{

    py::class_<StackFramePy>(m, "StackFrame")
        .def(py::init<const std::string&, unsigned, const std::string&>(), py::arg("file"), py::arg("line"), py::arg("func"))
        .def_readonly("file", &StackFramePy::file)
        .def_readonly("line", &StackFramePy::line)
        .def_readonly("func", &StackFramePy::func)
        .def("__repr__",
            [](const StackFramePy& self) {
                std::stringstream s;
                s << "(file=" << self.file << ", line=" << self.line << ", func=" << self.func << ")";
                return s.str();
            })
        ;


    py::class_<StackTracePy>(m, "StackTrace")
        .def(py::init<>())
        .def("appendFrame", py::overload_cast<const std::string&, unsigned, const std::string&>(&StackTracePy::appendFrame), 
                py::arg("file"), py::arg("line"), py::arg("func"))
        .def("appendFrame", py::overload_cast<const StackFramePy&>(&StackTracePy::appendFrame), 
                py::arg("stackFrame"))
        .def("getFrames", py::overload_cast<>(&StackTracePy::getFrames, py::const_))
        // .def("__repr__",
        //     [](const StackTrace& self) {
        //         return self.print();
        //     })
        ;

}
