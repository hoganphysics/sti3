#include "LocalEventEngineScheduler.h"

#include <sti/fwd/RawEvent_fwd.h>

#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessage.h>

#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/EventEngineJob.h>
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
#include <sti/engine/SequenceResult.h>
#include <sti/engine/Shot.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/StackTraceData.h>

#include <sti/utils/SynchronizedMap.h>

#include "EventEngineFactory.h"
#include "EventEngineManager.h"
#include "LocalEventEngineDependencyParser.h"
#include "LocalEventEngineFactory.h"
#include "LocalEventEngineJob.h"
#include "LocalShot.h"
#include "VirtualFileHolder.h"
#include <sti/utils/VirtualFileServer.h>


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


LocalEventEngineScheduler::LocalEventEngineScheduler(STI::Device::LocalDevice* localDevice, 
                                                    const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory,
                                                    const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher,
                                                    const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager)
: MessageGenerator(dispatcher), completedJobs(3), persistenceManager(persistenceManager)
{
    completedJobs.setMaxSize(3);

    localDeviceID = localDevice->getID();
    
    localDependencyParser = std::make_shared<LocalEventEngineDependencyParser>(localDevice);

    running = true;
    schedulerThread = std::thread(&LocalEventEngineScheduler::assignJobs, this);

    searchingParseResult = false;

    setEngineFactory(engineFactory);

    engineSchedulerMessageListenerDelegate = std::make_shared<LocalEventEngineScheduler::EngineSchedulerMessageListenerDelegate>(this);
}

LocalEventEngineScheduler::~LocalEventEngineScheduler()
{
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
    std::unique_lock<std::mutex> jobLock(jobMutex);
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
            
            addEngine(id, engine->getDeviceParser());  //replaces existing engine with newly created engine (using new factory)
        }
    }
}

void LocalEventEngineScheduler::addEngine(const EngineID& engineID, DeviceEventParser* deviceParser)
{
    if (eventEngineFactory != 0) {
 
        std::shared_ptr<LocalEventEngine> engine = eventEngineFactory->createEngine(engineID, deviceParser);

        auto manager = std::make_shared<EventEngineManager>(engineID, engine, this);

        engineManagers.add(engineID, manager);        
    }
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
    ParseJobStatus parseJobStatus;
    parseJobStatus.status = EngineJobStatus::New;
    // ParseID parseID;
    
    if (shot != 0) {
        parseJobStatus.pid.shotConfig = shot->getShotConfig();      
    }

    auto job = std::make_shared<LocalEventEngineJob>(parseJobStatus.pid, shot, localDeviceID);

    parse(job);

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

void LocalEventEngineScheduler::parse(const std::shared_ptr<LocalEventEngineJob>& job)
{
    if (job == 0) return;

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

    addJob(job);
}


PlayJobStatus LocalEventEngineScheduler::play(const ParseID& parseID, const EngineJobSourceID& source)
{
    PlayJobStatus playJobStatus;
    playJobStatus.sid = ShotID::generateUniqueID(parseID, source);
    playJobStatus.status = EngineJobStatus::New;

    std::shared_ptr<SequenceResult> sequenceResult;
    if (parseID.shotConfig.shotType == ShotType::Sequence &&
        persistenceManager != 0 && 
        persistenceManager->getSequenceResult(parseID.sequenceEntryID.seqID, sequenceResult)) {
        //sequence found
        // sequenceResult->status[sequenceEntryID.seqIndex.index] = 
        // add sequence message
    }

    play(playJobStatus.sid);

    return playJobStatus;
}

void LocalEventEngineScheduler::play(const ShotID& shotID)
{
    //Make play job
    EngineJobID jobID;
    jobID.pid = shotID.parseID;
    jobID.sid = shotID;
    jobID.type = EventEngineJobType::Play;
    auto job = std::make_shared<LocalEventEngineJob>(jobID, localDeviceID);

    addJob(job);
}


AddSequenceStatus LocalEventEngineScheduler::addSequence(const std::shared_ptr<Sequence>& sequence, const EngineJobSourceID& source)
{
    AddSequenceStatus addSequenceStatus;
    addSequenceStatus.seqid = SequenceID::generateUniqueID(source);
    addSequenceStatus.status = EngineJobStatus::New;

    auto result = std::make_shared<SequenceResult>(addSequenceStatus.seqid, sequence);

    if (persistenceManager != 0) {
        persistenceManager->addSequence(result);
    }

    return addSequenceStatus;
}

ParseJobStatus LocalEventEngineScheduler::parse(const std::shared_ptr<Shot>& shot, const SequenceEntryID& sequenceEntryID)
{
    ParseJobStatus parseJobStatus;
    parseJobStatus.status = EngineJobStatus::New;

    if (shot != 0) {
        parseJobStatus.pid.shotConfig = shot->getShotConfig();
    }

    parseJobStatus.pid.shotConfig.shotType = ShotType::Sequence;
    parseJobStatus.pid.sequenceEntryID = sequenceEntryID;

    auto job = std::make_shared<LocalEventEngineJob>(parseJobStatus.pid, shot, localDeviceID);

    std::shared_ptr<SequenceResult> sequenceResult;
    if (persistenceManager != 0 && persistenceManager->getSequenceResult(sequenceEntryID.seqID, sequenceResult)) {
        //sequence found

        if (sequenceResult->sequence->type == STI::Engine::SequenceType::Closed && 
            sequenceResult->sequence->sequenceTable.count(sequenceEntryID.seqIndex.index) == 0) {
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

    parse(job);

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
    std::unique_lock<std::mutex> jobLock(jobMutex);

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

std::set<EngineJobID> LocalEventEngineScheduler::getJobIDs(const EventEngineJobList& jobListType) const
{
    std::set<EngineJobID> jobIDs;

    switch (jobListType)
    {
    case EventEngineJobList::Queued:
        queuedJobs.getKeys(jobIDs);
        break;
    case EventEngineJobList::Running:
        runningJobs.getKeys(jobIDs);
        break;
    case EventEngineJobList::Completed:
        completedJobs.getKeys(jobIDs);
        break;
    case EventEngineJobList::Archived:
        break;
    default:
        break;
    }
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
        completedJobs.getValues(jobs);
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
    std::unique_lock<std::mutex> jobLock(jobMutex);
    _cancelJob(jobID);
}

void LocalEventEngineScheduler::_cancelJob(const EngineJobID& jobID)
{
    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<EventEngineJob> archivedJob;
    bool archivedJobValid = false;
    std::shared_ptr<EventEngineManager> manager;

    if (runningJobs.get(jobID, job) && job != 0) {

        if (getManager(jobID, manager)) {
            manager->abortJob();
        }

        runningJobs.remove(jobID);
        job->markCancelled();
        
        archivedJobValid = completedJobs.addAndRemove(jobID, job, archivedJob);

        //Message: Job complete
        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toCompleteList(job);
        sendMessage(message);
    }
    
    if (queuedJobs.get(jobID, job) && job != 0) {
        queuedJobs.remove(jobID);
        job->markCancelled();
        
        archivedJobValid = completedJobs.addAndRemove(jobID, job, archivedJob);

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


void LocalEventEngineScheduler::jobComplete(const EngineJobID& jobID)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);

    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<EventEngineJob> archivedJob;
    
    if (runningJobs.get(jobID, job) && job != 0) {
        runningJobs.remove(jobID);
        job->markComplete();
        bool valid = completedJobs.addAndRemove(jobID, job, archivedJob);

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

void LocalEventEngineScheduler::assignJobs()
{
    std::unique_lock<std::mutex> jobLock(jobMutex);

    std::set<EngineID> allEngines;
    std::set<EngineID> freeEngines;
    std::set<EngineJobID> queuedJobIDs;     //sorted by priority
  
    std::shared_ptr<EventEngineManager> manager;
    EngineID engineID;

    int assignableJobCount;

    do {
        engineManagers.getKeys(allEngines);
        queuedJobs.getKeys(queuedJobIDs);
        freeEngines.clear();

        //check for available engines
        for (auto& id : allEngines) {
            if (engineManagers.get(id, manager) && manager != 0 && !manager->jobRunning()) {
                freeEngines.insert(id);
            }
        }
        assignableJobCount = static_cast<int>(queuedJobIDs.size());

        if (freeEngines.size() > 0) {
            
            //First priority is to run a play event if a free engine is parsed for it, regardless of position in set.
            for (auto& jobID : queuedJobIDs) {
                
                if (jobID.type == EventEngineJobType::Play) {
                    
                    //check if the associated parse job was canceled
                    if (isCanceledJob(jobID.pid)) {
                        _cancelJob(jobID);  //cancel play if parse was canceled
                    }
                
                    if (findParsedEngine(jobID.pid, freeEngines, engineID) && assignJob(jobID, engineID)) {
                        //play job assigned to engineID
                        freeEngines.erase(engineID);
                    }
                    else {
                        assignableJobCount--;
                    }
                }
            }

            //assign parse jobs
            for (auto& jobID : queuedJobIDs) {
                if (jobID.type == EventEngineJobType::Parse) {

                    if (findOldestParsedEngine(freeEngines, engineID) && assignJob(jobID, engineID)) {
                        freeEngines.erase(engineID);
                    }
                    else {
                        assignableJobCount--;
                    }
                }
            }
        }

        if (queuedJobs.size() == 0 || freeEngines.size() == 0 || assignableJobCount < 1) {
            jobCondition.wait(jobLock);
        }

    } while (running);

}

bool LocalEventEngineScheduler::isCanceledJob(const STI::Engine::ParseID& parseID)
{
    EngineJobID jobID;
    jobID.type = EventEngineJobType::Parse;
    jobID.pid = parseID;

    std::shared_ptr<EventEngineJob> job;
    bool success = completedJobs.get(jobID, job) && job != 0;

    return (success && job->getStatus() == EngineJobStatus::Canceled);
}

bool LocalEventEngineScheduler::assignJob(const EngineJobID& jobID, const EngineID& engineID)
{
    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<EventEngineJob> job;

    bool freeEngineCheck = engineManagers.get(engineID, manager) && manager != 0 && !manager->jobRunning();
    
    if (!freeEngineCheck) return false;
    
    if (queuedJobs.get(jobID, job) && job != 0 && manager->submitJob(job)) {
        
        queuedJobs.remove(jobID);
        runningJobs.add(jobID, job);

        //Message: Job running
        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toRunningList(job);
        sendMessage(message);
       
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

    if (runningJobs.get(jobID, job) && job != 0
        && engineManagers.get(job->getEngineID(), manager) && manager != 0) {
        return true;
    }

    return false;
}

bool LocalEventEngineScheduler::findJob(const ParseID& parseID, std::shared_ptr<EventEngineJob>& job) const
{
    std::unique_lock<std::mutex> jobLock(jobMutex);
    
    EngineJobID jobID;
    jobID.type = EventEngineJobType::Parse;
    jobID.pid = parseID;

    bool success = completedJobs.get(jobID, job);

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
    std::unique_lock<std::mutex> jobLock(jobMutex);
    
    EngineJobID jobID;
    jobID.type = EventEngineJobType::Play;
    jobID.sid = shotID;
    jobID.pid = shotID.parseID;

    bool success = completedJobs.get(jobID, job);

    if (!success) {
        success = runningJobs.get(jobID, job);
    }
    if (!success) {
        success = queuedJobs.get(jobID, job);
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

    if (completedJobs.get(jobID, job) && job !=0 
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

    if (completedJobs.get(jobID, job) && job !=0 
        && engineManagers.get(job->getEngineID(), manager) && manager != 0) 
    {       
        manager->getEngine(engine);
        return engine != 0;
    }

    return false;
}

