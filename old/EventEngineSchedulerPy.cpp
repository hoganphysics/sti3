
#include "EventEngineSchedulerPy.h"

#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/Shot.h>

#include <iostream>


using STI::Python::EventEngineSchedulerPy;
using STI::Engine::EventEngineScheduler;
using STI::Engine::ParseID;
using STI::Engine::ShotID;
using STI::Engine::EngineJobStatus;
using STI::Engine::ParseJobStatus;
using STI::Engine::PlayJobStatus;
using STI::Engine::AddSequenceStatus;
using STI::Engine::Sequence;
using STI::Engine::SequenceEntryID;

EventEngineSchedulerPy::EventEngineSchedulerPy(const std::shared_ptr<EventEngineScheduler>& engineScheduler)
: engineScheduler(engineScheduler)
{
}

EventEngineSchedulerPy::~EventEngineSchedulerPy()
{
}

ParseJobStatus EventEngineSchedulerPy::parse(const std::shared_ptr<STI::Engine::Shot>& shot)
{
    ParseJobStatus parseJobStatus;

    if (engineScheduler != 0) {
        // std::cout << "EventEngineSchedulerPy::parse " << shot->getShotConfig().print() << std::endl;
        parseJobStatus = engineScheduler->parse(shot);
    }
    return parseJobStatus;
}

PlayJobStatus EventEngineSchedulerPy::play(const ParseID& parseID, const STI::Engine::EngineJobSourceID& source)
{
    PlayJobStatus playJobStatus;

    if (engineScheduler != 0) {
        playJobStatus = engineScheduler->play(parseID, source);
    }
    return playJobStatus;
}

AddSequenceStatus EventEngineSchedulerPy::addSequence(const std::shared_ptr<Sequence>& sequence, const STI::Engine::EngineJobSourceID& source)
{
    AddSequenceStatus addSequenceStatus;
    return addSequenceStatus;
}

ParseJobStatus EventEngineSchedulerPy::parse(const std::shared_ptr<STI::Engine::Shot>& shot, const SequenceEntryID& sequenceEntryID)
{
    ParseJobStatus parseJobStatus;

    if (engineScheduler != 0) {
        parseJobStatus = engineScheduler->parse(shot, sequenceEntryID);
    }
    return parseJobStatus;
}



EngineJobStatus EventEngineSchedulerPy::getStatus(const ParseID& pid)
{
    EngineJobStatus status;

    if (engineScheduler != 0) {
        status = engineScheduler->getStatus(pid);
    }
    return status;
}

EngineJobStatus EventEngineSchedulerPy::getStatus(const ShotID& sid)
{
    EngineJobStatus status;

    if (engineScheduler != 0) {
        status = engineScheduler->getStatus(sid);
    }
    return status;
}

void EventEngineSchedulerPy::cancelJob(const STI::Engine::EngineJobID& jobID)
{
    if (engineScheduler != 0) {
        engineScheduler->cancelJob(jobID);
    }
}

void EventEngineSchedulerPy::cancelAll()
{
    if (engineScheduler != 0) {
        engineScheduler->cancelAll();
    }
}

std::set<STI::Engine::EngineJobID> EventEngineSchedulerPy::getJobIDs(const STI::Engine::EventEngineJobList& jobListType) const
{
    std::set<STI::Engine::EngineJobID> ids;
    
    if (engineScheduler != 0) {
        ids = engineScheduler->getJobIDs(jobListType);
    }
    return ids;
}

// std::set<STI::Engine::EngineJobID> EventEngineSchedulerPy::getQueuedJobs() const
// {
//     std::set<STI::Engine::EngineJobID> ids;
    
//     if (engineScheduler != 0) {
//         engineScheduler->getQueuedJobs(ids);
//     }
//     return ids;
// }

// std::set<STI::Engine::EngineJobID> EventEngineSchedulerPy::getRunningJobs() const
// {
//     std::set<STI::Engine::EngineJobID> ids;
    
//     if (engineScheduler != 0) {
//         engineScheduler->getRunningJobs(ids);
//     }
//     return ids;
// }

// std::set<STI::Engine::EngineJobID> EventEngineSchedulerPy::getCompletedJobs() const
// {
//     std::set<STI::Engine::EngineJobID> ids;
    
//     if (engineScheduler != 0) {
//         engineScheduler->getCompletedJobs(ids);
//     }
//     return ids;
// }

