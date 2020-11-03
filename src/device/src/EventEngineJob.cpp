

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
using STI::Device::DeviceID;


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

EventEngineJob::EventEngineJob(const EngineJobID& id, const DeviceID& owner)
: jobID(id), jobOwner(owner)
{
    jobID.type = EventEngineJobType::Play;
}
                   

EngineJobID EventEngineJob::getJobID() const
{
    std::unique_lock< std::mutex > writeLock(jobMutex);
    return jobID;
}


EventEngineJob::EngineJobStatus EventEngineJob::getStatus() const
{
    std::unique_lock< std::mutex > writeLock(jobMutex);
    return status;
}

void EventEngineJob::markRunning(const EngineID& id)
{
    std::unique_lock< std::mutex > writeLock(jobMutex);
    status = EventEngineJob::EngineJobStatus::Running;
    engineID = id;
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
