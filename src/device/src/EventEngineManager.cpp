

#include "EventEngineManager.h"
#include "EventEngineJob.h"
#include "EventEngineScheduler.h"
#include "ParseID.h"
#include "EngineID.h"

#include <mutex>

using STI::Engine::EventEngineManager;
using STI::Engine::EventEngineJob;
using STI::Engine::ParseID;
using STI::Engine::EngineID;


EventEngineManager::EventEngineManager(const EngineID& engineID, std::shared_ptr<EventEngine> engine, EventEngineScheduler* scheduler)
: engineID(engineID), engine(engine), scheduler(scheduler), running(false)
{
}

EventEngineManager::~EventEngineManager()
{
    abortJob();
    
    if(jobThread.joinable()) {
        jobThread.join();
    }
}

bool EventEngineManager::isParsed(const ParseID& parseID)
{
    if (jobRunning()) return false;

    return engine->isState(STI::Engine::EngineState::Parsed) 
            && engine->getLastParseID() == parseID;
}

const ParseID& EventEngineManager::getLastParseID()
{
    return engine->getLastParseID();
}

bool EventEngineManager::submitJob(const std::shared_ptr<EventEngineJob>& job)
{
    std::unique_lock<std::mutex> writeLock(jobMutex);

    if(running || job == 0) {
        return false;
    }

    if(jobThread.joinable()) {
        jobThread.join();
    }

    currentJob = job;
    currentJob->markRunning(engineID);
    running = true;

    jobThread = std::thread(&EventEngineManager::runJob, this);
    
}

bool EventEngineManager::getJob(std::shared_ptr<EventEngineJob>& job)
{
    std::unique_lock<std::mutex> writeLock(jobMutex);
    
    if(currentJob != 0) {
        job = currentJob;
        return true;
    }
    return false;
}

bool EventEngineManager::jobRunning()
{
    std::unique_lock<std::mutex> writeLock(jobMutex);
    return running;
}

void EventEngineManager::abortJob()
{
   // std::unique_lock<std::mutex> writeLock(jobMutex);
    engine->stop();
}

void EventEngineManager::runJob()
{
    if(currentJob == 0) {
        return;
    }

    switch(currentJob->getJobID().type) {
        case EventEngineJobType::Parse:
            currentJob->setEventEngine(engine);
            engine->parse(*currentJob); 
        break;

        case EventEngineJobType::Play:
            currentJob->setEventEngine(engine);
            engine->play(*currentJob);  //reserve not needed because it's already handled by the queue system.  When play is called, engines should call upstreat with their reference; server then calls play when all have been received.
        break;
    }

    std::unique_lock<std::mutex> writeLock(jobMutex);
    running = false;

    scheduler->jobComplete(currentJob->getJobID());
}

void EventEngineManager::handleParseMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& evt)
{
    if(jobRunning()) {
        engine->handleParseMessage(evt);
    }
}

void EventEngineManager::handlePlayMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& evt)
{
    if(jobRunning()) {
        engine->handlePlayMessage(evt);
    }
}
