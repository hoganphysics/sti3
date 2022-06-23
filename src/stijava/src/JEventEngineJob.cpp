
#include "JEventEngineJob.h"
#include <sti/engine/EngineJobID.h>
#include "JShot.h"
#include "EventEngineDependencyTree.h"
#include "JEventEngine.h"


using STI::Engine::JEventEngineJob;
using STI::Engine::EngineJobID;
using STI::Engine::EventEngineJob;
using STI::Engine::EngineID;
using STI::Engine::EngineParsingMessage;
using STI::Engine::EngineID;
using STI::Engine::JShot;
using STI::Engine::EventEngineDependencyTree;
using STI::Engine::EventEngine;
using STI::Engine::JEventEngine;
using STI::Engine::EngineJobStatus;


JEventEngineJob::JEventEngineJob(const std::shared_ptr<EventEngineJob>& eventEngineJob)
: eventEngineJob(eventEngineJob)
{
    std::shared_ptr<Shot> shot;
    eventEngineJob->getShot(shot);

    jshot = std::make_shared<JShot>(shot);
}

JEventEngineJob::~JEventEngineJob()
{
}


EngineJobID JEventEngineJob::getJobID() const
{
    return eventEngineJob->getJobID();
}

STI::Device::DeviceID JEventEngineJob::getJobOwner() const
{
    return eventEngineJob->getJobOwner();
}

EngineJobStatus JEventEngineJob::getStatus() const
{
    return eventEngineJob->getStatus();
}


const EngineID& JEventEngineJob::getEngineID() const
{
    return eventEngineJob->getEngineID();
}

std::shared_ptr<JEventEngine> JEventEngineJob::getEngine() const
{
    std::shared_ptr<EventEngine> eventEngine;
    eventEngineJob->getEngine(eventEngine);

    auto jEventEngine = std::make_shared<JEventEngine>(eventEngine);

    return jEventEngine;
}


std::shared_ptr<JShot> JEventEngineJob::getShot() const
{
    std::shared_ptr<STI::Engine::Shot> shot;
    eventEngineJob->getShot(shot);

    auto jshot = std::make_shared<JShot>(shot);
    return jshot;
}


std::shared_ptr<EventEngineDependencyTree> JEventEngineJob::getDependencies() const
{
    std::shared_ptr<EventEngineDependencyTree> tree;
    eventEngineJob->getDependencies(tree);

    return tree;
}


std::set<STI::Device::DeviceID> JEventEngineJob::getMissingTargetIDs() const
{
    return eventEngineJob->getMissingTargetIDs();
}


const std::vector<EngineParsingMessage>& JEventEngineJob::getParsingMessages() const
{
    return eventEngineJob->getParsingMessages();
}

