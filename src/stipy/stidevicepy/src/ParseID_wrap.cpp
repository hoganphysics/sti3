
#include <sti/engine/ParseID.h>
#include <sti/engine/ShotConfig.h>


#include <pybind11/pybind11.h>

namespace py = pybind11;

using STI::Engine::TimeStamp;
using STI::Engine::ParseID;

void init_ParseID(py::module& m)
{

    py::class_<STI::Engine::TimeStamp>(m, "TimeStamp")
        .def(py::init<>())
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
        .def("date_YYYY_MM_DD", &TimeStamp::date_YYYY_MM_DD)
        .def("time", &TimeStamp::time)
        .def("time_hh_mm_ss", &TimeStamp::time_hh_mm_ss)
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
        .def_readonly("parseTimestamp", &ParseID::parseTimestamp)
        .def_readonly("shotConfig", &ParseID::shotConfig)
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
