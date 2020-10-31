

#include "EventEngineManager.h"
#include "EventEngineJob.h"
#include "EventEngineScheduler.h"
#include "ParseID.h"

#include <mutex>

using STI::Engine::EventEngineManager;
using STI::Engine::EventEngineJob;
using STI::Engine::ParseID;


EventEngineManager::EventEngineManager(std::shared_ptr<EventEngine> engine, EventEngineScheduler* scheduler)
: engine(engine), scheduler(scheduler), running(false)
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
            //engine->parse(currentJob->getJobID().pid, currentJob->parsedShot.events, currentJob->jobOwner);
            //could do parseReserve(job) here, allowing each server to get devices reserved. Would respond to yield. Same for play.
            engine->parse(currentJob);
        break;
        case EventEngineJobType::Play:
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
