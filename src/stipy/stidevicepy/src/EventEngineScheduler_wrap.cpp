
#include "EventEngineSchedulerPy.h"

#include <sti/engine/ShotID.h>
#include <sti/engine/ShotConfig.h>

#include <sti/engine/RawEvent.h>
#include "LocalShot.h"

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
using STI::Engine::LocalShot;
using STI::Engine::RawEvent;
// using STI::Python::LocalShotPy;


void init_EventEngineScheduler(py::module& m)
{

    py::class_<LocalShot, std::shared_ptr<LocalShot>>(m, "LocalShot")
        // .def(py::init<>())
        .def(py::init<const ShotConfig&, const std::shared_ptr<STI::Engine::RawEventGroup>&>(), py::arg("shotConfig"), py::arg("baseGroup"))
        .def("getShotConfig", &LocalShot::getShotConfig)
        .def("addEvent",
            [](LocalShot& self, RawEvent& evt) {
                std::shared_ptr<STI::Engine::RawEventGroup> baseGroup;
                self.getBaseEventGroup(baseGroup);
                if (baseGroup != 0) {
                    baseGroup->addEvent(evt);
                }
            })
        .def("getBaseEventGroup",
            [](LocalShot& self) {
                std::shared_ptr<STI::Engine::RawEventGroup> baseGroup;
                self.getBaseEventGroup(baseGroup);
                return baseGroup;
            })
        .def("__repr__",
            [](const LocalShot& self) {
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
