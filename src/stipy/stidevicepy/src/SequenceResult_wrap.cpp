#include <sti/engine/ShotResult.h>

#include <sti/engine/SequenceResult.h>


#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>


#include <sstream>

using STI::Engine::SequenceResult;


namespace py = pybind11;


void init_SequenceResult(py::module& m)
{
    py::class_<SequenceResult, std::shared_ptr<SequenceResult>>(m, "SequenceResult")
        .def_readonly("seqid", &SequenceResult::seqid)
        .def_readonly("sequence", &SequenceResult::sequence)
        .def_readonly("status", &SequenceResult::status)
        .def_readonly("shots", &SequenceResult::shots)
        .def("__eq__",  // operator ==
            [](const SequenceResult& self, const SequenceResult& other) {
                return self.seqid == other.seqid;
            })
        .def("__lt__",  // operator <
            [](const SequenceResult& self, const SequenceResult& other) {
                return self.seqid < other.seqid;
            })
        .def("__repr__",
            [](const SequenceResult& self) {
                std::stringstream s;
                s << "<SequenceResult | " << self.seqid.print() << ">";
                return s.str();
            })
        ;

}
