#include <sti/engine/EventEngineScheduler.h>

#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/EngineState.h>

#include <sti/engine/EventEngineJob.h>
#include <sti/engine/EventEngineJobList.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/ParsedDependencyTree.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/Shot.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotConfig.h>
#include <sti/device/DeviceIDIndexedGraph.h>

#include "LocalShot.h"
#include "EventEngineDependencyTree.h"

#include <string>
#include <memory>
#include <sstream>
#include <set>
#include <vector>

// #include <iostream>

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
using STI::Engine::EventEngineJob;
using STI::Engine::EngineState;
using STI::Utils::DependencyTree;
using STI::Device::DeviceID;

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

    // EngineState {Idle, Parsing, Parsed, PreparingPlay, PlayReady, WaitingForTrigger, Playing, Paused, Unknown, Missing, Error}

    py::enum_<EngineState>(m, "EngineState")
        .value("Idle", EngineState::Idle)
        .value("Parsing", EngineState::Parsing)
        .value("Parsed", EngineState::Parsed)
        .value("PreparingPlay", EngineState::PreparingPlay)
        .value("PlayReady", EngineState::PlayReady)
        .value("WaitingForTrigger", EngineState::WaitingForTrigger)
        .value("Playing", EngineState::Playing)
        .value("Paused", EngineState::Paused)
        .value("Unknown", EngineState::Unknown)
        .value("Missing", EngineState::Missing)
        .value("Error", EngineState::Error)
        ;

    py::enum_<EventEngineJobType>(m, "EventEngineJobType")
        .value("Parse", EventEngineJobType::Parse)
        .value("Play", EventEngineJobType::Play)
        .value("Sequence", EventEngineJobType::Sequence)
        ;

    py::class_<STI::Engine::EngineJobID>(m, "EngineJobID")
        .def(py::init<>())
        .def(py::init<const STI::Engine::ParseID&>(), py::arg("parseID"))
        .def(py::init<const STI::Engine::ShotID&>(), py::arg("shotID"))
        .def(py::init<const STI::Engine::SequenceID&>(), py::arg("sequenceID"))
        .def(py::init<STI::Engine::EventEngineJobType, STI::Engine::ParseID, STI::Engine::ShotID, STI::Engine::SequenceID, STI::Utils::TimeStamp>(),
            py::arg("type"), py::arg("pid"), py::arg("sid"), py::arg("seqid"), py::arg("runTime"))
        
        .def_readonly("type", &EngineJobID::type)
        .def_readonly("pid", &EngineJobID::pid)
        .def_readonly("sid", &EngineJobID::sid)
        .def_readonly("seqid", &EngineJobID::seqid)
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
                if (self.type == EventEngineJobType::Sequence) {
                    buffer << "Sequence" << " | ";
                    buffer << self.seqid.print();
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

    py::class_<EventEngineJob, std::shared_ptr<EventEngineJob>>(m, "EventEngineJob")
        .def("jobID", &EventEngineJob::getJobID)
        .def("jobOwner", &EventEngineJob::getJobOwner)
        .def("status", &EventEngineJob::getStatus)
        .def("engineID", &EventEngineJob::getEngineID)
        .def("shot",
            [](EventEngineJob& self) {
                std::shared_ptr<STI::Engine::Shot> shot;
                self.getShot(shot);
                return shot;
            })
        .def("dependencies",
            [](const EventEngineJob& self) {
                std::shared_ptr<STI::Engine::EventEngineDependencyTree> tree;
                std::shared_ptr<STI::Engine::ParsedDependencyTree> parsedTree;
                
                if (self.getDependencies(tree)) {
                    parsedTree = std::make_shared<STI::Engine::ParsedDependencyTree>(tree);
                }
                else {
                    parsedTree = std::make_shared<STI::Engine::ParsedDependencyTree>();
                }

                return parsedTree;
            })
        .def("dependencyGraph",
            [](const EventEngineJob& self) {

                py::dict nodeDict;
                std::shared_ptr<STI::Engine::EventEngineDependencyTree> tree;
                if (self.getDependencies(tree)) {

                    std::shared_ptr<DependencyTree<DeviceID>> depTree =
                        std::static_pointer_cast<DependencyTree<DeviceID>>(tree);
                    
                    STI::Device::DeviceIDIndexedGraph graph(*depTree);

                    for(auto& node : graph.getNodes()) {
                        nodeDict[node.node.getID().c_str()] = node.outConnections;
                    }
                    return nodeDict;
                }
                return nodeDict;
            })
        .def("getParsingMessages", &EventEngineJob::getParsingMessages)
        // .def("getParsingMessages",
        //     [](EventEngineJob& self) {
        //         // return self.getParsingMessages();
                
        //         std::vector<STI::Engine::EngineParsingMessage> messages = self.getParsingMessages();
        //         std::cout << "getParsingMessages() " << messages.size() << std::endl;

        //         py::list messageList;
        //         for (const auto& msg : messages) {
        //             messageList.append(msg);
        //             std::cout << "Message: " << msg.getMessage() << std::endl;
        //         }
        //         return messageList;
        //     })
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
        .def("closeSequence", &EventEngineScheduler::closeSequence)
        .def("cancelSequence", &EventEngineScheduler::cancelSequence)
        .def("getStatus", py::overload_cast<const STI::Engine::ParseID&>(&EventEngineScheduler::getStatus), py::arg("parseID"))
        .def("getStatus", py::overload_cast<const STI::Engine::ShotID&>(&EventEngineScheduler::getStatus), py::arg("shotID"))
        .def("getStatus", py::overload_cast<const STI::Engine::SequenceID&>(&EventEngineScheduler::getStatus), py::arg("sequenceID"))
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
