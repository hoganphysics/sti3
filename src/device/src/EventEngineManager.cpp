#include "EventEngineManager.h"

#include <sti/engine/EngineID.h>
#include <sti/engine/EventEngineJob.h>
#include <sti/engine/ParseID.h>

#include "LocalEventEngine.h"
#include "LocalEventEngineScheduler.h"

#include <mutex>

using STI::Engine::EventEngineManager;
using STI::Engine::EventEngineJob;
using STI::Engine::ParseID;
using STI::Engine::EngineID;
using STI::Engine::LocalEventEngine;
using STI::Engine::LocalEventEngineScheduler;


EventEngineManager::EventEngineManager(const EngineID& engineID, 
                                        const std::shared_ptr<LocalEventEngine>& engine, 
                                        LocalEventEngineScheduler* scheduler)
: engineID(engineID), engine(engine), scheduler(scheduler), running(false), jobFinished(true)
{
}

EventEngineManager::~EventEngineManager()
{
    abortJob();
    
    if(jobThread.joinable()) {
        jobThread.join();
    }
}

bool EventEngineManager::getEngine(std::shared_ptr<LocalEventEngine>& eventEngine)
{
    eventEngine = engine;
    
    return (eventEngine != 0);
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

// bool EventEngineManager::submitJob(const std::shared_ptr<EventEngineJob>& job)
// {
//     std::thread threadToJoin;
//     std::unique_lock<std::mutex> writeLock(jobMutex);

//     if(running || job == 0) {
//         return false;
//     }

//     // if (job->getStatus() != STI::Engine::EngineJobStatus::New) {
//     //     return false;
//     // }

//     if (jobThread.joinable()) {
//         if (!jobFinished) {
//             return false;
//         }
//         threadToJoin = std::move(jobThread);
//     }

//     currentJob = job;
//     currentJob->markRunning(engineID);
//     running = true;
//     jobFinished = false;

//     jobThread = std::thread(&EventEngineManager::runJob, this);

//     writeLock.unlock();
//     if (threadToJoin.joinable()) {
//         threadToJoin.join();
//     }

//     return true;
// }

bool EventEngineManager::submitJob(const std::shared_ptr<EventEngineJob>& job)
{
    std::unique_lock<std::mutex> writeLock(jobMutex);

    if(running || job == 0) {
        return false;
    }

    // if (job->getStatus() != STI::Engine::EngineJobStatus::New) {
    //     return false;
    // }

    if(jobThread.joinable()) {
        jobThread.join();
    }

    currentJob = job;
    currentJob->markRunning(engineID);
    running = true;

    jobThread = std::thread(&EventEngineManager::runJob, this);
    
	return true;

}

// void EventEngineManager::joinJobThread()
// {
//     std::thread threadToJoin;

//     {
//         std::unique_lock<std::mutex> writeLock(jobMutex);
//         if (!jobThread.joinable() || !jobFinished) {
//             return;
//         }
//         threadToJoin = std::move(jobThread);
//     }

//     threadToJoin.join();
// }

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
    std::unique_lock<std::mutex> writeLock(jobMutex);
    
    if (currentJob != 0) {
        currentJob->markCancelled();
    }
    
    if (running) {
        engine->stop();
    }

    running = false;
}

void EventEngineManager::runJob()
{
    if(currentJob == 0) {
        return;
    }

    switch(currentJob->getJobID().type) {
        case EventEngineJobType::Parse:
            scheduler->parseJob(currentJob);
            currentJob->setEventEngine(engine);
            engine->parse(*currentJob); 
        break;

        case EventEngineJobType::Play:
            currentJob->setEventEngine(engine);
            engine->play(*currentJob);  //reserve not needed because it's already handled by the queue system.  When play is called, engines should call upstreat with their reference; server then calls play when all have been received.
        break;
    }
   
    if (engine->jobCancelled()) {
        if (currentJob->getJobID().type == EventEngineJobType::Parse) {
            scheduler->saveFailedSequenceParse(currentJob);
        }
        scheduler->cancelJob(currentJob->getJobID());
    }
    else {
        scheduler->jobComplete(currentJob->getJobID());
    }

    {
        std::unique_lock<std::mutex> writeLock(jobMutex);
        running = false;
    }

    // {
    //     std::unique_lock<std::mutex> writeLock(jobMutex);
    //     jobFinished = true;
    // }
}

void EventEngineManager::unloadEngine()
{
    if (engine != 0) {
        engine->unload();
    }
}

void EventEngineManager::handleParseMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& evt)
{
    if(jobRunning()) {
        engine->handleParseMessage(evt);
    }
}

void EventEngineManager::handlePlayReadyMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& evt)
{
    if(jobRunning()) {
        engine->handlePlayReadyMessage(evt);
    }
}

void EventEngineManager::handlePlayCompleteMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& evt)
{
    if(jobRunning()) {
        engine->handlePlayCompleteMessage(evt);
    }
}
