#include "LocalEventEngineScheduler.h"

#include <sti/fwd/RawEvent_fwd.h>

#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessage.h>

#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/EventEngineJob.h>
#include <sti/engine/EngineConflictPolicy.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/ResultsCollector.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/ResultTicket.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/SequenceJob.h>
#include <sti/engine/SequenceResult.h>
#include <sti/engine/Shot.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/StackTraceData.h>

#include <sti/utils/SynchronizedMap.h>
#include <sti/utils/VirtualFileHolder.h>
#include <sti/utils/VirtualFileServer.h>

#include "EventEngineFactory.h"
#include "EventEngineManager.h"
#include "LocalEventEngineDependencyParser.h"
#include "LocalEventEngineFactory.h"
#include "LocalEventEngineJob.h"
#include "LocalShot.h"



#include <set>
#include <vector>
#include <memory>
#include <algorithm>


using STI::Device::DeviceID;
using STI::Device::EngineSchedulerMessage;

using STI::Engine::LocalEventEngineJob;
using STI::Engine::LocalEventEngineScheduler;
using STI::Engine::EventEngineDependencyTree;
using STI::Engine::EngineID;
using STI::Engine::EventEngine;
using STI::Engine::LocalEventEngine;
using STI::Engine::ParseID;
using STI::Engine::EventEngineJob;
using STI::Engine::Shot;
using STI::Engine::LocalEventEngineJob;
using STI::Engine::ShotID;
using STI::Engine::ShotResult;
using STI::Engine::EngineJobID;
using STI::Engine::EventEngineManager;
using STI::Engine::LocalEventEngineFactory;
using STI::Engine::EngineParsingMessage;
using STI::Engine::ParsingMessageType;
using STI::Engine::LocalShot;
using STI::Engine::ResultTicket;
using STI::Engine::ResultsCollector;
using STI::Engine::EngineJobStatus;
using STI::Engine::EventEngineDependencyParser;
using STI::Engine::SequenceID;
using STI::Engine::Sequence;
using STI::Engine::SequenceEntryID;
using STI::Engine::SequenceResult;
using STI::Engine::ParseJobStatus;
using STI::Engine::PlayJobStatus;
using STI::Engine::AddSequenceStatus;
using STI::Engine::StackTraceData;
using STI::Utils::FileServer;
using STI::Utils::FileID;
using STI::Utils::VirtualFileHolder;
using STI::Utils::VirtualFileServer;
using STI::Engine::SequenceJob;
using STI::Engine::EngineState;

std::map<std::string, unsigned> LocalEventEngineScheduler::playMessageIDs;


LocalEventEngineScheduler::LocalEventEngineScheduler(STI::Device::LocalDevice* localDevice, 
                                                    const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory,
                                                    const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher,
                                                    const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager)
: MessageGenerator(dispatcher), completedParseJobs(3), completedPlayJobs(3), completedSequenceJobs(3), persistenceManager(persistenceManager)
{
    completedParseJobs.setMaxSize(3);
    completedPlayJobs.setMaxSize(6);
    completedSequenceJobs.setMaxSize(3);

    localDeviceID = localDevice->getID();
    
    localDependencyParser = std::make_shared<LocalEventEngineDependencyParser>(localDevice);

    running = true;
    schedulerThread = std::thread(&LocalEventEngineScheduler::assignJobs, this);

    searchingParseResult = false;
    searchingShotResult = false;

    setEngineFactory(engineFactory);

    engineSchedulerMessageListenerDelegate = std::make_shared<LocalEventEngineScheduler::EngineSchedulerMessageListenerDelegate>(this);

    conflictPolicy = std::make_shared<EngineConflictPolicyDefault>();

    // sequenceJobs.addListener()

}

LocalEventEngineScheduler::~LocalEventEngineScheduler()
{
    stopAll();
    clearAll();
    
    stop();
    schedulerThread.join();
}


bool LocalEventEngineScheduler::getDependencyParser(std::shared_ptr<EventEngineDependencyParser>& dependencyParser)
{
    dependencyParser = localDependencyParser;
    return (dependencyParser != 0);
}

void LocalEventEngineScheduler::stop()
{
    // std::unique_lock<std::mutex> jobLock(jobMutex);
    running = false;

    jobCondition.notify_all();
}

void LocalEventEngineScheduler::setEngineFactory(const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory)
{
    if (engineFactory == 0) {
        return;
    }

    eventEngineFactory = engineFactory;

    //replace existing engines using new factory

    std::set<EngineID> ids;
    engineManagers.getKeys(ids);
    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<LocalEventEngine> engine;

    for (auto& id : ids) {
        if (engineManagers.get(id, manager) && manager != 0 && manager->getEngine(engine)) {
            
            addEngine(id, engine->getDeviceParser(), engine->getTriggerTarget());  //replaces existing engine with newly created engine (using new factory)
        }
    }
}

void LocalEventEngineScheduler::addEngine(const EngineID& engineID, DeviceEventParser* deviceParser, EngineTriggerTarget* triggerTarget)
{
    if (eventEngineFactory != 0) {
 
        std::shared_ptr<LocalEventEngine> engine = eventEngineFactory->createEngine(engineID, deviceParser, triggerTarget);
        auto manager = std::make_shared<EventEngineManager>(engineID, engine, this);

        engineManagers.add(engineID, manager);        
    }
}

void LocalEventEngineScheduler::getEngineIDs(std::set<EngineID>& engineIDs) const
{
    engineManagers.getKeys(engineIDs);
}

EngineState LocalEventEngineScheduler::getEngineState(const EngineID& engineID) const
{
    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<LocalEventEngine> engine;

    if (engineManagers.get(engineID, manager) && manager != 0 && manager->getEngine(engine) && engine != 0) {
        return engine->getState();
    }
    return EngineState::Missing;
}

void LocalEventEngineScheduler::stopEngine(const EngineID& engineID)
{
    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<LocalEventEngine> engine;

    if (engineManagers.get(engineID, manager) && manager != 0 && manager->getEngine(engine) && engine != 0) {
        engine->stop();
    }
}

void LocalEventEngineScheduler::getEngineStates(std::map<EngineID, EngineState>& engineStates) const
{
    std::set<EngineID> engineIDs;
    engineManagers.getKeys(engineIDs);
    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<LocalEventEngine> engine;

    for (auto& id : engineIDs) {
        engineStates[id] = getEngineState(id);
    }
}

void LocalEventEngineScheduler::setEngineConflictPolicy(const std::shared_ptr<EngineConflictPolicy>& policy)
{
    if (policy == 0) return;
    conflictPolicy = policy;
}

EngineJobStatus LocalEventEngineScheduler::getStatus(const ParseID& pid)
{
    std::shared_ptr<EventEngineJob> job;

    EngineJobStatus status;
    
    if (findJob(pid, job)) {
        status = job->getStatus();
    }
    else {
        status = EngineJobStatus::NotFound;
    }

    return status;
}

EngineJobStatus LocalEventEngineScheduler::getStatus(const ShotID& sid)
{
    std::shared_ptr<EventEngineJob> job;

    EngineJobStatus status;
    
    if (findJob(sid, job)) {
        status = job->getStatus();
    }
    else {
        status = EngineJobStatus::NotFound;
    }

    return status;
}

EngineJobStatus LocalEventEngineScheduler::getStatus(const SequenceID& seqID)
{
    std::shared_ptr<SequenceJob> job;

    EngineJobStatus status;
    
    if (findJob(seqID, job)) {
        status = job->getJobStatus();
    }
    else {
        status = EngineJobStatus::NotFound;
    }

    return status;
}


/// Recursively find all concrete event targets in event group
void LocalEventEngineScheduler::findEventTargets(const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup, std::set<DeviceID>& eventTargets)
{
    if (eventGroup == 0) return;

    if (eventGroup->getEvents() != 0) {
        auto events = eventGroup->getEvents();
        for(auto& evt : *events) {
            if (!evt.target().isAbstract()) {
                eventTargets.insert(evt.target().device().deviceID());
            }
        }
    }

    for (auto& g : eventGroup->getSubgroups()) {
        findEventTargets(g, eventTargets);
    }
}


ParseJobStatus LocalEventEngineScheduler::parse(const std::shared_ptr<Shot>& shot)
{
    if (shot == 0) {
        ParseJobStatus parseJobStatus;
        parseJobStatus.status = EngineJobStatus::Canceled;
        return parseJobStatus;
    }

    ParseJobStatus parseJobStatus;
    parseJobStatus.status = EngineJobStatus::New;
    parseJobStatus.pid = ParseID::generateUniqueID(shot->getShotConfig().jobSourceID);
    parseJobStatus.pid.shotType = shot->getShotConfig().shotType;

    auto job = std::make_shared<LocalEventEngineJob>(parseJobStatus.pid, shot, localDeviceID);

    //parse(job);
    addJob(job);

    return parseJobStatus;
}


void LocalEventEngineScheduler::transferTimingFiles(StackTraceData& stackTraceData, FileServer& remoteFileSever, VirtualFileServer& targetFileServer)
{
    const std::vector<FileID>& fileIDs = stackTraceData.getTimingFiles();

    for (auto& fileID : fileIDs) {
        auto virtualFile = persistenceManager->makeVirtualFileHolder(fileID);
        remoteFileSever.transferFile(fileID, virtualFile, STI::Utils::FileTransferType::Binary);
        targetFileServer.addFile(virtualFile);
        stackTraceData.replaceFile(fileID.getFullFilename(), virtualFile->getID()); //replace orginal filename with new FileID
    }
    
}

void LocalEventEngineScheduler::parseJob(const std::shared_ptr<EventEngineJob>& job)
{
    if (job == 0) return;

    if (job->getJobOwner() != localDeviceID) {
        // Only parse jobs owned by local device need to be processed here
        return;
    }

    //Get list unique device targets
    std::set<DeviceID> eventTargets;
    std::shared_ptr<STI::Engine::RawEventGroup> eventGroup;

    std::shared_ptr<Shot> shot;
    if (job->getShot(shot)) {
        shot->getRootEventGroup(eventGroup);
    }

    if (eventGroup != 0) {
        findEventTargets(eventGroup, eventTargets);

        auto stackTraceData = eventGroup->getStackTraceData();
        std::shared_ptr<STI::Utils::FileServer> remoteFileServer;
        auto virtualFileServer = persistenceManager->makeVirtualFileServer();
        
        if (stackTraceData != 0 && stackTraceData->getFileServer(remoteFileServer)) {
            transferTimingFiles(*stackTraceData, *remoteFileServer, *virtualFileServer);

            stackTraceData->setFileServer(virtualFileServer);
        }
    }
    else {
        job->addMessage(ParsingMessageType::Error, 24, "Missing Root Group")
            << "Invalid shot: the root event group is missing.";
        job->markCancelled();
    }

    //Create dependency tree
    auto tree = std::make_shared<EventEngineDependencyTree>();
    tree->addVertex(localDeviceID);     //begin tree with parse job owner
    
    std::set<DeviceID> missingTargets;
    std::vector<EngineParsingMessage> messages;

    // Begin multi-pass search. Keep calling while new missingTargets are found.
    if (localDependencyParser != 0) {
        localDependencyParser->getDependants(eventTargets, *tree, missingTargets, messages, 5);  //max 5 passes
    }

    for (auto& m : messages) {
        job->addMessage(m);
    }

    //Check for targets missing from original event target list
    std::set<DeviceID> resolvedTargets;
    tree->getNodes(resolvedTargets);

    std::set<DeviceID> diff;
    std::set_difference(eventTargets.begin(), eventTargets.end(), 
                        resolvedTargets.begin(), resolvedTargets.end(),
                        std::inserter(diff, diff.end()));

    if (diff.size() > 0) {
        //missing targets...
        //Warning: will attempt parse but cannot play
        auto& m = job->addMessage(ParsingMessageType::Warning, 1000, "Missing Targets")
            << "Some required event targets could not be found. "
            << "Parsing is proceeding as an abstract shot. Playing will not be possible. "
            << "Missing targets: \n";
        for (auto& id : diff) {
            m << "    " << id.getID() << "\n";
        }
    }

    //check for circular dependencies
    std::vector<DeviceID> orderedNodes;
    bool isDAG = tree->sortTree(orderedNodes);
    
    if (!isDAG) {
        //Error: Circular dependency loop detected. The device network must be a directed acyclic graph (DAG).
        auto& err = job->addMessage(ParsingMessageType::Error, 1, "Circular Dependency")
            << "Circular dependency detected. The event dependency network must be a directed acyclic graph (DAG). \n";
        //The following devices specify event targets that form the illegal loop:
        
        std::vector<DeviceID> cycle;
        if(tree->getCycle(cycle)) {
            //Throw error with cycle
            err << "The following devices form an illegal event dependency loop:\n";

            for (auto& id : cycle) {
                err << "\t" << id.getID() << "\n";
            }
        }
        job->markCancelled();
        //return; //abort
    }

    job->setDependencies(tree);
    job->setMissingTargets(diff);

    //addJob(job);
}


PlayJobStatus LocalEventEngineScheduler::play(const ParseID& parseID, const EngineJobSourceID& source)
{
    PlayJobStatus playJobStatus;
    playJobStatus.sid = ShotID::generateUniqueID(parseID, source);
    playJobStatus.status = EngineJobStatus::New;

    std::shared_ptr<SequenceJob> seqJob;
    if (parseID.shotType == ShotType::SequenceEntry) {
        if (!findJob(parseID.sequenceEntryID.seqID, seqJob)) {
            //Error: SequenceID not found
            playJobStatus.status = EngineJobStatus::NotFound;
            return playJobStatus;
        }
        seqJob->sequenceResult->addShotResult(parseID.sequenceEntryID.seqIndex, playJobStatus.sid, playJobStatus.status);
    }

    std::shared_ptr<EventEngineJob> parseJob;
    std::shared_ptr<Shot> shot;
    std::shared_ptr<EventEngineDependencyTree> tree;
    
    if (parseID.shotType == ShotType::Single 
        || parseID.shotType == ShotType::SequenceEntry 
        || parseID.shotType == ShotType::SingleUndocumented) {
        
            if (!findJob(parseID, parseJob)) {
            //Error: ParseID not found
            playJobStatus.status = EngineJobStatus::NotFound;
            return playJobStatus;
        }

        parseJob->getShot(shot);
        parseJob->getDependencies(tree);
    }

    // play(playJobStatus.sid, shot);

    //Make play job
    EngineJobID jobID(playJobStatus.sid);
    auto job = std::make_shared<LocalEventEngineJob>(jobID, shot, localDeviceID);
    job->setDependencies(tree);

    addJob(job);

    return playJobStatus;
}

// void LocalEventEngineScheduler::play(const ShotID& shotID, const std::shared_ptr<Shot>& shot)
// {
//     //Make play job
//     EngineJobID jobID(shotID);
//     auto job = std::make_shared<LocalEventEngineJob>(jobID, shot, localDeviceID);

//     addJob(job);
// }


AddSequenceStatus LocalEventEngineScheduler::addSequence(const std::shared_ptr<Sequence>& sequence, const EngineJobSourceID& source)
{
    AddSequenceStatus addSequenceStatus;
    addSequenceStatus.seqid = SequenceID::generateUniqueID(source);

    if (sequence == 0) {
        addSequenceStatus.status = EngineJobStatus::Canceled;
        return addSequenceStatus;
    }
    addSequenceStatus.status = EngineJobStatus::New;

    sequence->shotConfig.shotType = ShotType::Sequence;
    sequence->shotConfig.jobSourceID = source;

    auto result = std::make_shared<SequenceResult>(addSequenceStatus.seqid, sequence);

    if (persistenceManager != 0) {
        persistenceManager->addSequence(result);
    }

    //Make sequence job
    EngineJobID jobID;
    jobID.seqid = addSequenceStatus.seqid;
    jobID.type = EventEngineJobType::Sequence;

    auto job = std::make_shared<SequenceJob>(jobID, localDeviceID, sequence, result);
    addSequenceJob(job);

    return addSequenceStatus;
}


void LocalEventEngineScheduler::addSequenceJob(const std::shared_ptr<SequenceJob>& job)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);

    if (job != 0) {
        queuedSequenceJobs.add(job->jobID, job);

        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toQueuedList(job);
        sendMessage(message);
    }

    jobCondition.notify_all();
}

void LocalEventEngineScheduler::closeSequence(const SequenceID& seqid)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);

    if (currentSequenceJob != 0 && currentSequenceJob->jobID.seqid == seqid) {
        //close current sequence job
        currentSequenceJob->close();

        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toRunningList(currentSequenceJob);
        sendMessage(message);        
    }
    else {
        //attempt to find sequence job in queue and close it
        std::set<EngineJobID> allSequenceJobsIDs;
        queuedSequenceJobs.getKeys(allSequenceJobsIDs);
        std::shared_ptr<SequenceJob> job;
        
        for (auto& jobID : allSequenceJobsIDs) {
            if (jobID.seqid == seqid && queuedSequenceJobs.get(jobID, job) && job != 0) {
                //found sequence job
                job->close();

                auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
                message->toCompleteList(job);
                sendMessage(message);      
                
                break;
            }
        }
    }

    jobCondition.notify_all();
}

void LocalEventEngineScheduler::cancelSequence(const SequenceID& seqid)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);

    if (currentSequenceJob != 0 && currentSequenceJob->jobID.seqid == seqid) {
        //cancel current sequence job
        currentSequenceJob->cancel();

        //cancel all running jobs
        std::set<EngineJobID> jobIDs;
        runningJobs.getKeys(jobIDs);
        for (auto& jobID : jobIDs) {
            if (jobID.seqid == seqid) {
                _cancelJob(jobID);
            }
        }

        //Message: Job complete
        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toCompleteList(currentSequenceJob);
        sendMessage(message);
    }
    else {
        //attempt to find sequence job in queue and cancel it
        std::set<EngineJobID> allSequenceJobsIDs;
        queuedSequenceJobs.getKeys(allSequenceJobsIDs);
        std::shared_ptr<SequenceJob> job;
        
        for (auto& jobID : allSequenceJobsIDs) {
            if (jobID.seqid == seqid && queuedSequenceJobs.get(jobID, job) && job != 0) {
                //found sequence job
                job->cancel();

                //Message: Job complete
                auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
                message->toCompleteList(job);
                sendMessage(message);

                break;
            }
        }
    }

    jobCondition.notify_all();
}

ParseJobStatus LocalEventEngineScheduler::parse(const std::shared_ptr<Shot>& shot, const SequenceID& sequenceID)
{
    SequenceEntryID sequenceEntryID;
    sequenceEntryID.seqID = sequenceID;
    std::shared_ptr<SequenceJob> seqJob;

    if (!findJob(sequenceID, seqJob)) {
        ParseJobStatus parseJobStatus;
        parseJobStatus.status = EngineJobStatus::NotFound;
        return parseJobStatus;
    }

    sequenceEntryID.seqIndex = seqJob->sequenceResult->append(EngineJobStatus::New);

    return parse(shot, sequenceEntryID);
}

ParseJobStatus LocalEventEngineScheduler::parse(const std::shared_ptr<Shot>& shot, const SequenceEntryID& sequenceEntryID)
{
    ParseJobStatus parseJobStatus;
    parseJobStatus.status = EngineJobStatus::New;
    parseJobStatus.pid = ParseID::generateUniqueID(shot->getShotConfig().jobSourceID, sequenceEntryID);

    // if (shot != 0) {
    //     parseJobStatus.pid.shotConfig = shot->getShotConfig();
    // }

    // parseJobStatus.pid.shotType = ShotType::Sequence;
    // parseJobStatus.pid.sequenceEntryID = sequenceEntryID;

    auto job = std::make_shared<LocalEventEngineJob>(parseJobStatus.pid, shot, localDeviceID);

    std::shared_ptr<SequenceResult> sequenceResult;
    if (persistenceManager != 0 && persistenceManager->getSequenceResult(sequenceEntryID.seqID, sequenceResult)) {
        //sequence found

        if (sequenceResult->sequence->type == STI::Engine::SequenceType::Closed && 
            sequenceResult->sequence->sequenceTable.count(sequenceEntryID.seqIndex) == 0) {
            //Error: Sequence entry not found in sequence table
            
            job->addMessage(ParsingMessageType::Error, 10, "Invalid sequence entry")
                << "Tried to parse a sequence entry with invalid index: \n"
                << sequenceEntryID.seqIndex.print() << "\n"
                << "This entry does not appear in the shot table for this sequence.\n";
        }
    }
    else {
        //sequence not found
        job->addMessage(ParsingMessageType::Error, 11, "Sequence not found")
            << "Attempted to add shot [" << parseJobStatus.pid.print() << "]"
            << " with shot table index " << sequenceEntryID.seqIndex.print()
            << " to sequence [" << sequenceEntryID.seqID.print() << "].\n"
            << "This sequence does not exist.\n";
    }

    // parseJob(job);
    addJob(job);

    return parseJobStatus;
}

void LocalEventEngineScheduler::addJob(const std::shared_ptr<EventEngineJob>& newJob)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);
        
    if (newJob != 0) {
        queuedJobs.add(newJob->getJobID(), newJob);

        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toQueuedList(newJob);
        sendMessage(message);
    }

    jobCondition.notify_all();
}

void LocalEventEngineScheduler::cancelAll()
{
    // std::unique_lock<std::mutex> jobLock(jobMutex);

    std::set<EngineJobID> ids;
    
    queuedJobs.getKeys(ids);
    for (auto& jobID : ids) {
        _cancelJob(jobID);
    }

    ids.clear();
    
    runningJobs.getKeys(ids);
    for (auto& jobID : ids) {
        _cancelJob(jobID);
    }

    stopAll();
}

void LocalEventEngineScheduler::stopAll()
{
    std::set<EngineID> ids;
    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<LocalEventEngine> engine;
    engineManagers.getKeys(ids);

    for (auto& id : ids) {
        if (engineManagers.get(id, manager) && (manager != 0)) {
            manager->getEngine(engine);
            if (engine != 0) {
                engine->stop();
            }
        }
    }
}

void LocalEventEngineScheduler::clearAll()
{
    std::set<EngineID> ids;
    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<LocalEventEngine> engine;
    engineManagers.getKeys(ids);

    for (auto& id : ids) {
        if (engineManagers.get(id, manager) && (manager != 0)) {
            manager->getEngine(engine);
            if (engine != 0) {
                engine->clear();
            }
        }
    }
}

std::set<EngineJobID> LocalEventEngineScheduler::getJobIDs(const EventEngineJobList& jobListType) const
{
    // std::unique_lock<std::mutex> jobLock(jobMutex);

    std::set<EngineJobID> jobIDs;
    std::set<EngineJobID> seqJobIDs;

    switch (jobListType)
    {
    case EventEngineJobList::Queued:
        queuedJobs.getKeys(jobIDs);
        queuedSequenceJobs.getKeys(seqJobIDs);
        break;
    case EventEngineJobList::Running:
        runningJobs.getKeys(jobIDs);
        if (currentSequenceJob != 0) {
            seqJobIDs.insert(currentSequenceJob->jobID);
        }
        break;
    case EventEngineJobList::Completed:
        completedParseJobs.getKeys(jobIDs);
        {
            std::set<EngineJobID> playJobIDs;
            completedPlayJobs.getKeys(playJobIDs);
            jobIDs.insert(playJobIDs.begin(), playJobIDs.end());
        }
        completedSequenceJobs.getKeys(seqJobIDs);
        break;
    case EventEngineJobList::Archived:
        break;
    default:
        break;
    }
    
    jobIDs.insert(seqJobIDs.begin(), seqJobIDs.end());
    return jobIDs;
}

std::vector<std::shared_ptr<EventEngineJob>> LocalEventEngineScheduler::getJobs(const EventEngineJobList& jobListType) const
{
    std::vector<std::shared_ptr<EventEngineJob>> jobs;

    switch (jobListType)
    {
    case EventEngineJobList::Queued:
        queuedJobs.getValues(jobs);
        break;
    case EventEngineJobList::Running:
        runningJobs.getValues(jobs);
        break;
    case EventEngineJobList::Completed:
        completedParseJobs.getValues(jobs);
        {
            std::vector<std::shared_ptr<EventEngineJob>> playJobs;
            completedPlayJobs.getValues(playJobs);
            jobs.insert(jobs.end(), playJobs.begin(), playJobs.end());
        }
        break;
    case EventEngineJobList::Archived:
        break;
    default:
        break;
    }

    return jobs;
}

bool LocalEventEngineScheduler::getJob(const EngineJobID& id, std::shared_ptr<EventEngineJob>& job) const
{
    return findJob(id.pid, job);
}

void LocalEventEngineScheduler::cancelJob(const EngineJobID& jobID)
{
    // std::unique_lock<std::mutex> jobLock(jobMutex);
    _cancelJob(jobID);
}

void LocalEventEngineScheduler::_cancelJob(const EngineJobID& jobID)
{
    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<EventEngineJob> archivedJob;
    bool archivedJobValid = false;
    std::shared_ptr<EventEngineManager> manager;

    if (runningJobs.get(jobID, job) && job != 0) {

        if (getManager(job, manager)) {
            manager->abortJob();
        }

        runningJobs.remove(jobID);
        job->markCancelled();

        if (jobID.type == EventEngineJobType::Parse) {
            archivedJobValid = completedParseJobs.addAndRemove(jobID, job, archivedJob);
        }
        else if (jobID.type == EventEngineJobType::Play) {
            archivedJobValid = completedPlayJobs.addAndRemove(jobID, job, archivedJob);
        }
        else if (jobID.type == EventEngineJobType::Sequence) {
            //should not reach here - sequence jobs handled separately
        }
        // archivedJobValid = completedJobs.addAndRemove(jobID, job, archivedJob);

        //Message: Job complete
        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toCompleteList(job);
        sendMessage(message);
    }
    
    if (queuedJobs.get(jobID, job) && job != 0) {
        queuedJobs.remove(jobID);
        job->markCancelled();
        
        if (jobID.type == EventEngineJobType::Parse) {
            archivedJobValid = completedParseJobs.addAndRemove(jobID, job, archivedJob);
        }
        else if (jobID.type == EventEngineJobType::Play) {
            archivedJobValid = completedPlayJobs.addAndRemove(jobID, job, archivedJob);
        }
        else if (jobID.type == EventEngineJobType::Sequence) {
            //should not reach here - sequence jobs handled separately
        }
        // archivedJobValid = completedJobs.addAndRemove(jobID, job, archivedJob);

        //Message: Job complete
        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toCompleteList(job);
        sendMessage(message);
    }

    if (archivedJobValid) {
        archivedJob->markArchived();
        //Message: Job archived
        auto message2 = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message2->toArchive(archivedJob);
        sendMessage(message2);
    }

    jobCondition.notify_all();
}


std::shared_ptr<Shot> LocalEventEngineScheduler::createShot(const ShotConfig& shotConfig, const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup)
{
    auto shot = std::make_shared<LocalShot>(shotConfig, eventGroup);
    return shot;
}

// 1) Filter jobs that are part of the running sequence 
// 2) If there are none, wait for 100 ms after any of the running sequence's jobs completes
// 3) If there are still no jobs for the running sequence, but there are other jobs, yield priority

void LocalEventEngineScheduler::jobComplete(const EngineJobID& jobID)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);

    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<EventEngineJob> archivedJob;
    
    if (runningJobs.get(jobID, job) && job != 0) {
        runningJobs.remove(jobID);
        job->markComplete();
        
        bool valid = false;
        if (jobID.type == EventEngineJobType::Parse) {
            valid = completedParseJobs.addAndRemove(jobID, job, archivedJob);
        }
        else if (jobID.type == EventEngineJobType::Play) {
            valid = completedPlayJobs.addAndRemove(jobID, job, archivedJob);
        }
        else if (jobID.type == EventEngineJobType::Sequence) {
            //should not reach here - sequence jobs handled separately
        }

        //Message: Job complete
        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toCompleteList(job);
        sendMessage(message);

        if (valid) {
            archivedJob->markArchived();
            //Message: Job archived
            auto message2 = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
            message2->toArchive(archivedJob);
            sendMessage(message2);
        }
    }

    jobCondition.notify_all();
}

void LocalEventEngineScheduler::refreshSequenceJobs()
{
    bool done = false;
    
    if (currentSequenceJob == 0) {
        done = true;
    }

    if (currentSequenceJob != 0 && currentSequenceJob->isDone()) {
        currentSequenceJob->runningJobs.clear();

        //remove current sequence job
        completedSequenceJobs.add(currentSequenceJob->jobID, currentSequenceJob);
        currentSequenceJob = 0;
        done = true;

        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toCompleteList(currentSequenceJob);
        sendMessage(message);
    }

    if (!done) return;

    std::set<EngineJobID> jobIDs;
    queuedSequenceJobs.getKeys(jobIDs);

    // If there are sequence jobs in the queue, get the first one
    // and remove it from the queue.
    if (jobIDs.size() > 0) {
        queuedSequenceJobs.get(*jobIDs.begin(), currentSequenceJob);
        
        if (currentSequenceJob == 0) {
            return; //no sequence job found
        }

        queuedSequenceJobs.remove(currentSequenceJob->jobID);

        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toRunningList(currentSequenceJob);
        sendMessage(message);
    }
}

void LocalEventEngineScheduler::assignPlayJobs(const std::set<EngineJobID>& queuedJobIDs, std::set<EngineID>& freeEngines)
{
    EngineID engineID;

    for (auto& jobID : queuedJobIDs) {
        if (jobID.type != EventEngineJobType::Play) {
            continue; //only process play jobs
        }

        //check if the associated parse job was canceled
        if (isCanceledJob(jobID.pid)) {
            _cancelJob(jobID);  //cancel play if parse was canceled
            continue;
        }

        if (!findParsedEngine(jobID.pid, freeEngines, engineID)) {
            EngineJobID parseJobID;
            parseJobID.type = EventEngineJobType::Parse;
            parseJobID.pid = jobID.pid;

            if (queuedJobs.contains(parseJobID) || runningJobs.contains(parseJobID)) {
                continue; //defer play while parse is queued or running
            }

            _cancelJob(jobID);  //cancel play if no parsed engine available and no pending parse
            continue;
        }
    
        if (assignJob(jobID, engineID)) {
            //play job assigned to engineID
            freeEngines.erase(engineID);

            //if current sequence job is running, add job to sequence
            if (currentSequenceJob != 0 && currentSequenceJob->isMemberOfSequence(jobID)) {
                std::shared_ptr<EventEngineJob> job;
                runningJobs.get(jobID, job);
                currentSequenceJob->runningJobs.add(engineID, job);
            }
        }
    }
}

int LocalEventEngineScheduler::getTargetPool(const EngineJobID& jobID)
{
    // This function determines the target engine pool based on the job ID.
    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<Shot> shot;
    
    if (queuedJobs.get(jobID, job) && job != 0 && job->getShot(shot) && shot != 0) {
        return shot->getShotConfig().targetEnginePool;
    }
    return 1;   //common sync pool by default 
}

void LocalEventEngineScheduler::assignParseJobs(const std::set<EngineJobID>& queuedJobIDs, std::set<EngineID>& freeEngines)
{
    EngineID engineID;

    for (auto& jobID : queuedJobIDs) {
        if (jobID.type != EventEngineJobType::Parse) {
            continue; //only process parse jobs
        }
        
        int targetEnginePool = getTargetPool(jobID);

        if (targetEnginePool == 0) {
            //async engine pool, assign engineID #0 if available
            EngineID asyncEngineID(0);
            auto it = freeEngines.find(asyncEngineID);
            
            if (it != freeEngines.end() && assignJob(jobID, *it)) {
                freeEngines.erase(it);
            }
            continue; //async engine unavailable
        }

        if (findOldestParsedEngine(freeEngines, engineID) && assignJob(jobID, engineID)) {
            freeEngines.erase(engineID);
            
            //if current sequence job is running, add job to sequence
            if (currentSequenceJob != 0 && currentSequenceJob->isMemberOfSequence(jobID)) {
                std::shared_ptr<EventEngineJob> job;
                runningJobs.get(jobID, job);
                currentSequenceJob->runningJobs.add(engineID, job);
            }
        }
    }
}

void LocalEventEngineScheduler::assignJobs()
{
    std::unique_lock<std::mutex> jobLock(jobMutex);

    std::set<EngineID> allEngines;
    std::set<EngineID> freeEngines;
    std::set<EngineJobID> queuedJobIDs;     //sorted by priority

    std::set<EngineJobID> filteredJobsIDs;

    std::shared_ptr<EventEngineManager> manager;
    EngineID engineID;

    if (currentSequenceJob != 0) {
        currentSequenceJob->jobID;
    }

    int assignableJobCount;
    int freeEngineCount;

    while (running) {
        engineManagers.getKeys(allEngines);
        queuedJobs.getKeys(queuedJobIDs);
        freeEngines.clear();

        //check for available engines
        for (auto& id : allEngines) {
            if (engineManagers.get(id, manager) && manager != 0 && !manager->jobRunning()) {
                freeEngines.insert(id);
            }
        }

        // Intial counts before attempting assignment
        freeEngineCount = freeEngines.size();
        assignableJobCount = queuedJobs.size();

        if (freeEngineCount > 0 && assignableJobCount > 0) {
            
            //First priority is to run a play event if a free engine is parsed for it, regardless of position in set.
            assignPlayJobs(queuedJobIDs, freeEngines);

            //Next priority is to start any new sequence job
            refreshSequenceJobs();
            
            std::set<EngineJobID>* queuedParsedJobIDs;

            if (currentSequenceJob !=0 && 
                !currentSequenceJob->isDone() && 
                currentSequenceJob->hasPriority(queuedJobIDs)) {
                //if current sequence has priority, filter jobs for just the sequence
                currentSequenceJob->filter(queuedJobIDs, filteredJobsIDs);
                queuedParsedJobIDs = &filteredJobsIDs;
            }
            else {
                queuedParsedJobIDs = &queuedJobIDs;
            }

            //Finally, assign parse jobs to remaining free engines
            assignParseJobs(*queuedParsedJobIDs, freeEngines);
        }

        if (queuedJobs.size() == 0 || freeEngines.size() == 0) {
            jobCondition.wait(jobLock, [this] { return !running || queuedJobs.size() > 0; } );
        }
        else if (assignableJobCount == queuedJobs.size() && freeEngineCount == freeEngines.size()) {
            //no assignments made, wait for job completion or new job
            //100 ms timeout to recheck
            jobCondition.wait_for(jobLock, std::chrono::milliseconds(100));
        }
    }

}


bool LocalEventEngineScheduler::isCanceledJob(const STI::Engine::ParseID& parseID)
{
    EngineJobID jobID;
    jobID.type = EventEngineJobType::Parse;
    jobID.pid = parseID;

    std::shared_ptr<EventEngineJob> job;
    bool success = false;

    if (jobID.type == EventEngineJobType::Parse) {
        success = completedParseJobs.get(jobID, job) && job != 0;
    }
    else if (jobID.type == EventEngineJobType::Play) {
        success = completedPlayJobs.get(jobID, job) && job != 0;
    }
    else if (jobID.type == EventEngineJobType::Sequence) {
        //should not reach here - sequence jobs handled separately
    }

    return (success && job->getStatus() == EngineJobStatus::Canceled);
}

bool LocalEventEngineScheduler::satisfiesConflictPolicy(const std::shared_ptr<EventEngineJob>& runningJob, 
                                                        const EventEngineJobType& newJobType,
                                                        const EngineID& engineID)
{
    if (runningJob == 0) return true;
    if (conflictPolicy == 0) return true;

    if (runningJob->getJobID().type == EventEngineJobType::Parse) {
        
        if (newJobType == EventEngineJobType::Parse) {
            //Parse vs Parse
            return conflictPolicy->parseWhenParsing(runningJob->getEngineID(), engineID);
        }
        else if (newJobType == EventEngineJobType::Play) {
            //Parse vs Play
            return conflictPolicy->playWhenParsing(runningJob->getEngineID(), engineID);
        }
    }
    else if (runningJob->getJobID().type == EventEngineJobType::Play) {
        
        if (newJobType == EventEngineJobType::Parse) {
            //Play vs Parse
            return conflictPolicy->parseWhenPlaying(runningJob->getEngineID(), engineID);
        }
        else if (newJobType == EventEngineJobType::Play) {
            //Play vs Play (this should never happen as only one play job per engine is allowed)
            return false; //cannot play when playing
        }
    }
    return true;
}

void LocalEventEngineScheduler::unloadEngines(const EventEngineJobType& type, const EngineID& engineID)
{
    //unload other engines according to policy, based on job type running on engineID

    std::shared_ptr<EventEngineManager> manager;
    std::set<EngineID> engineIDs;
    engineManagers.getKeys(engineIDs);

    for (auto& id : engineIDs) {
        if (id == engineID) continue;
        
        if (type == EventEngineJobType::Parse && conflictPolicy->unloadAfterParsing(engineID, id)) {
            // engineID is parsing, unload other engines as per policy
            if (engineManagers.get(id, manager) && manager != 0) {
                manager->unloadEngine();
            }
        }
        else if (type == EventEngineJobType::Play && conflictPolicy->unloadAfterPlaying(engineID, id)) {
            // engineID is playing, unload other engines as per policy
            if (engineManagers.get(id, manager) && manager != 0) {
                manager->unloadEngine();
            }
        }
    }
}

bool LocalEventEngineScheduler::assignJob(const EngineJobID& jobID, const EngineID& engineID)
{
    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<EventEngineJob> job;

    bool freeEngineCheck = engineManagers.get(engineID, manager) && manager != 0 && !manager->jobRunning();
    
    if (!freeEngineCheck) return false;

    std::vector<std::shared_ptr<EventEngineJob>> runningJobList;
    runningJobs.getValues(runningJobList);

    for (auto& job : runningJobList) {
      if (!satisfiesConflictPolicy(job, jobID.type, engineID)) {
          return false;
      }
    }

    switch (jobID.type)
    {
    case EventEngineJobType::Parse:
        if (manager->isParsed(jobID.pid)) {
            //already parsed on this engine
            return false;
        }
        break;
    case EventEngineJobType::Play:        
        if (!manager->isParsed(jobID.pid)) {
            //not parsed on this engine
            return false;
        }
        break;
    }

    if (!queuedJobs.get(jobID, job)) return false;
    if (job == nullptr) return false;

    if (manager->submitJob(job)) {
        
        queuedJobs.remove(jobID);
        runningJobs.add(jobID, job);

        //Message: Job running
        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toRunningList(job);
        sendMessage(message);

        //unload other engines according to policy
        unloadEngines(jobID.type, engineID);
       
        return true;
    }

    return false;
}


bool LocalEventEngineScheduler::findParsedEngine(const STI::Engine::ParseID& parseID, 
                                                 std::set<EngineID>& freeEngines, EngineID& engineID)
{
    std::shared_ptr<EventEngineManager> manager;
    bool found = false;

    for (auto& id : freeEngines) {
        if (engineManagers.get(id, manager) && manager != 0 && manager->isParsed(parseID)) {
            engineID = id;
            found = true;
            break;
        }
    }

    return found;
}

bool LocalEventEngineScheduler::findOldestParsedEngine(std::set<EngineID>& freeEngines, EngineID& engineID)
{
    std::shared_ptr<EventEngineManager> manager;
    STI::Utils::TimeStamp oldest;

    bool found = false;

    for (auto& id : freeEngines) {
        if (id.getNumber() == 0) continue; //skip async engine ID

        if (engineManagers.get(id, manager) && manager != 0) {
 
            //first time through, found == false, so we initialize with the first timestamp
            if (!found || manager->getLastParseID().parseTimestamp < oldest) {
                oldest = manager->getLastParseID().parseTimestamp;
                engineID = id;
                found = true;
            }
        }
    }

    return found;
}

void LocalEventEngineScheduler::handleMessage(const std::shared_ptr<EngineSchedulerMessage>& mess)
{
    typedef EngineSchedulerMessage::SchedulerMessageType MessageType;

    //ParseComplete, YieldParse, PartialParse, PlayReady, YieldPlay
    
    std::shared_ptr<EventEngineManager> manager;

    switch(mess->schedulerMessageType) {
        case MessageType::ParseComplete:
            if (getManager(mess->jobID, manager)) {
                manager->handleParseMessage(mess);
            }
        break;

        case MessageType::YieldParse:
        break;
        
        case MessageType::PlayReady:
            if (getManager(mess->jobID, manager)) {
                manager->handlePlayReadyMessage(mess);
            }
        break;
        
        case MessageType::PlayComplete:
            if (getManager(mess->jobID, manager)) {
                manager->handlePlayCompleteMessage(mess);
            }
        break;
    }
}

bool LocalEventEngineScheduler::getManager(const EngineJobID& jobID, std::shared_ptr<EventEngineManager>& manager)
{
    std::shared_ptr<EventEngineJob> job;

    // if (runningJobs.get(jobID, job) && job != 0
    //     && engineManagers.get(job->getEngineID(), manager) && manager != 0) {
    //     return true;
    // }

    if (runningJobs.get(jobID, job) && job != 0) {
        return getManager(job, manager);
    }

    return false;
}

bool LocalEventEngineScheduler::getManager(const std::shared_ptr<EventEngineJob>& job, std::shared_ptr<EventEngineManager>& manager)
{
    if (job != 0 && engineManagers.get(job->getEngineID(), manager) && manager != 0) {
        return true;
    }

    return false;
}


bool LocalEventEngineScheduler::findJob(const ParseID& parseID, std::shared_ptr<EventEngineJob>& job) const
{
    // std::unique_lock<std::mutex> jobLock(jobMutex);
    
    EngineJobID jobID;
    jobID.type = EventEngineJobType::Parse;
    jobID.pid = parseID;

    bool success = completedParseJobs.get(jobID, job);

    if (!success) {
        success = runningJobs.get(jobID, job);
    }
    if (!success) {
        success = queuedJobs.get(jobID, job);
    }

    return success && (job != 0);
}

bool LocalEventEngineScheduler::findJob(const ShotID& shotID, std::shared_ptr<EventEngineJob>& job) const
{
    // std::unique_lock<std::mutex> jobLock(jobMutex);
    
    EngineJobID jobID;
    jobID.type = EventEngineJobType::Play;
    jobID.sid = shotID;
    jobID.pid = shotID.parseID;

    bool success = completedPlayJobs.get(jobID, job);

    if (!success) {
        success = runningJobs.get(jobID, job);
    }
    if (!success) {
        success = queuedJobs.get(jobID, job);
    }

    return success && (job != 0);
}

bool LocalEventEngineScheduler::findJob(const SequenceID& seqID, std::shared_ptr<SequenceJob>& job) const
{
    std::unique_lock<std::mutex> jobLock(jobMutex);
    
    EngineJobID jobID;
    jobID.type = EventEngineJobType::Sequence;
    jobID.seqid = seqID;

    bool success = completedSequenceJobs.get(jobID, job);

    if (!success && currentSequenceJob != 0 && currentSequenceJob->jobID.seqid == seqID) {
        job = currentSequenceJob;
        success = true;
    }
    if (!success) {
        success = queuedSequenceJobs.get(jobID, job);
    }

    return success && (job != 0);
}

bool LocalEventEngineScheduler::getParsedEngine(const ParseID& parseID, std::shared_ptr<LocalEventEngine>& engine) const
{
    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<EventEngineManager> manager;

    EngineJobID jobID;
    jobID.type = EventEngineJobType::Parse;
    jobID.pid = parseID;

    if (completedParseJobs.get(jobID, job) && job !=0 
        && engineManagers.get(job->getEngineID(), manager) && manager != 0) 
    {        
        manager->getEngine(engine);   
        return engine != 0 && engine->getLastParseID() == parseID;
    }

    return false;
}

bool LocalEventEngineScheduler::getParseResult(const ParseID& parseID, std::shared_ptr<ParseResult>& parseResult) const
{
    if (searchingParseResult) return false;

    std::unique_lock<std::mutex> resultLock(parseResultMutex);
    searchingParseResult = true;

    std::shared_ptr<LocalEventEngine> engine;

    bool success = false;

    if (getParsedEngine(parseID, engine)) {
        success = engine->getParseResult(parseID, parseResult);
    }
    else {
        //ParseResult is not in engine anymore; check for result in PersistenceManager
        success = persistenceManager != 0 && persistenceManager->getParseResult(parseID, parseResult);
    }

    searchingParseResult = false;
    return success;
}

bool LocalEventEngineScheduler::getShotResult(const ShotID& shotID, std::shared_ptr<ShotResult>& shotResult) const
{
    if (searchingShotResult) return false;

    std::unique_lock<std::mutex> resultLock(shotResultMutex);
    searchingShotResult = true;

    std::shared_ptr<LocalEventEngine> engine;

    bool success = false;

    if (findRunningEngine(shotID, engine) || findCompletedEngine(shotID, engine)) {
        success = engine->getShotResult(shotID, shotResult);
    }

    if (!success) {
        success = persistenceManager != 0 && persistenceManager->getShotResult(shotID, shotResult);
    }

    searchingShotResult = false;
    return success;
}


bool LocalEventEngineScheduler::findRunningEngine(const ShotID& shotID, std::shared_ptr<LocalEventEngine>& engine) const
{
    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<EventEngineManager> manager;

    EngineJobID jobID;
    jobID.type = EventEngineJobType::Play;
    jobID.sid = shotID;
    jobID.pid = shotID.parseID;

    if (runningJobs.get(jobID, job) && job !=0 
        && engineManagers.get(job->getEngineID(), manager) && manager != 0) 
    {       
        manager->getEngine(engine);   
        return engine != 0;
    }

    return false;
}


bool LocalEventEngineScheduler::findCompletedEngine(const ShotID& shotID, std::shared_ptr<LocalEventEngine>& engine) const
{
    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<EventEngineManager> manager;

    EngineJobID jobID;
    jobID.type = EventEngineJobType::Play;
    jobID.sid = shotID;
    jobID.pid = shotID.parseID;

    if (completedPlayJobs.get(jobID, job) && job !=0 
        && engineManagers.get(job->getEngineID(), manager) && manager != 0) 
    {       
        manager->getEngine(engine);
        return engine != 0;
    }

    return false;
}

void LocalEventEngineScheduler::definePlayMessageIDs()
{
	// playMessageIDs["Missing Channel"] 					= 30;
	// playMessageIDs["Incorrect Output Type"]  			= 31;
}

const std::map<std::string, unsigned>& LocalEventEngineScheduler::getPlayMessageIDs()
{
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        LocalEventEngineScheduler::definePlayMessageIDs();
    });

    return playMessageIDs;
}
