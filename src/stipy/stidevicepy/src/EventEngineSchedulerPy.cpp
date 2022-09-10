
#include "EventEngineSchedulerPy.h"
#include <sti/engine/ParseID.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/EngineJobID.h>

#include <iostream>
#include <sti/engine/Shot.h>

using STI::Python::EventEngineSchedulerPy;
using STI::Engine::EventEngineScheduler;
using STI::Engine::ParseID;
using STI::Engine::ShotID;
using STI::Engine::EngineJobStatus;
// using STI::Python::LocalShotPy;



EventEngineSchedulerPy::EventEngineSchedulerPy(const std::shared_ptr<EventEngineScheduler>& engineScheduler)
: engineScheduler(engineScheduler)
{
}

EventEngineSchedulerPy::~EventEngineSchedulerPy()
{
}

ParseID EventEngineSchedulerPy::parse(const std::shared_ptr<STI::Engine::Shot>& shot)
{
    ParseID pid;

    if (engineScheduler != 0) {
        std::cout << "EventEngineSchedulerPy::parse " << shot->getShotConfig().print() << std::endl;
        pid = engineScheduler->parse(shot);
    }
    return pid;
}

ShotID EventEngineSchedulerPy::play(const ParseID& parseID, const STI::Engine::EngineJobSourceID& source)
{
    ShotID sid;

    if (engineScheduler != 0) {
        sid = engineScheduler->play(parseID, source);
    }
    return sid;
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

std::set<STI::Engine::EngineJobID> EventEngineSchedulerPy::getQueuedJobs() const
{
    std::set<STI::Engine::EngineJobID> ids;
    
    if (engineScheduler != 0) {
        engineScheduler->getQueuedJobs(ids);
    }
    return ids;
}

std::set<STI::Engine::EngineJobID> EventEngineSchedulerPy::getRunningJobs() const
{
    std::set<STI::Engine::EngineJobID> ids;
    
    if (engineScheduler != 0) {
        engineScheduler->getRunningJobs(ids);
    }
    return ids;
}

std::set<STI::Engine::EngineJobID> EventEngineSchedulerPy::getCompletedJobs() const
{
    std::set<STI::Engine::EngineJobID> ids;
    
    if (engineScheduler != 0) {
        engineScheduler->getCompletedJobs(ids);
    }
    return ids;
}

