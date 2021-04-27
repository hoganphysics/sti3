

#include "LocalEventEngineJob.h"
#include "EventEngineJob.h"
#include "Shot.h"
#include "EventEngineDependencyTree.h"
#include "DeviceID.h"
#include "ParseID.h"
#include "EngineJobID.h"
#include "EngineID.h"
#include "EngineParsingMessage.h"
#include "RawEvent.h"

#include <set>
#include <memory>

using STI::Engine::EventEngineJob;
using STI::Engine::LocalEventEngineJob;
using STI::Engine::EngineJobID;
using STI::Engine::ParseID;
using STI::Engine::Shot;
using STI::Engine::EventEngineDependencyTree;
using STI::Engine::EventEngineJobType;
using STI::Device::DeviceID;
using STI::Engine::EngineID;
using STI::Engine::EngineParsingMessage;
using STI::Engine::ParsingMessageType;


LocalEventEngineJob::LocalEventEngineJob(const ParseID& parseID, 
                                         const std::shared_ptr<Shot>& shot,
                                         const STI::Device::DeviceID& owner)
: parsedShot(shot), jobOwner(owner)
{
    std::unique_lock< std::mutex > writeLock(jobMutex);

    status = EventEngineJob::EngineJobStatus::New;
    jobID.type = EventEngineJobType::Parse;

    jobID.pid = parseID;
}


LocalEventEngineJob::LocalEventEngineJob(const EngineJobID& id, const DeviceID& owner)
: jobID(id), jobOwner(owner)
{
    status = EventEngineJob::EngineJobStatus::New;
    jobID.type = EventEngineJobType::Play;
}
                   

EngineJobID LocalEventEngineJob::getJobID() const
{
    std::unique_lock< std::mutex > writeLock(jobMutex);
    return jobID;
}

STI::Device::DeviceID LocalEventEngineJob::getJobOwner() const
{
    return jobOwner;
}

EventEngineJob::EngineJobStatus LocalEventEngineJob::getStatus() const
{
    std::unique_lock< std::mutex > writeLock(jobMutex);
    return status;
}

void LocalEventEngineJob::markRunning(const EngineID& id)
{
    std::unique_lock< std::mutex > writeLock(jobMutex);
    status = EventEngineJob::EngineJobStatus::Running;
    engineID = id;
}

void LocalEventEngineJob::markComplete()
{
    std::unique_lock< std::mutex > writeLock(jobMutex);
    status = EventEngineJob::EngineJobStatus::Completed;
}


void LocalEventEngineJob::markCancelled()
{
     std::unique_lock< std::mutex > writeLock(jobMutex);
     status = EventEngineJob::EngineJobStatus::Cancelled;
}

void LocalEventEngineJob::attachSubjob(const std::shared_ptr<EventEngineJob>& job)
{
    //store job references for owned devices
    //This is needed so remote owned devices have a servant.
    attachedJobs.push_back(job);
}

bool LocalEventEngineJob::getParsedShot(std::shared_ptr<Shot>& shot) const
{
    shot = parsedShot;

    return (shot != 0);
}

bool LocalEventEngineJob::getDependencies(std::shared_ptr<EventEngineDependencyTree>& tree) const
{
    tree = dependencies;

    return (tree != 0);
}

std::set<STI::Device::DeviceID> LocalEventEngineJob::getMissingTargetIDs() const
{
    return missingTargetIDs;
}

void LocalEventEngineJob::setDependencies(const std::shared_ptr<EventEngineDependencyTree>& tree)
{
    dependencies = tree;
}
void LocalEventEngineJob::setMissingTargets(const std::set<STI::Device::DeviceID>& missingTargets)
{
    missingTargetIDs = missingTargets;
}

EngineParsingMessage& LocalEventEngineJob::addMessage(const EngineParsingMessage& message)
{
    parsingMessages.push_back(std::move(message));
    return parsingMessages.back();
}

EngineParsingMessage& LocalEventEngineJob::addMessage(const ParsingMessageType& type, unsigned id, const std::string& name)
{
    parsingMessages.emplace_back(type, id, name);
    return parsingMessages.back();
}
