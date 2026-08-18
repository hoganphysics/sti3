#include "LocalEventEngineScheduler.h"

#include <sti/fwd/RawEvent_fwd.h>

#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/PostProcessingManager.h>
#include <sti/engine/PostProcessRequest.h>

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

#include <sti/utils/Configuration.h>
#include <sti/utils/SynchronizedMap.h>
#include <sti/utils/VirtualFileHolder.h>
#include <sti/utils/VirtualFileServer.h>

#include "EventEngineDependencyTree.h"
#include "EventEngineFactory.h"
#include "EventEngineManager.h"
#include "LocalEventEngine.h"
#include "LocalEventEngineDependencyParser.h"
#include "LocalEventEngineFactory.h"
#include "LocalEventEngineJob.h"
#include "LocalPersistenceManager.h"
#include "LocalShot.h"



#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <vector>


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
using STI::Engine::ParseResult;
using STI::Device::LocalPersistenceManager;
using STI::Utils::Configuration;
using STI::Utils::normalizeStringForLookup;
using STI::Engine::SequenceSchedulingMode;

std::map<std::string, unsigned> LocalEventEngineScheduler::playMessageIDs;

namespace {

std::chrono::milliseconds getPositiveMilliseconds(
    const Configuration& config,
    const std::string& key,
    std::chrono::milliseconds defaultValue)
{
    auto value = config.get<int>("EngineManager", key, static_cast<int>(defaultValue.count())).get();
    if (value <= 0) {
        return defaultValue;
    }
    return std::chrono::milliseconds(value);
}

//Like getPositiveMilliseconds, but zero is a valid (feature-disabling) value.
std::chrono::milliseconds getNonNegativeMilliseconds(
    const Configuration& config,
    const std::string& key,
    std::chrono::milliseconds defaultValue)
{
    auto value = config.get<int>("EngineManager", key, static_cast<int>(defaultValue.count())).get();
    if (value < 0) {
        return defaultValue;
    }
    return std::chrono::milliseconds(value);
}

SequenceSchedulingMode getConfiguredSequenceSchedulingMode(const Configuration& config)
{
    auto configuredMode = config.get<std::string>(
        "EventScheduler",
        "Sequence Mode",
        LocalEventEngineScheduler::sequenceSchedulingModeToString(SequenceSchedulingMode::Normal)).get();

    SequenceSchedulingMode mode = SequenceSchedulingMode::Normal;
    LocalEventEngineScheduler::parseSequenceSchedulingMode(configuredMode, mode);
    return mode;
}

} // namespace


LocalEventEngineScheduler::LocalEventEngineScheduler(STI::Device::LocalDevice* localDevice, 
                                                    const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory,
                                                    const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher,
                                                    const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager)
: LocalEventEngineScheduler(localDevice, engineFactory, dispatcher, persistenceManager, Configuration())
{
}

LocalEventEngineScheduler::LocalEventEngineScheduler(STI::Device::LocalDevice* localDevice,
                                                    const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory,
                                                    const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher,
                                                    const std::shared_ptr<STI::Device::PersistenceManager>& persistenceManager,
                                                    const Configuration& config)
: MessageGenerator(dispatcher),
completedParseJobs(3),
completedPlayJobs(3),
completedSequenceJobs(3),
persistenceManager(persistenceManager),
ownedDevicePlayReadyTimeout(getPositiveMilliseconds(config, "PlayReady Timeout ms", LocalEventEngine::DefaultOwnedDevicePlayReadyTimeout)),
ownedDeviceTriggerTimeout(getPositiveMilliseconds(config, "Trigger Timeout ms", LocalEventEngine::DefaultOwnedDeviceTriggerTimeout)),
ownedDevicePlayCompleteGrace(getPositiveMilliseconds(config, "PlayComplete Grace ms", LocalEventEngine::DefaultOwnedDevicePlayCompleteGrace)),
ownedDeviceMaxMeasurementGrace(getNonNegativeMilliseconds(config, "Max Measurement Grace ms", LocalEventEngine::DefaultOwnedDeviceMaxMeasurementGrace)),
ownedDeviceMeasurementPollInterval(getPositiveMilliseconds(config, "Measurement Poll ms", LocalEventEngine::DefaultOwnedDeviceMeasurementPollInterval)),
sequenceSchedulingMode(getConfiguredSequenceSchedulingMode(config))
{
    completedParseJobs.setMaxSize(3);
    completedPlayJobs.setMaxSize(6);
    completedSequenceJobs.setMaxSize(3);

    localDeviceID = localDevice->getID();
    this->localDevice = localDevice;

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

std::string LocalEventEngineScheduler::sequenceSchedulingModeToString(SequenceSchedulingMode mode)
{
    switch (mode) {
    case SequenceSchedulingMode::Normal:
        return "Normal";
    case SequenceSchedulingMode::Interleaved:
        return "Interleaved";
    default:
        return "Normal";
    }
}

bool LocalEventEngineScheduler::parseSequenceSchedulingMode(const std::string& value, SequenceSchedulingMode& mode)
{
    auto normalized = normalizeStringForLookup(value);

    if (normalized == "normal") {
        mode = SequenceSchedulingMode::Normal;
        return true;
    }

    if (normalized == "interleaved" || normalized == "interleave") {
        mode = SequenceSchedulingMode::Interleaved;
        return true;
    }

    return false;
}

void LocalEventEngineScheduler::setSequenceSchedulingMode(SequenceSchedulingMode mode)
{
    sequenceSchedulingMode.store(mode);
    jobCondition.notify_all();
}

SequenceSchedulingMode LocalEventEngineScheduler::getSequenceSchedulingMode() const
{
    return sequenceSchedulingMode.load();
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
        engine->setPlaybackTimeouts(ownedDevicePlayReadyTimeout, ownedDeviceTriggerTimeout, ownedDevicePlayCompleteGrace);
        engine->setMeasurementGrace(ownedDeviceMaxMeasurementGrace, ownedDeviceMeasurementPollInterval);
        engine->setScheduler(this);   //so the engine can initiate post-processing dispatch at PlayComplete
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

void LocalEventEngineScheduler::clearEngine(const EngineID& engineID)
{
    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<LocalEventEngine> engine;

    if (engineManagers.get(engineID, manager) && manager != 0 && manager->getEngine(engine) && engine != 0) {
        engine->clear();
    }
}

void LocalEventEngineScheduler::stopEngine(const EngineID& engineID)
{
    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<LocalEventEngine> engine;

    if (engineManagers.get(engineID, manager) && manager != 0 && manager->getEngine(engine) && engine != 0) {
        std::shared_ptr<EventEngineJob> job;
        if (manager->getJob(job) && job != 0 && job->getStatus() == EngineJobStatus::Running) {
            _cancelJob(job->getJobID());
        }
        else {
            engine->stop();
        }
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


/// Recursively find all concrete post-processing target devices in the side-list.
/// Mirrors findEventTargets but reads the post-processing requests instead of the
/// hard-timed event table. Abstract (name-only) targets are skipped here; they are
/// reported as non-fatal "Missing post-processing target" warnings during resolve.
void LocalEventEngineScheduler::findPostProcessTargets(const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup, std::set<DeviceID>& ppTargets)
{
    if (eventGroup == 0) return;

    for (auto& request : eventGroup->postProcessRequests()) {
        if (!request.target().device().isAbstract()) {
            ppTargets.insert(request.target().device().deviceID());
        }
    }

    for (auto& g : eventGroup->getSubgroups()) {
        findPostProcessTargets(g, ppTargets);
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
    std::set<DeviceID> ppTargets;       //post-processing side-list targets (concrete)
    std::shared_ptr<STI::Engine::RawEventGroup> eventGroup;

    std::shared_ptr<Shot> shot;
    if (job->getShot(shot)) {
        shot->getRootEventGroup(eventGroup);
    }

    if (eventGroup != 0) {
        findEventTargets(eventGroup, eventTargets);
        findPostProcessTargets(eventGroup, ppTargets);

        auto stackTraceData = eventGroup->getStackTraceData();
        std::shared_ptr<STI::Utils::FileServer> remoteFileServer;
        auto virtualFileServer = persistenceManager->makeVirtualFileServer();
        
        if (stackTraceData != 0 && stackTraceData->getFileServer(remoteFileServer)) {
            transferTimingFiles(*stackTraceData, *remoteFileServer, *virtualFileServer);

            stackTraceData->setFileServer(virtualFileServer);
        }
    }
    else {
        job->addMessage(ParsingMessageType::Error, 5, "Missing Root Group")
            << "Invalid shot: the root event group is missing.";
        job->markCancelled();
    }

    //Create dependency tree
    auto tree = std::make_shared<EventEngineDependencyTree>();
    tree->addVertex(localDeviceID);     //begin tree with parse job owner
    
    //Dependency discovery follows declared event-target edges so generated partner
    //events can be routed later. A target missing during this phase is only a
    //potential requirement until an event in this shot actually targets it.
    std::set<DeviceID> discoveryMissingTargets;
    std::vector<EngineParsingMessage> messages;

    // Begin multi-pass search. Keep calling while new missing targets are found.
    if (localDependencyParser != 0) {
        localDependencyParser->getDependants(eventTargets, *tree, discoveryMissingTargets, messages, 5);  //max 5 passes

        //Extend the SAME tree with the post-processing targets' owning-server chains
        //so the owner can route dispatch to nested analysis devices (getDependants is
        //additive — it adds vertices/edges and never clears the passed-in tree). The
        //ppMissing/ppMessages results are intentionally kept separate from the event
        //path: a missing analysis device must never make the shot abstract or emit a
        //"Missing Targets" warning. Unreachable post-process targets are reported as a
        //non-fatal "Missing post-processing target" warning during resolve.
        std::set<DeviceID> ppMissing;
        std::vector<EngineParsingMessage> ppMessages;
        localDependencyParser->getDependants(ppTargets, *tree, ppMissing, ppMessages, 5);
    }

    for (auto& m : messages) {
        job->addMessage(m);
    }

    //Only discovery misses that are concrete targets in the submitted shot are
    //required now. Missing targets reached solely through a device's declaration
    //remain warning-only unless parsing actually generates events for them.
    std::set<DeviceID> missingTargets;
    std::set_intersection(eventTargets.begin(), eventTargets.end(),
                          discoveryMissingTargets.begin(), discoveryMissingTargets.end(),
                          std::inserter(missingTargets, missingTargets.end()));

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
        auto& m = job->addMessage(ParsingMessageType::Warning, 102, "Missing Targets")
            << "Some required event targets could not be found. "
            << "Parsing is proceeding as an abstract shot. Playing will not be possible. "
            << "Missing targets: \n";
        for (auto& id : diff) {
            m << "    " << id.getID() << "\n";
        }
    }

    missingTargets.insert(diff.begin(), diff.end());

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
    job->setMissingTargets(missingTargets);

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
    if (parseJob != 0) {
        job->setMissingTargets(parseJob->getMissingTargetIDs());
        //Resolved post-processing requests are owned by the engine that parsed the
        //shot (see LocalEventEngine::resolvePostProcessRequests). Play is always
        //assigned to that same engine, so the PlayComplete dispatch reads them
        //directly off the engine — no scheduler-side stash and no play-job copy.
    }

    addJob(job);

    return playJobStatus;
}

void LocalEventEngineScheduler::distributePostProcessing(const std::vector<PostProcessRequest>& requests,
                                                         const std::shared_ptr<EventEngineDependencyTree>& tree,
                                                         const ShotID& shotID, const STI::Device::DeviceID& jobOwnerID)
{
    //Tree-routed dispatch: for each request, deliver locally, deliver to a
    //directly-owned target, or forward the per-branch sublist to the owned branch
    //that leads to the target. Each hop repeats the same routing with the same tree,
    //to arbitrary depth (mirrors how parse distributes events). Every delivered
    //requestPostProcessing only enqueues and returns, so this is thin per hop.
    if (tree == 0 || localDevice == 0) {
        return;
    }

    std::shared_ptr<STI::Device::DeviceCollection> collection;
    localDevice->getCollection(collection);

    std::shared_ptr<STI::Device::PostProcessingManager> localManager;
    localDevice->getPostProcessingManager(localManager);

    //Requests bound for deeper devices, grouped by the directly-owned branch to forward to.
    std::map<DeviceID, std::vector<PostProcessRequest>> branchSublists;

    for (auto& request : requests) {
        const DeviceID targetID = request.target().device().deviceID();
        const std::string targetName = request.target().name();

        if (targetID == localDeviceID) {
            //Target is this device: deliver to the local PostProcessingManager.
            if (localManager != 0) {
                localManager->requestPostProcessing(targetName, shotID, jobOwnerID, request.options());
            }
            continue;
        }

        DeviceID branch;
        if (!tree->getBranchToTarget(localDeviceID, targetID, branch)) {
            //Unroutable: the tree should always contain a route (resolve used the same
            //lookup). Best-effort log; never throws or blocks (decision: log-only).
            localDevice->log() << "PostProcessing: no route from '" << localDeviceID.getID()
                               << "' to target '" << targetID.getID() << "' for shot " << shotID.print() << "\n";
            continue;
        }

        if (branch == targetID) {
            //Directly owned: deliver straight to the target's PostProcessingManager
            //(a RemotePostProcessingManager CORBA call when the target is remote).
            std::shared_ptr<STI::Device::Device> device;
            std::shared_ptr<STI::Device::PostProcessingManager> ppm;
            if (collection != 0 && collection->get(targetID, device) && device != 0
                && device->getPostProcessingManager(ppm) && ppm != 0) {
                ppm->requestPostProcessing(targetName, shotID, jobOwnerID, request.options());
            }
            else {
                localDevice->log() << "PostProcessing: directly-owned target '" << targetID.getID()
                                   << "' is unreachable for shot " << shotID.print() << "\n";
            }
        }
        else {
            //Deeper in the tree: forward to the owned branch that leads to the target.
            branchSublists[branch].push_back(request);
        }
    }

    //Forward each branch sublist to that branch's scheduler (recursive RPC; a
    //RemoteEventEngineScheduler when the branch is remote).
    for (auto& entry : branchSublists) {
        std::shared_ptr<STI::Device::Device> device;
        std::shared_ptr<EventEngineScheduler> sched;
        if (collection != 0 && collection->get(entry.first, device) && device != 0
            && device->getEngineScheduler(sched) && sched != 0) {
            sched->distributePostProcessing(entry.second, tree, shotID, jobOwnerID);
        }
        else {
            localDevice->log() << "PostProcessing: branch '" << entry.first.getID()
                               << "' is unreachable for forwarding for shot " << shotID.print() << "\n";
        }
    }
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
            SequenceID jobSeqID;
            if (getSequenceIDForJob(jobID, jobSeqID) && jobSeqID == seqid) {
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
        markSequenceJobSubmitted(newJob->getJobID());

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
        updateSequenceJobStatus(jobID, EngineJobStatus::Canceled);

        
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
        updateSequenceJobStatus(jobID, EngineJobStatus::Canceled);
        
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
        updateSequenceJobStatus(jobID, EngineJobStatus::Completed);
        
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

void LocalEventEngineScheduler::jobCanceled(const EngineJobID& jobID)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);

    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<EventEngineJob> archivedJob;
    
    if (runningJobs.get(jobID, job) && job != 0) {
        runningJobs.remove(jobID);
        job->markCancelled();
        updateSequenceJobStatus(jobID, EngineJobStatus::Canceled);
        
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

        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toCompleteList(job);
        sendMessage(message);

        if (valid) {
            archivedJob->markArchived();
            auto message2 = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
            message2->toArchive(archivedJob);
            sendMessage(message2);
        }
    }

    jobCondition.notify_all();
}

void LocalEventEngineScheduler::saveFailedSequenceParse(const std::shared_ptr<EventEngineJob>& job)
{
    if (job == 0) return;

    auto jobID = job->getJobID();
    if (jobID.type != EventEngineJobType::Parse) return;
    if (jobID.pid.shotType != ShotType::SequenceEntry) return;

    std::shared_ptr<EventEngine> eventEngine;
    if (!job->getEngine(eventEngine) || eventEngine == 0) return;

    std::shared_ptr<ParseResult> parseResult;
    if (!eventEngine->getParseResult(jobID.pid, parseResult) || parseResult == 0) return;

    auto localPersistenceManager = std::dynamic_pointer_cast<LocalPersistenceManager>(persistenceManager);
    if (localPersistenceManager == 0) return;

    localPersistenceManager->saveSequenceParseResult(
        jobID.pid,
        parseResult,
        job->getStatus(),
        job->getJobOwner() == localDeviceID);
}

bool LocalEventEngineScheduler::getSequenceIDForJob(const EngineJobID& jobID, SequenceID& seqid) const
{
    if (jobID.type == EventEngineJobType::Parse && jobID.pid.shotType == ShotType::SequenceEntry) {
        seqid = jobID.pid.sequenceEntryID.seqID;
        return true;
    }

    if (jobID.type == EventEngineJobType::Play && jobID.sid.parseID.shotType == ShotType::SequenceEntry) {
        seqid = jobID.sid.parseID.sequenceEntryID.seqID;
        return true;
    }

    return false;
}

bool LocalEventEngineScheduler::getActiveSequenceJob(const SequenceID& seqid, std::shared_ptr<SequenceJob>& job) const
{
    if (currentSequenceJob != 0 && currentSequenceJob->jobID.seqid == seqid) {
        job = currentSequenceJob;
        return true;
    }

    EngineJobID seqJobID(seqid);
    return queuedSequenceJobs.get(seqJobID, job) && job != 0;
}

bool LocalEventEngineScheduler::getSequenceJob(const SequenceID& seqid, std::shared_ptr<SequenceJob>& job) const
{
    if (getActiveSequenceJob(seqid, job)) {
        return true;
    }

    EngineJobID seqJobID(seqid);
    return completedSequenceJobs.get(seqJobID, job) && job != 0;
}

bool LocalEventEngineScheduler::getSequenceJobForQueuedJob(const EngineJobID& queuedJobID, std::shared_ptr<SequenceJob>& job) const
{
    SequenceID seqid;
    if (!getSequenceIDForJob(queuedJobID, seqid)) {
        return false;
    }

    EngineJobID seqJobID(seqid);
    return queuedSequenceJobs.get(seqJobID, job) && job != 0;
}

void LocalEventEngineScheduler::markSequenceJobSubmitted(const EngineJobID& jobID)
{
    SequenceID seqid;
    if (!getSequenceIDForJob(jobID, seqid)) {
        return;
    }

    std::shared_ptr<SequenceJob> job;
    if (getActiveSequenceJob(seqid, job) && job != 0) {
        job->markJobSubmitted();
    }
}

void LocalEventEngineScheduler::updateSequenceJobStatus(const EngineJobID& jobID, const EngineJobStatus& status)
{
    SequenceID seqid;
    if (!getSequenceIDForJob(jobID, seqid)) {
        return;
    }

    std::shared_ptr<SequenceJob> job;
    if (!getSequenceJob(seqid, job) || job == 0 || job->sequenceResult == 0) {
        return;
    }

    SequenceIndex index;
    if (jobID.type == EventEngineJobType::Play) {
        index = jobID.sid.parseID.sequenceEntryID.seqIndex;
        job->sequenceResult->shots[index] = jobID.sid;
    }
    else if (jobID.type == EventEngineJobType::Parse && status == EngineJobStatus::Canceled) {
        index = jobID.pid.sequenceEntryID.seqIndex;
    }
    else {
        return;
    }

    job->sequenceResult->status[index] = status;
}

void LocalEventEngineScheduler::promoteSequenceJob(const std::shared_ptr<SequenceJob>& job)
{
    if (job == 0) return;

    queuedSequenceJobs.remove(job->jobID);
    currentSequenceJob = job;

    auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
    message->toRunningList(currentSequenceJob);
    sendMessage(message);
}

void LocalEventEngineScheduler::completeSequenceJob(const std::shared_ptr<SequenceJob>& job)
{
    if (job == 0) return;

    completedSequenceJobs.add(job->jobID, job);

    auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
    message->toCompleteList(job);
    sendMessage(message);
}

void LocalEventEngineScheduler::refreshQueuedSequenceJobs()
{
    std::set<EngineJobID> jobIDs;
    queuedSequenceJobs.getKeys(jobIDs);

    std::shared_ptr<SequenceJob> job;
    for (const auto& jobID : jobIDs) {
        if (queuedSequenceJobs.get(jobID, job) && job != 0 && job->isDone()) {
            queuedSequenceJobs.remove(jobID);
            completeSequenceJob(job);
        }
    }
}

void LocalEventEngineScheduler::refreshSequenceJobs(const std::set<EngineJobID>& queuedJobIDs)
{
    bool done = false;
    
    if (currentSequenceJob == 0) {
        done = true;
    }

    if (currentSequenceJob != 0 && currentSequenceJob->isDone()) {
        currentSequenceJob->runningJobs.clear();

        auto completedJob = currentSequenceJob;
        currentSequenceJob = 0;
        done = true;

        completeSequenceJob(completedJob);
    }

    if (currentSequenceJob != 0 && !currentSequenceJob->hasSubmittedJobs()) {
        auto idleJob = currentSequenceJob;
        currentSequenceJob = 0;
        queuedSequenceJobs.add(idleJob->jobID, idleJob);
        done = true;

        auto message = std::make_shared<STI::Device::EngineJobUpdateDeviceMessage>(localDeviceID);
        message->toQueuedList(idleJob);
        sendMessage(message);
    }

    refreshQueuedSequenceJobs();

    if (!done) return;

    std::shared_ptr<SequenceJob> nextJob;

    // Promote the sequence associated with the oldest queued sequence entry job.
    // Merely adding a sequence does not make it current.
    for (const auto& queuedJobID : queuedJobIDs) {
        if (getSequenceJobForQueuedJob(queuedJobID, nextJob) && nextJob != 0) {
            promoteSequenceJob(nextJob);
            return;
        }
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
            std::set<EngineJobID>* assignableJobIDs = &queuedJobIDs;

            if (sequenceSchedulingMode.load() == SequenceSchedulingMode::Normal) {
                refreshSequenceJobs(queuedJobIDs);

                if (currentSequenceJob != 0 &&
                    !currentSequenceJob->isDone() &&
                    currentSequenceJob->hasPriority(queuedJobIDs)) {
                    currentSequenceJob->filter(queuedJobIDs, filteredJobsIDs);
                    assignableJobIDs = &filteredJobsIDs;
                }
            }
            else {
                refreshQueuedSequenceJobs();
            }

            assignPlayJobs(*assignableJobIDs, freeEngines);
            assignParseJobs(*assignableJobIDs, freeEngines);
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

    return getSequenceJob(seqID, job);
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

bool LocalEventEngineScheduler::getLastParseResult(const EngineID& engineID, std::shared_ptr<ParseResult>& parseResult) const
{
    if (searchingParseResult) return false;

    std::unique_lock<std::mutex> resultLock(parseResultMutex);
    searchingParseResult = true;

    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<LocalEventEngine> engine;

    bool success = engineManagers.get(engineID, manager) && manager != 0
        && manager->getEngine(engine) && engine != 0
        && engine->getLastParseResult(parseResult);

    searchingParseResult = false;
    return success;
}

bool LocalEventEngineScheduler::getLastShotResult(const EngineID& engineID, std::shared_ptr<ShotResult>& shotResult) const
{
    if (searchingShotResult) return false;

    std::unique_lock<std::mutex> resultLock(shotResultMutex);
    searchingShotResult = true;

    std::shared_ptr<EventEngineManager> manager;
    std::shared_ptr<LocalEventEngine> engine;

    bool success = engineManagers.get(engineID, manager) && manager != 0
        && manager->getEngine(engine) && engine != 0
        && engine->getLastShotResult(shotResult);

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
    playMessageIDs["Cannot Play Abstract Shot"]             = 70;
    playMessageIDs["Bad engine state"]                      = 71;
    playMessageIDs["Owned devices not PlayReady"]           = 72;
    playMessageIDs["Not Parsed"]                            = 73;
    playMessageIDs["Failed to contact owned device"]        = 74;
    playMessageIDs["PlayReady message invalid"]             = 75;
    playMessageIDs["PlayComplete message invalid"]          = 76;
    playMessageIDs["Play called while not PlayReady"]       = 77;
    playMessageIDs["Play parse ID mismatch"]                = 78;
    playMessageIDs["Failed to enter WaitingForTrigger"]     = 79;
    playMessageIDs["Failed to enter Playing"]               = 80;
    playMessageIDs["Play canceled"]                         = 81;
    playMessageIDs["Owned device state invalid"]            = 82;
    playMessageIDs["Owned device PlayReady timeout"]        = 83;
    playMessageIDs["Owned device trigger timeout"]          = 84;
    playMessageIDs["Owned device PlayComplete timeout"]     = 85;
    playMessageIDs["Incorrect Measurement Type"]            = 86;
    playMessageIDs["Device read failed"]                    = 87;
    playMessageIDs["Device write failed"]                   = 88;
    playMessageIDs["Missing Measurement Result"]            = 89;
}

const std::map<std::string, unsigned>& LocalEventEngineScheduler::getPlayMessageIDs()
{
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        LocalEventEngineScheduler::definePlayMessageIDs();
    });

    return playMessageIDs;
}
