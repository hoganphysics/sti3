
#include "JEventEngineScheduler.h"
#include "JShot.h"

using STI::Engine::JEventEngineScheduler;
using STI::Engine::EventEngineJob;
using STI::Engine::EventEngineDependencyTree;


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

void JEventEngineScheduler::parse(const STI::Engine::ParseID& parseID, const std::shared_ptr<STI::Engine::JShot>& jshot)
{
    if(localScheduler != 0) {
        std::shared_ptr<STI::Engine::Shot> shot = std::static_pointer_cast<STI::Engine::Shot>(jshot);
        localScheduler->parse(parseID, shot);
    }
}

void JEventEngineScheduler::cancelJob(const STI::Engine::EngineJobID& jobID)
{
    if(localScheduler != 0) {
        localScheduler->cancelJob(jobID);
    }
}

// std::shared_ptr<EventEngineJob> JEventEngineScheduler::createJob(const STI::Engine::ParseID& parseID, 
//                                                     const std::shared_ptr<STI::Engine::Shot>& shot,
//                                                     const std::shared_ptr<EventEngineDependencyTree>& tree, 
//                                                     const STI::Device::DeviceID& owner, 
//                                                     const std::set<STI::Device::DeviceID>& missingTargets)
// {
//     std::shared_ptr<STI::Engine::EventEngineJob> job;

//     if(localScheduler != 0) {
//         job = localScheduler->createJob(parseID, shot, tree, owner, missingTargets);
//     }

//     return job;

// }

