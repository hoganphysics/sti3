
#include "JEventEngineJob.h"
#include <sti/engine/EngineJobID.h>
#include "JShot.h"
#include "EventEngineDependencyTree.h"
#include "JEventEngine.h"
#include <sti/engine/EngineID.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/RawEvent.h>

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


JEventEngineJob::JEventEngineJob()
: JEventEngineJob(0)
{
}

JEventEngineJob::JEventEngineJob(const std::shared_ptr<EventEngineJob>& eventEngineJob)
: eventEngineJob(eventEngineJob)
{
    std::shared_ptr<Shot> shot;

    if (eventEngineJob != 0) {
        eventEngineJob->getShot(shot);
    }

    jshot = std::make_shared<JShot>(shot);
}

JEventEngineJob::~JEventEngineJob()
{
}


EngineJobID JEventEngineJob::getJobID() const
{
    if (eventEngineJob != 0) {
        return eventEngineJob->getJobID();
    }

    EngineJobID missing;
    return missing;
}

STI::Device::DeviceID JEventEngineJob::getJobOwner() const
{
    if (eventEngineJob != 0) {
        return eventEngineJob->getJobOwner();
    }
    STI::Device::DeviceID missing;
    return missing;
}

EngineJobStatus JEventEngineJob::getStatus() const
{
    if (eventEngineJob != 0) {
        return eventEngineJob->getStatus();
    }
    EngineJobStatus missing = EngineJobStatus::NotFound;
    return missing;
}


EngineID JEventEngineJob::getEngineID() const
{
    if (eventEngineJob != 0) {
        return eventEngineJob->getEngineID();
    }
    EngineID missing;
    return missing;
}

std::shared_ptr<JEventEngine> JEventEngineJob::getEngine() const
{
    std::shared_ptr<EventEngine> eventEngine;

    if (eventEngineJob != 0) {
        eventEngineJob->getEngine(eventEngine);
    }

    auto jEventEngine = std::make_shared<JEventEngine>(eventEngine);

    return jEventEngine;
}


std::shared_ptr<JShot> JEventEngineJob::getShot() const
{
    std::shared_ptr<STI::Engine::Shot> shot;

    if (eventEngineJob != 0) {
        eventEngineJob->getShot(shot);
    }

    auto jshot = std::make_shared<JShot>(shot);
    return jshot;
}


std::shared_ptr<EventEngineDependencyTree> JEventEngineJob::getDependencies() const
{
    std::shared_ptr<EventEngineDependencyTree> tree;

    if (eventEngineJob != 0) {
        eventEngineJob->getDependencies(tree);
    }
    else {
        tree = std::make_shared<EventEngineDependencyTree>();   //empty
    }

    return tree;
}

STI::Device::DeviceIDIndexedGraph JEventEngineJob::getDependenciesIndexed() const
{
    std::shared_ptr<EventEngineDependencyTree> tree;

    if (eventEngineJob != 0) {
        eventEngineJob->getDependencies(tree);
    }

    if (tree != 0) {
        STI::Device::DeviceIDIndexedGraph graph(*tree);
        return graph;
    }
    STI::Device::DeviceIDIndexedGraph emptyGraph;
    return emptyGraph;
}

std::set<STI::Device::DeviceID> JEventEngineJob::getMissingTargetIDs() const
{
    if (eventEngineJob != 0) {
        return eventEngineJob->getMissingTargetIDs();
    }

    std::set<STI::Device::DeviceID> missing;
    return missing;
}


std::vector<EngineParsingMessage> JEventEngineJob::getParsingMessages() const
{
    if (eventEngineJob != 0) {
        return eventEngineJob->getParsingMessages();
    }

    std::vector<EngineParsingMessage> missing;
    return missing;
}

