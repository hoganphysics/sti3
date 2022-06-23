
#include "EventEngineSchedulerPy.h"

#include <sti/engine/ShotID.h>
#include <sti/engine/ShotConfig.h>

#include <sti/engine/RawEvent.h>
#include "LocalShotPy.h"

#include <sti/engine/EngineJobID.h>

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

using STI::Python::LocalShotPy;


void init_EventEngineScheduler(py::module& m)
{




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
