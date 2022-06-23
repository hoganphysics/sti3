
#include "JEventEngineScheduler.h"
#include "JShot.h"

#include <sti/engine/ParseID.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/EngineJobID.h>


using STI::Engine::JEventEngineScheduler;
using STI::Engine::EventEngineJob;
using STI::Engine::EventEngineDependencyTree;
using STI::Engine::EngineJobID;


JEventEngineScheduler::JEventEngineScheduler(const std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
    : localScheduler(scheduler)
{

}

JEventEngineScheduler::~JEventEngineScheduler()
{
}


// void JEventEngineScheduler::getDependants(const std::set<STI::Device::DeviceID>& evtTargets, STI::Engine::EventEngineDependencyTree& tree, 
//                             std::set<STI::Device::DeviceID>& missingTargets, const STI::Device::DeviceTrace& trace)
// {
//     if(localScheduler != 0) {
//         localScheduler->getDependants(evtTargets, tree, missingTargets, trace);
//     }
// }


// void JEventEngineScheduler::addDeviceEventTargets(STI::Engine::EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace)
// {
//     if(localScheduler != 0) {
//         localScheduler->addDeviceEventTargets(tree, trace);
//     }
// }


// void JEventEngineScheduler::addJob(const std::shared_ptr<STI::Engine::EventEngineJob>& newJob)
// {
//     if(localScheduler != 0) {
//         localScheduler->addJob(newJob);
//     }
// }


STI::Engine::ParseID JEventEngineScheduler::parse(const std::shared_ptr<STI::Engine::JShot>& jshot)
{
    STI::Engine::ParseID pid;

    if(localScheduler != 0) {
        std::shared_ptr<STI::Engine::Shot> shot = std::static_pointer_cast<STI::Engine::Shot>(jshot);
        pid = localScheduler->parse(shot);
    }
    return pid;
}

STI::Engine::ShotID JEventEngineScheduler::play(const ParseID& parseID, const EngineJobSourceID& source)
{
    STI::Engine::ShotID sid;

    if(localScheduler != 0) {
        sid = localScheduler->play(parseID, source);
    }
    return sid;
}

void JEventEngineScheduler::cancelJob(const STI::Engine::EngineJobID& jobID)
{
    if(localScheduler != 0) {
        localScheduler->cancelJob(jobID);
    }
}

void JEventEngineScheduler::cancelAll()
{
    if(localScheduler != 0) {
        localScheduler->cancelAll();
    }
}

std::set<EngineJobID> JEventEngineScheduler::getQueuedJobs() const
{
    std::set<EngineJobID> ids;

    if(localScheduler != 0) {
        localScheduler->getQueuedJobs(ids);
    }
    return ids;
}

std::set<EngineJobID> JEventEngineScheduler::getRunningJobs() const
{
    std::set<EngineJobID> ids;

    if(localScheduler != 0) {
        localScheduler->getRunningJobs(ids);
    }
    return ids;
}

std::set<EngineJobID> JEventEngineScheduler::getCompletedJobs() const
{
    std::set<EngineJobID> ids;

    if(localScheduler != 0) {
        localScheduler->getCompletedJobs(ids);
    }
    return ids;
}

