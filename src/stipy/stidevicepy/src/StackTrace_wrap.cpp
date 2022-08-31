
// #include <sti/engine/StackTrace.h>
#include "RawStackTrace.h"

#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using STI::Engine::RawStackFrame;
using STI::Engine::RawStackTrace;


namespace py = pybind11;

void init_StackTrace(py::module& m)
{

    py::class_<RawStackFrame>(m, "StackFrame")
        .def(py::init<const std::string&, unsigned, const std::string&>(), py::arg("file"), py::arg("line"), py::arg("func"))
        .def_readonly("file", &RawStackFrame::file)
        .def_readonly("line", &RawStackFrame::line)
        .def_readonly("func", &RawStackFrame::func)
        .def("__repr__",
            [](const RawStackFrame& self) {
                std::stringstream s;
                s << "(file=" << self.file << ", line=" << self.line << ", func=" << self.func << ")";
                return s.str();
            })
        ;


    py::class_<RawStackTrace>(m, "StackTrace")
        .def(py::init<>())
        .def("appendFrame", py::overload_cast<const std::string&, unsigned, const std::string&>(&RawStackTrace::appendFrame), 
                py::arg("file"), py::arg("line"), py::arg("func"))
        .def("appendFrame", py::overload_cast<const RawStackFrame&>(&RawStackTrace::appendFrame), 
                py::arg("stackFrame"))
        .def("getFrames", py::overload_cast<>(&RawStackTrace::getFrames, py::const_))
        // .def("__repr__",
        //     [](const StackTrace& self) {
        //         return self.print();
        //     })
        ;

}
