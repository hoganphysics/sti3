#include <sti/engine/EventEngineScheduler.h>

#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EventEngineJobList.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotConfig.h>

#include "LocalShot.h"

#include <string>
#include <memory>
#include <sstream>
#include <set>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

using STI::Engine::ParseID;
using STI::Utils::TimeStamp;
using STI::Engine::ShotConfig;
using STI::Engine::LocalShot;
using STI::Engine::RawEvent;
using STI::Engine::EventEngineJobList;
using STI::Engine::EngineJobStatus;
using STI::Engine::ParseJobStatus;
using STI::Engine::EventEngineScheduler;
using STI::Engine::Shot;
using STI::Engine::SequenceEntryID;
using STI::Engine::PlayJobStatus;
using STI::Engine::AddSequenceStatus;
using STI::Engine::Sequence;
using STI::Engine::EngineJobSourceID;
using STI::Engine::EventEngineJobType;
using STI::Engine::EngineJobID;


void init_EventEngineScheduler(py::module& m)
{

    py::class_<LocalShot, std::shared_ptr<LocalShot>>(m, "LocalShot")
        .def(py::init<const ShotConfig&, const std::shared_ptr<STI::Engine::RawEventGroup>&>(), py::arg("shotConfig"), py::arg("baseGroup"))
        .def("getShotConfig", &LocalShot::getShotConfig)
        .def("addEvent",
            [](LocalShot& self, RawEvent& evt) {
                std::shared_ptr<STI::Engine::RawEventGroup> rootGroup;
                self.getRootEventGroup(rootGroup);
                if (rootGroup != 0) {
                    rootGroup->addEvent(evt);
                }
            })
        .def("getRootEventGroup",
            [](LocalShot& self) {
                std::shared_ptr<STI::Engine::RawEventGroup> rootGroup;
                self.getRootEventGroup(rootGroup);
                return rootGroup;
            })
        .def("__repr__",
            [](const LocalShot& self) {
                return self.getShotConfig().print();
            })
        ;
    
    //EngineJobStatus { New, Running, Completed, Canceled, NotFound, Archived, Deferred };
    py::enum_<EngineJobStatus>(m, "EngineJobStatus")
        .value("New", EngineJobStatus::New)
        .value("Running", EngineJobStatus::Running)
        .value("Completed", EngineJobStatus::Completed)
        .value("Canceled", EngineJobStatus::Canceled)
        .value("NotFound", EngineJobStatus::NotFound)
        .value("Archived", EngineJobStatus::Archived)
        .value("Deferred", EngineJobStatus::Deferred)
        ;

    py::enum_<EventEngineJobList>(m, "EventEngineJobList")
        .value("Queued", EventEngineJobList::Queued)
        .value("Running", EventEngineJobList::Running)
        .value("Completed", EventEngineJobList::Completed)
        .value("Archived", EventEngineJobList::Archived)
        ;

    py::enum_<EventEngineJobType>(m, "EventEngineJobType")
        .value("Parse", EventEngineJobType::Parse)
        .value("Play", EventEngineJobType::Play)
        ;

    py::class_<STI::Engine::EngineJobID>(m, "EngineJobID")
        .def(py::init<>())
        .def_readonly("type", &EngineJobID::type)
        .def_readonly("pid", &EngineJobID::pid)
        .def_readonly("sid", &EngineJobID::sid)
        .def_readonly("runTime", &EngineJobID::runTime)
        .def("__repr__",
            [](const EngineJobID& self) {
                std::stringstream buffer;
                buffer << "<EngineJobID | Type: ";
                if (self.type == EventEngineJobType::Parse) {
                    buffer << "Parse" << " | ";
                    buffer << self.pid.print();
                }
                if (self.type == EventEngineJobType::Play) {
                    buffer << "Play" << " | ";
                    buffer << self.sid.print();
                }
                buffer << ">";
                return buffer.str();
            })
        .def("__eq__",  // operator ==
            [](const EngineJobID& self, const EngineJobID& other) {
                return self == other;
            })
        .def("__lt__",  // operator <
            [](const EngineJobID& self, const EngineJobID& other) {
                return self < other;
            })
        ;

    py::class_<STI::Engine::ParseJobStatus>(m, "ParseJobStatus")
        .def(py::init<>())
        .def_readonly("status", &ParseJobStatus::status)
        .def_readonly("pid", &ParseJobStatus::pid)
        .def("__repr__",
            [](const STI::Engine::ParseJobStatus& self) {
                return self.pid.print();
            })
        .def("__eq__",  // operator ==
            [](const ParseJobStatus& self, const ParseJobStatus& other) {
                return self.pid == other.pid && self.status == other.status;
            })
        .def("__lt__",  // operator <
            [](const ParseJobStatus& self, const ParseJobStatus& other) {
                if (self.pid == other.pid) {
                    return self.status < other.status;
                }
                return self.pid < other.pid;
            })
        ;


    py::class_<STI::Engine::PlayJobStatus>(m, "PlayJobStatus")
        .def(py::init<>())
        .def_readonly("status", &PlayJobStatus::status)
        .def_readonly("sid", &PlayJobStatus::sid)
        .def("__repr__",
            [](const STI::Engine::PlayJobStatus& self) {
                return self.sid.print();
            })
        .def("__eq__",  // operator ==
            [](const PlayJobStatus& self, const PlayJobStatus& other) {
                return self.sid == other.sid && self.status == other.status;
            })
        .def("__lt__",  // operator <
            [](const PlayJobStatus& self, const PlayJobStatus& other) {
                if (self.sid == other.sid) {
                    return self.status < other.status;
                }
                return self.sid < other.sid;
            })
        ;

    py::class_<STI::Engine::AddSequenceStatus>(m, "AddSequenceStatus")
        .def(py::init<>())
        .def_readonly("status", &AddSequenceStatus::status)
        .def_readonly("seqid", &AddSequenceStatus::seqid)
        .def("__repr__",
            [](const STI::Engine::AddSequenceStatus& self) {
                return self.seqid.print();
            })
        .def("__eq__",  // operator ==
            [](const AddSequenceStatus& self, const AddSequenceStatus& other) {
                return self.seqid == other.seqid && self.status == other.status;
            })
        .def("__lt__",  // operator <
            [](const AddSequenceStatus& self, const AddSequenceStatus& other) {
                if (self.seqid == other.seqid) {
                    return self.status < other.status;
                }
                return self.seqid < other.seqid;
            })
        ;


    py::class_<EventEngineScheduler, std::shared_ptr<EventEngineScheduler>>(m, "EventEngineScheduler")
        .def("parse", py::overload_cast<const std::shared_ptr<Shot>&>(&EventEngineScheduler::parse), py::arg("shot"))
        .def("parse", py::overload_cast<const std::shared_ptr<Shot>&, const SequenceEntryID&>(&EventEngineScheduler::parse), py::arg("shot"), py::arg("sequenceEntryID"))
        .def("play", &EventEngineScheduler::play, py::arg("parseID"), py::arg("source"))
        .def("addSequence", py::overload_cast<const std::shared_ptr<Sequence>&, const EngineJobSourceID&>(&EventEngineScheduler::addSequence), py::arg("sequence"), py::arg("source"))
        .def("getStatus", py::overload_cast<const STI::Engine::ParseID&>(&EventEngineScheduler::getStatus), py::arg("parseID"))
        .def("getStatus", py::overload_cast<const STI::Engine::ShotID&>(&EventEngineScheduler::getStatus), py::arg("shotID"))
        .def("cancelJob", &EventEngineScheduler::cancelJob)
        .def("cancelAll", &EventEngineScheduler::cancelAll)
        .def("jobIDs",
            [](EventEngineScheduler& self, const EventEngineJobList& jobListType) {

                std::set<EngineJobID> jobIDset = self.getJobIDs(jobListType);
                std::vector<EngineJobID> jobIDvec;

                for(auto& id : jobIDset) {
                    jobIDvec.push_back(id);
                }

                return jobIDvec;
            })
        ;

}
