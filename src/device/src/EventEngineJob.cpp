

#include "EventEngineJob.h"
#include "ParsedShot.h"
#include "EventEngineDependencyTree.h"
#include "DeviceID.h"
#include "ParseID.h"
#include "EngineJobID.h"

#include <set>
#include <memory>


using STI::Engine::EventEngineJob;
using STI::Engine::EngineJobID;
using STI::Engine::ParseID;
using STI::Engine::ParsedShot;
using STI::Engine::EventEngineDependencyTree;
using STI::Engine::EventEngineJobType;


EventEngineJob::EventEngineJob(const ParseID& parseID, 
                               const std::shared_ptr<ParsedShot>& shot,
                               const std::shared_ptr<EventEngineDependencyTree>& tree, 
                               const STI::Device::DeviceID& owner, 
                               const std::set<STI::Device::DeviceID>& missingTargets)
: parsedShot(shot), dependencies(tree), jobOwner(owner)
{
    std::unique_lock< std::mutex > writeLock(jobMutex);

    missingTargetIDs = missingTargets;

    status = EventEngineJob::EngineJobStatus::New;

    jobID.type = EventEngineJobType::Parse;
    jobID.pid = parseID;
}


EngineJobID EventEngineJob::getJobID()
{
    std::unique_lock< std::mutex > writeLock(jobMutex);
    return jobID;
}


EventEngineJob::EngineJobStatus EventEngineJob::getStatus()
{
    std::unique_lock< std::mutex > writeLock(jobMutex);
    return status;
}

void EventEngineJob::markComplete()
{
    std::unique_lock< std::mutex > writeLock(jobMutex);
    status = EventEngineJob::EngineJobStatus::Completed;
}


void EventEngineJob::markCancelled()
{
     std::unique_lock< std::mutex > writeLock(jobMutex);
     status = EventEngineJob::EngineJobStatus::Cancelled;
}
