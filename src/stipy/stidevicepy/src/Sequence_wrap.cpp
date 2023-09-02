#include <sti/engine/ParsedVar.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/SequenceID.h>

#include <string>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>

namespace py = pybind11;

using STI::Engine::Sequence;
using STI::Engine::SequenceID;
using STI::Engine::SequenceIndex;
using STI::Engine::SequenceEntryID;
using STI::Engine::SequenceEntry;
using STI::Engine::SequenceType;
using STI::Engine::ParsedVar;


void init_Sequence(py::module& m)
{
    py::class_<SequenceIndex>(m, "SequenceIndex")
        .def(py::init<>())
        .def(py::init<int, int>())
        .def_readwrite("index", &SequenceIndex::index)
        .def_readwrite("repeat", &SequenceIndex::repeat)
        .def("__repr__",
            [](const SequenceIndex& self) {
                return self.print();
            })
        .def("__eq__",  // operator ==
            [](const SequenceIndex& self, const SequenceIndex& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const SequenceIndex& self, const SequenceIndex& other) {
                return self < other;
            })
        ;

    py::class_<SequenceID>(m, "SequenceID")
        .def(py::init<>())
        .def_readonly("timestamp", &SequenceID::timestamp)
        .def_readwrite("jobSourceID", &SequenceID::jobSourceID)
        .def("__repr__",
            [](const SequenceID& self) {
                return self.print();
            })
        .def("__eq__",  // operator ==
            [](const SequenceID& self, const SequenceID& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const SequenceID& self, const SequenceID& other) {
                return self < other;
            })
        ;

    py::class_<SequenceEntryID>(m, "SequenceEntryID")
        .def(py::init<>())
        .def_readwrite("seqID", &SequenceEntryID::seqID)
        .def_readwrite("seqIndex", &SequenceEntryID::seqIndex)
        .def("__repr__",
            [](const SequenceEntryID& self) {
                return self.seqID.print() + "#" + self.seqIndex.print();
            })
        .def("__eq__",  // operator ==
            [](const SequenceEntryID& self, const SequenceEntryID& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const SequenceEntryID& self, const SequenceEntryID& other) {
                return self < other;
            })
        ;

    py::class_<SequenceEntry>(m, "SequenceEntry")
        .def(py::init<>())
        .def_readwrite("index", &SequenceEntry::index)
        .def_readwrite("overwritten", &SequenceEntry::overwritten)
        .def("__repr__",
            [](const SequenceEntry& self) {
                return self.print();
            })
        .def("__eq__",  // operator ==
            [](const SequenceEntry& self, const SequenceEntry& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const SequenceEntry& self, const SequenceEntry& other) {
                return self < other;
            })
        ;
    
    //SequenceType { Open, Closed };
    py::enum_<SequenceType>(m, "SequenceType")
        .value("Open", SequenceType::Open)
        .value("Closed", SequenceType::Closed)
        ;
        //.export_values();
    

    py::class_<Sequence, std::shared_ptr<Sequence>>(m, "Sequence")
        .def(py::init<>())
        .def(py::init<const SequenceType&>())
        .def_readwrite("repeats", &Sequence::repeats)
        .def_readwrite("type", &Sequence::type)
        .def_readwrite("sequenceTable", &Sequence::sequenceTable)
        .def("addEntry", py::overload_cast<const SequenceEntry&>(&Sequence::addEntry), py::arg("entry"))
        .def("addEntry", py::overload_cast<int, const std::set<ParsedVar>&>(&Sequence::addEntry), py::arg("index"), py::arg("overwritten"))
        .def("append", py::overload_cast<const std::set<ParsedVar>&>(&Sequence::append), py::arg("overwritten"))
        ;


}

