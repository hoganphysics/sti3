
#include "EventEngineSchedulerPy.h"
#include "ParseID.h"
#include "ShotID.h"
#include "ShotConfig.h"
#include "RawEvent.h"
#include "LocalShotPy.h"

#include "EngineJobID.h"

#include <string>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>


namespace py = pybind11;

using STI::Python::EventEngineSchedulerPy;
using STI::Engine::ParseID;
using STI::Engine::TimeStamp;
using STI::Engine::ShotConfig;
using STI::Engine::ShotType;
using STI::Python::LocalShotPy;


void init_EventEngineScheduler(py::module& m)
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

    py::enum_<ShotType>(m, "ShotType")
        .value("Single", ShotType::Single)
        .value("Sequence", ShotType::Sequence)
        .value("SingleUndocumented", ShotType::SingleUndocumented)
        .export_values();


    py::class_<STI::Engine::ShotConfig>(m, "ShotConfig")
        .def(py::init<>())
        // .def("type", [](const ShotConfig& self) {
        //         return printShotType(self.shotType);
        //     })
        .def_readonly("shotType", &ShotConfig::shotType)
        .def_readonly("jobSourceID", &ShotConfig::jobSourceID)
        .def_readonly("targetEnginePool", &ShotConfig::targetEnginePool)
        .def_readonly("file", &ShotConfig::file)
        .def_readonly("comment", &ShotConfig::comment)

        .def("__repr__",
            [](const ShotConfig& self) {
                return self.print();
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



    py::class_<STI::Python::LocalShotPy, std::shared_ptr<LocalShotPy>>(m, "LocalShot")
        .def(py::init<const ShotConfig&>(), py::arg("shotConfig"))
        .def("getShotConfig", &LocalShotPy::getShotConfig)
        .def("getEvents", py::overload_cast<>(&LocalShotPy::getEvents))
        .def("addEvent", &LocalShotPy::addEvent, py::arg("event"))

        .def("__repr__",
            [](const LocalShotPy& self) {
                return self.getShotConfig().print();
            })
        ;


    py::class_<EventEngineSchedulerPy, std::shared_ptr<EventEngineSchedulerPy>>(m, "EventEngineScheduler")
        .def("parse", &EventEngineSchedulerPy::parse, py::arg("shot"))
        .def("play", &EventEngineSchedulerPy::play, py::arg("parseID"), py::arg("source"))
        .def("getStatus", py::overload_cast<const STI::Engine::ParseID&>(&EventEngineSchedulerPy::getStatus), py::arg("parseID"))
        .def("getStatus", py::overload_cast<const STI::Engine::ShotID&>(&EventEngineSchedulerPy::getStatus), py::arg("shotID"))
        .def("cancelAll", &EventEngineSchedulerPy::cancelAll)
        .def("getQueuedJobs", &EventEngineSchedulerPy::getQueuedJobs)
        .def("getRunningJobs", &EventEngineSchedulerPy::getRunningJobs)
        .def("getCompletedJobs", &EventEngineSchedulerPy::getCompletedJobs)
        ;


}
