
#include "JEventEngineScheduler.h"
#include "JShot.h"

#include "ParseID.h"
#include "ShotID.h"
#include "EngineJobID.h"
#include "JEventEngineJob.h"

using STI::Engine::JEventEngineScheduler;
using STI::Engine::EventEngineJob;
using STI::Engine::EventEngineDependencyTree;
using STI::Engine::EngineJobID;
using STI::Engine::JEventEngineJob;


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

std::shared_ptr<JEventEngineJob> JEventEngineScheduler::getJob(const EngineJobID& id) const
{
    std::shared_ptr<JEventEngineJob> jJob;
    std::shared_ptr<EventEngineJob> job;

    if(localScheduler != 0) {
        if (localScheduler->getJob(id, job)) {
            jJob = std::make_shared<JEventEngineJob>(job);
        }
    }
    return jJob;
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

std::set<EngineJobID> JEventEngineScheduler::getJobIDs(const EventEngineJobList& jobListType) const
{
    std::set<EngineJobID> ids;

    if(localScheduler != 0) {
        ids = localScheduler->getJobIDs(jobListType);
    }
    return ids;
}

std::vector<std::shared_ptr<JEventEngineJob>> JEventEngineScheduler::getJobs(const EventEngineJobList& jobListType) const
{
    std::vector<std::shared_ptr<EventEngineJob>> jobs;
    std::vector<std::shared_ptr<JEventEngineJob>> jJobs;

    if(localScheduler != 0) {
        jobs = localScheduler->getJobs(jobListType);
    }
    for (auto job : jobs) {
        jJobs.push_back(std::make_shared<JEventEngineJob>(job));
    }
    return jJobs;
}

// std::set<EngineJobID> JEventEngineScheduler::getQueuedJobs() const
// {
//     std::set<EngineJobID> ids;

//     if(localScheduler != 0) {
//         localScheduler->getQueuedJobs(ids);
//     }
//     return ids;
// }

// std::set<EngineJobID> JEventEngineScheduler::getRunningJobs() const
// {
//     std::set<EngineJobID> ids;

//     if(localScheduler != 0) {
//         localScheduler->getRunningJobs(ids);
//     }
//     return ids;
// }

// std::set<EngineJobID> JEventEngineScheduler::getCompletedJobs() const
// {
//     std::set<EngineJobID> ids;

//     if(localScheduler != 0) {
//         localScheduler->getCompletedJobs(ids);
//     }
//     return ids;
// }

