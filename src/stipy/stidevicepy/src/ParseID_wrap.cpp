#include <sti/engine/ParseID.h>
#include <sti/engine/ShotConfig.h>

#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Utils::TimeStamp;
using STI::Engine::ParseID;
using STI::Engine::EngineJobSourceID;


void init_ParseID(py::module& m)
{

    py::class_<TimeStamp>(m, "TimeStamp")
        .def(py::init<>())
        .def(py::init<int, int, int, int, int, int, int, int, int>(),
            py::arg("year"), py::arg("month"), py::arg("day"),
            py::arg("hour"), py::arg("minute"), py::arg("sec"),
            py::arg("millis") = 0, py::arg("micros") = 0, py::arg("nanos"))
        .def("year", &TimeStamp::year)
        .def("month", &TimeStamp::month)
        .def("day", &TimeStamp::day)
        .def("hour", &TimeStamp::hour)
        .def("minute", &TimeStamp::minute)
        .def("sec", &TimeStamp::sec)
        .def("millis", &TimeStamp::millis)
        .def("micros", &TimeStamp::micros)
        .def("nanos", &TimeStamp::nanos)
        .def("date", &TimeStamp::date)
        .def("date_YYYY_MM_DD", py::overload_cast<>(&TimeStamp::date_YYYY_MM_DD, py::const_))
        .def("time", &TimeStamp::time)
        .def("time_hh_mm_ss", py::overload_cast<>(&TimeStamp::time_hh_mm_ss, py::const_))
        .def("time_hh_mm_ss_mmmuuunnn", &TimeStamp::time_hh_mm_ss_mmmuuunnn)
        .def("__repr__",
            [](const TimeStamp& self) {
                return self.print();
            })
        .def("__eq__",  // operator ==
            [](const TimeStamp& self, const TimeStamp& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const TimeStamp& self, const TimeStamp& other) {
                return self < other;
            })
        ;

    py::class_<STI::Engine::ParseID>(m, "ParseID")
        .def(py::init<>())
        .def(py::init(
            [](const STI::Engine::ParseID& parseID) 
            {
                return STI::Engine::ParseID(parseID.parseTimestamp, parseID.jobSourceID, parseID.sequenceEntryID);
            } ), py::arg("parseID"))
        .def(py::init<const TimeStamp&, const EngineJobSourceID&>(),
            py::arg("parseTimestamp"), py::arg("jobSourceID"))
        .def(py::init<const TimeStamp&, const EngineJobSourceID&, const STI::Engine::SequenceEntryID&>(),
            py::arg("parseTimestamp"), py::arg("jobSourceID"), py::arg("sequenceEntryID"))
        .def_readonly("parseTimestamp", &ParseID::parseTimestamp)
        // .def_readonly("shotConfig", &ParseID::shotConfig)
        .def_readonly("shotType", &ParseID::shotType)
        .def_readonly("jobSourceID", &ParseID::jobSourceID)
        .def_readonly("sequenceEntryID", &ParseID::sequenceEntryID)
        .def("__repr__",
            [](const ParseID& self) {
                return self.print();
            })
        .def("__eq__",  // operator ==
            [](const ParseID& self, const ParseID& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const ParseID& self, const ParseID& other) {
                return self < other;
            })
        ;

}
