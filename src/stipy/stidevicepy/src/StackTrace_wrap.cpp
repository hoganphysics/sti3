#include "StackTrace.h"

#include <sti/engine/CompressedStackTrace.h>
#include <sti/engine/StackTraceData.h>
#include <sti/engine/StackTraceResult.h>
#include <sti/utils/FileID.h>
#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using STI::Engine::StackFrame;
using STI::Engine::StackTrace;
using STI::Engine::StackTraceData;
using STI::Engine::StackTraceResult;
using STI::Utils::FileID;
using STI::Engine::CompressedStackFrame;
using STI::Engine::CompressedStackTrace;

namespace py = pybind11;

void init_StackTrace(py::module& m)
{
    py::class_<CompressedStackTrace>(m, "CompressedStackTrace")
        .def(py::init<>())
        .def("appendFrame", py::overload_cast<unsigned, unsigned, unsigned>(&CompressedStackTrace::appendFrame), 
             py::arg("file"), py::arg("line"), py::arg("func"))
        .def("appendFrame", py::overload_cast<const CompressedStackFrame&>(&CompressedStackTrace::appendFrame), 
             py::arg("frame"))
        .def("getFrames", &CompressedStackTrace::getFrames)
        .def("__repr__",
            [](const CompressedStackTrace& self) {
                std::stringstream s;
                s << "CompressedStackTrace(frames=" << self.getFrames().size() << ")";
                return s.str();
            })
        ;
    
    py::class_<CompressedStackFrame>(m, "CompressedStackFrame")
        .def(py::init<>())
        .def(py::init<unsigned, unsigned, unsigned>(), 
             py::arg("file"), py::arg("line"), py::arg("func"))
        .def_readonly("file", &CompressedStackFrame::file)
        .def_readonly("line", &CompressedStackFrame::line)
        .def_readonly("func", &CompressedStackFrame::func)
        .def("__repr__",
            [](const CompressedStackFrame& self) {
                std::stringstream s;
                s << "(file=" << self.file << ", line=" << self.line << ", func=" << self.func << ")";
                return s.str();
            })
        ;

    py::class_<StackFrame>(m, "StackFrame")
        .def(py::init<const std::string&, unsigned, const std::string&>(), py::arg("file"), py::arg("line"), py::arg("func"))
        .def_readonly("file", &StackFrame::file)
        .def_readonly("line", &StackFrame::line)
        .def_readonly("func", &StackFrame::func)
        .def("__repr__",
            [](const StackFrame& self) {
                std::stringstream s;
                s << "(file=" << self.file << ", line=" << self.line << ", func=" << self.func << ")";
                return s.str();
            })
        ;

    // RawStackFrame -> StackFrame
    // StackFrame -> CompressedStackFrame, CompressedStackTrace
    py::class_<StackTrace>(m, "StackTrace")
        .def(py::init<>())
        .def("appendFrame", py::overload_cast<const std::string&, unsigned, const std::string&>(&StackTrace::appendFrame), 
                py::arg("file"), py::arg("line"), py::arg("func"))
        .def("appendFrame", py::overload_cast<const StackFrame&>(&StackTrace::appendFrame), 
                py::arg("stackFrame"))
        .def("getFrames", py::overload_cast<>(&StackTrace::getFrames, py::const_))
        // .def("__repr__",
        //     [](const StackTrace& self) {
        //         return self.print();
        //     })
        .def("__repr__",
            [](const StackTrace& self) {
                std::stringstream s;
                s << "StackTrace(frames=" << self.getFrames().size() << ")";
                return s.str();
            })
        ;

    py::class_<StackTraceData, std::shared_ptr<StackTraceData>>(m, "StackTraceData")
        .def("getFunctionNames", &StackTraceData::getFunctionNames)
        .def("getTimingFiles", &StackTraceData::getTimingFiles)
        .def("getStackTrace", &StackTraceData::getStackTrace)
        ;
    
    py::class_<StackTraceResult, std::shared_ptr<StackTraceResult>>(m, "StackTraceResult")
        .def_readonly("pid", &StackTraceResult::pid)
        .def_readonly("stackTraceData", &StackTraceResult::stackTraceData)
        // .def("__repr__",
        //     [](const StackTrace& self) {
        //         return self.print();
        //     })
        ;
        
}
