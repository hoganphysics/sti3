

#include "LocalEventEngineScheduler.h"
#include "DeviceID.h"
#include "SynchronizedMap.h"
#include "LocalEventEngineJob.h"
#include "DeviceMessage.h"
#include "EventEngineManager.h"
#include "fwd/RawEvent_fwd.h"
#include "ParseID.h"
#include "EventEngineJob.h"
#include "Shot.h"
#include "LocalEventEngineJob.h"
#include "EventEngineFactory.h"
#include "LocalEventEngineFactory.h"
#include "EngineParsingMessage.h"
#include "LocalShot.h"
#include "ShotID.h"
#include "EngineJobID.h"

#include <set>
#include <vector>
#include <memory>
#include <algorithm>


using STI::Engine::LocalEventEngineJob;
using STI::Engine::LocalEventEngineScheduler;
using STI::Engine::EventEngineDependencyTree;
using STI::Device::DeviceID;
using STI::Device::EngineSchedulerMessage;
using STI::Engine::EngineID;
using STI::Engine::EventEngine;
using STI::Device::DeviceTrace;
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


LocalEventEngineScheduler::LocalEventEngineScheduler(STI::Device::LocalDevice* localDevice, 
                                                    const std::shared_ptr<STI::Engine::EventEngineFactory>& engineFactory)
: localDevice(localDevice), completedJobs(3)
{
    completedJobs.setMaxSize(3);

    localDeviceID = localDevice->getID();
    localDevice->getCollection(localCollection);

    running = true;
    schedulerThread = std::thread(&LocalEventEngineScheduler::assignJobs, this);

    setEngineFactory(engineFactory);
}

LocalEventEngineScheduler::~LocalEventEngineScheduler()
{
    stop();
    schedulerThread.join();
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

void LocalEventEngineScheduler::parse(const ParseID& parseID, const std::shared_ptr<Shot>& shot)
{
    auto job = std::make_shared<LocalEventEngineJob>(parseID, shot, localDeviceID);

    //Get list unique device targets
    std::set<DeviceID> eventTargets;
    std::shared_ptr<STI::Engine::RawEventVector> events;

    if (shot != 0) {
        shot->getEvents(events);        
    }

    if (events != 0) {
        for(auto& evt : *events) {
            eventTargets.insert(evt.targetDevice());
        }
    }

    //Create dependency tree
    auto tree = std::make_shared<EventEngineDependencyTree>();
    
    std::set<DeviceID> missingTargets;
    std::vector<EngineParsingMessage> messages;

    // Begin multi-pass search. Keep calling while new missingTargets are found.
    getDependants(eventTargets, *tree, missingTargets, messages, 5);  //max 5 passes

    for (auto& m : messages) {
        job->addMessage(m);
    }

    //Check for missing targets
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

    //Make parse job
    //auto job = std::make_shared<LocalEventEngineJob>(parseID, shot, tree, localDeviceID, diff);

    job->setDependencies(tree);
    job->setMissingTargets(diff);

    addJob(job);
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

bool LocalEventEngineScheduler::loopDetected(const DeviceTrace& trace, DeviceTrace& newTrace)
{
    //Avoid infinite loop by using DeviceTrace
    if (trace.includesID(localDeviceID)) {
        //This node has already been visited; short circuit the call (the network graph has a loop)
        return true;
    }

    //Append this ID to the trace before passing down the graph to avoid infinite loop
    newTrace = trace;
    newTrace.addID(localDeviceID);

    return false;
}

bool LocalEventEngineScheduler::getTargetScheduler(const STI::Device::DeviceID& id, std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
{
    std::shared_ptr<STI::Device::Device> device;
    
    return localCollection != 0 && localCollection->get(id, device) 
            && device != 0 && device->getEngineScheduler(scheduler);
}

void LocalEventEngineScheduler::addDeviceEventTargets(EventEngineDependencyTree& tree, std::vector<EngineParsingMessage>& messages, 
                                                        const STI::Device::DeviceTrace& trace)
{
	STI::Device::DeviceTrace newTrace;
    if (loopDetected(trace, newTrace)) {
        return;     //avoid infinite recursion if the network graph has a loop
    }

    EventEngineDependencyTree subtree;
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    //Start the new tree by adding local device
    tree.clear();
    tree.addVertex(localDeviceID);
        
    //Get all event targets that were explicitly declared by this device
    std::set<STI::Device::DeviceID> targetIDs;
    localDevice->getEventTargets(targetIDs);
        
    //Add local event targets (the local device can generate events for these)
    for (auto& targetID : targetIDs) {

        subtree.clear();

        tree.addEdge(localDeviceID, targetID);

        //Attempt to get this device reference and then get it's subtree
        if (getTargetScheduler(targetID, scheduler)) {

            std::vector<EngineParsingMessage> partnerMessages;
            
            scheduler->addDeviceEventTargets(subtree, partnerMessages, newTrace);

            tree.addTree(subtree);
            messages.insert(messages.end(), partnerMessages.begin(), partnerMessages.end());
        }
        else {
            //Warning, event target device not connected
            messages.emplace_back(localDeviceID, ParsingMessageType::Warning, 1001, "Event Target Device Missing");
            messages.back() 
                << "Device '" << localDeviceID.getID() << "' may generate events for target device '"
                << targetID.getID() << "', but the target device's EventEngineScheduler could not be found " 
                << "(device is likely missing from the network). Parsed shot may be forced to become abstract.";
        }
    }
}

void LocalEventEngineScheduler::addToTargetsByServer(const std::set<DeviceID>& targets, const EventEngineDependencyTree& tree, std::map<std::string, std::set<DeviceID>>& targetsByServer)
{
    //Note: if the id is not in the tree, it will be trivially added to set
    for (auto& id : targets) {
        if (id !=localDeviceID && !tree.hasBranchToTarget(id, localDeviceID)) {
            //this id has no server path to the local device.
            targetsByServer[id.getTargetServerID()].insert(id);
        }        
    }
}

void LocalEventEngineScheduler::getServerChainIDs(std::set<STI::Device::DeviceID>& serverIDs)
{
    serverIDs.clear();

    std::set<STI::Device::DeviceID> ownedIDs;
    if (localCollection != 0) {
        localCollection->getIDs(ownedIDs);        
    }
   
    for (auto& id : ownedIDs) {
        if (localDeviceID.getID() == id.getTargetServerID()) {
            serverIDs.insert(id);
        }
    }
}

void LocalEventEngineScheduler::getPartnerDeviceDependants(const DeviceID& partnerID, const std::set<DeviceID>& targets, 
                                                    EventEngineDependencyTree& tree, std::set<DeviceID>& missingIDs, 
                                                    std::vector<EngineParsingMessage>& messages, const DeviceTrace& trace)
{
    if (targets.size() == 0) {
        return;
    }

    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    EventEngineDependencyTree subtree;

    if (getTargetScheduler(partnerID, scheduler)) {
        
        subtree.clear();
        scheduler->getDependants(targets, subtree, missingIDs, messages, trace);
           
        //Add found subtree to the tree.
        //Uses greater than 1 because the subtree always contains the server id, but we only add if there are also others.
        //(Unless the partnerID is in the target list)
        if (subtree.vertexCount() > 1 || targets.find(partnerID) != targets.end()) {
            tree.addTree(subtree);
            tree.addEdge(localDeviceID, partnerID);                
        }
    }
    else {
        //Could not contact the device; these targets cannot be reached
        missingIDs.insert(targets.begin(), targets.end());
    }
}

void LocalEventEngineScheduler::getDependants(const std::set<DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                                            std::set<STI::Device::DeviceID>& missingTargets, std::vector<EngineParsingMessage>& messages, 
                                            unsigned maxRecursions)
{
    unsigned passes = 0;
    bool repeat = false;

    std::set<DeviceID> targets = evtTargets;

    do {
        ++passes;
        repeat = false;

        getDependants(targets, tree, missingTargets, messages, STI::Device::DeviceTrace());

        // If there are still missingTargets, they may be found on another pass.
        // Make sure the new missingTarget list is not the same as the last targets list, 
        // since those IDs have already been tried and were missing.
        if (missingTargets.size() > 0 && targets != missingTargets) {
            targets = missingTargets;
            repeat = true;
        }

    } while (repeat && passes < maxRecursions);
}


/// Generates a graph of the network that contains all devices needed to parse the events.
/// Does this in three steps:  (1) Local device, (2) Direct device decendents, (3) Full depth recursive search of graph.
void LocalEventEngineScheduler::getDependants(const std::set<DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                                                std::set<STI::Device::DeviceID>& missingTargets, std::vector<EngineParsingMessage>& messages,
                                                const STI::Device::DeviceTrace& trace)
{
	STI::Device::DeviceTrace newTrace;
    if (loopDetected(trace, newTrace)) {
        return;     //avoid infinite recursion if the network graph has a loop
    }

    EventEngineDependencyTree subtree;

    //*** (1) Check for events targeting the local device ***//

    //If there are local events, add local ID *and* this device's event targets, since the local
    //device can generate events on its event targets.
    auto local_it = evtTargets.find(localDeviceID);

    if (local_it != evtTargets.end()) {
        addDeviceEventTargets(subtree, messages, STI::Device::DeviceTrace());
        tree.addTree(subtree);
    }

    //Now all partners for local device are added; but there are two issues:
    // 1) There are event targets in evtTargets that have not been found via a server chain
    // 2) There are targets in the current tree that don't have their server chain added 
    //    (partners can generally have some other server)


    //*** (2) Sort event targets by server ***//

    std::map<std::string, std::set<DeviceID>> targetsByServer;   // ["server", {devices}]
    addToTargetsByServer(evtTargets, tree, targetsByServer);   

    std::set<DeviceID> localTargets;    //for targets originating from addDeviceEventTargets above
    tree.getNodes(localTargets);
    addToTargetsByServer(localTargets, tree, targetsByServer);

   
    //*** (3) Pass targets down the server chain ***//

    //searchServerChain(serverChainIDs, targetsByServer, tree, missingTargets);

    //Server chain: get all connected devices that declare the local device as server.
    std::set<STI::Device::DeviceID> serverChainIDs;
    getServerChainIDs(serverChainIDs);
    
    std::set<STI::Device::DeviceID> missingIDs;

    //Pass all targetsByServer['ID'] target lists to any 'ID' found in serverChainIDs.
    for (auto it = targetsByServer.begin(); it != targetsByServer.end(); ) {

        // Check if it=targetsByServer['ID'] is in serverChainIDs:
        auto found_it = std::find_if(serverChainIDs.begin(), serverChainIDs.end(),
                        [&](const DeviceID& id) { return (it->first == id.getID()); });
        
        if (found_it != serverChainIDs.end()) {

            //Pass targetsByServer['ID'] target list to the locally connected server
            getPartnerDeviceDependants(*found_it, it->second, tree, missingIDs, messages, newTrace);  //it->second = target list
            //missingTargets.insert(missingIDs.begin(), missingIDs.end());

            it = targetsByServer.erase(it);     //remove after attempt to transfer
        }
        else {
            ++it;
        }
    }

    addToTargetsByServer(missingIDs, tree, targetsByServer);    //sort any new missing ids by their server
    missingIDs.clear();

    //*** (4) Add targets that declare the local device as server ***//

    std::set<DeviceID> targetsForLocal;    

    //Add any targets that declare this as their server
    auto it = targetsByServer.find(localDeviceID.getID());
    if (it != targetsByServer.end()) {

        //Targets found that need this device as server
        tree.addVertex(localDeviceID);   //add if not already added

        for (auto& id : it->second) {
            targetsForLocal.clear();
            targetsForLocal.insert(id);

            getPartnerDeviceDependants(id, targetsForLocal, tree, missingIDs, messages, newTrace);
        }
        targetsByServer.erase(it);
    }

    addToTargetsByServer(missingIDs, tree, targetsByServer);
    missingIDs.clear();


    //*** (5) Search the full graph for any missing targets, following server chain ***//

    //Any targets in targetsByServer could not be found by the local device.  Pass them downstream to the 
    //network, following the server chain.
    //Need to search all currently connected devices (full graph search) because the events targets and
    //their servers could be multiple layers deep.

    //First construct downstreamIDs
    std::set<STI::Device::DeviceID> downstreamIDs;
    getDownstreamIDs(targetsByServer, tree, downstreamIDs);

    if (downstreamIDs.size() == 0) {
        //No missing targets; short circuit.
        return;
    }

    for (auto& id : serverChainIDs) {    // Follow server chain through the graph
        missingIDs.clear();
        getPartnerDeviceDependants(id, downstreamIDs, tree, missingIDs, messages, newTrace);
        downstreamIDs.swap(missingIDs);
    }

    //Anything left is missing; may be reachable with another pass.
    missingTargets.insert(downstreamIDs.begin(), downstreamIDs.end());
}

void LocalEventEngineScheduler::getDownstreamIDs(const std::map<std::string, std::set<DeviceID>> targetsByServer, 
                                            const EventEngineDependencyTree& tree, std::set<DeviceID>& downstreamIDs)
{
    downstreamIDs.clear();

    //Put any remaining targets in downstreamIDs
    for (auto& it : targetsByServer) {
        downstreamIDs.insert(it.second.begin(), it.second.end());
    }
    //targetsByServer.clear(); 

    //Check for any target in the tree that is still not connected via a server chain
    std::set<STI::Device::DeviceID> allTreeIDs;
    tree.getNodes(allTreeIDs);

    //Add any targets in the tree that are not connected via the server chain
    for(auto& id : allTreeIDs) {
        if (id != localDeviceID && !tree.hasBranchToTarget(localDeviceID, id)) {
            downstreamIDs.insert(id);
        }
    }

}

void LocalEventEngineScheduler::addJob(const std::shared_ptr<EventEngineJob>& newJob)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);
        
    if (newJob != 0) {
        queuedJobs.add(newJob->getJobID(), newJob);
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

void LocalEventEngineScheduler::cancelJob(const EngineJobID& jobID)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);
    _cancelJob(jobID);
}

void LocalEventEngineScheduler::_cancelJob(const EngineJobID& jobID)
{
   
    // std::set<EngineJobID> jobIDs;
    // runningJobs.getKeys(jobIDs);
    
    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<EventEngineManager> manager;

    // auto it = jobIDs.find(jobID);   //find in runningJobs

    // if (it != jobIDs.end() && runningJobs.get(*it, job) && job != 0) {
    //     runningJobs.remove(jobID);
    //     job->markCancelled();
    //     completedJobs.add(jobID, job);

    //     if (getManager(jobID, manager) && manager != 0) {
    //         manager->abortJob();
    //     }        
    // }

    if (runningJobs.get(jobID, job) && job != 0) {
        runningJobs.remove(jobID);
        job->markCancelled();
        
        completedJobs.add(jobID, job);
    }

    if (queuedJobs.get(jobID, job) && job != 0) {
        queuedJobs.remove(jobID);
        job->markCancelled();
        
        completedJobs.add(jobID, job);
    }

    if (getManager(jobID, manager) && manager != 0) {
        manager->abortJob();
    }

    jobCondition.notify_all();
}

// std::shared_ptr<EventEngineJob> LocalEventEngineScheduler::createJob(const ParseID& parseID, 
//                                           const std::shared_ptr<Shot>& shot,
//                                           const std::shared_ptr<EventEngineDependencyTree>& tree, 
//                                           const STI::Device::DeviceID& owner, 
//                                           const std::set<STI::Device::DeviceID>& missingTargets)
// {
//     auto job = std::make_shared<LocalEventEngineJob>(parseID, shot, owner);
//     job->setDependencies(tree);
//     job->setMissingTargets(missingTargets);
//     return job;
// }

std::shared_ptr<Shot> LocalEventEngineScheduler::createShot(const std::shared_ptr<RawEventVector>& events)
{
    auto shot = std::make_shared<LocalShot>();
    shot->setEvents(events);
    return shot;
}



void LocalEventEngineScheduler::jobComplete(const EngineJobID& jobID)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);

    std::shared_ptr<EventEngineJob> job;
    
    if (runningJobs.get(jobID, job) && job != 0) {
        runningJobs.remove(jobID);
        job->markComplete();
        completedJobs.add(jobID, job);
    }

    jobCondition.notify_all();
}

void LocalEventEngineScheduler::assignJobs()
{
    std::unique_lock<std::mutex> jobLock(jobMutex);

    std::set<EngineID> allEngines;
    std::set<EngineID> freeEngines;
    std::set<EngineJobID> queuedJobIDs;     //sorted by priority

//    std::set<EngineJobID> queuedPlayJobIDs;     //sorted by priority
  
    std::shared_ptr<EventEngineManager> manager;
    EngineID engineID;

    int assignableJobCount;

    do {
        engineManagers.getKeys(allEngines);
        queuedJobs.getKeys(queuedJobIDs);
        freeEngines.clear();

        // //Filter play jobs
        // queuedPlayJobIDs.clear();
        // std::copy_if(queuedJobIDs.begin(), queuedJobIDs.end(), std::back_inserter(queuedPlayJobIDs),
        //          [](const EngineJobID& id){ return id.type == EventEngineJobType::Play; });

        //check for available engines
        for (auto& id : allEngines) {
            if (engineManagers.get(id, manager) && manager != 0 && !manager->jobRunning()) {
                freeEngines.insert(id);
            }
        }
        assignableJobCount = queuedJobIDs.size();
        //assignableFreeEngines = freeEngines.size();

        if (freeEngines.size() > 0) {
            
            //First priority is to run a play event if a free engine is parsed for it, regardless of position in set.
            for (auto jobID : queuedJobIDs) {
                
                if (jobID.type == EventEngineJobType::Play) {
                    
                    //check if the associated parse job was canceled
                    if (isCanceledJob(jobID.pid)) {
                        _cancelJob(jobID);  //cancel play if parse was canceled
                    }

                    // findParsedEngine(jobID.pid, freeEngines, engineID)
                    //     && assignJob(jobID, engineID)
                
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
            for (auto jobID : queuedJobIDs) {
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

    return (success && job->getStatus() == EventEngineJob::EngineJobStatus::Canceled);
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
    STI::Engine::TimeStamp oldest;

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

    bool b1 = runningJobs.get(jobID, job);
    bool b2 = b1 && job != 0;
    bool b3 = b2 && engineManagers.get(job->getEngineID(), manager);
    bool b4 = b3 && manager != 0;

    if (b4) {
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

bool LocalEventEngineScheduler::getParsedEngine(const ParseID& parseID, std::shared_ptr<LocalEventEngine>& engine) const
{
    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<EventEngineManager> manager;

    EngineJobID jobID;
    jobID.type = EventEngineJobType::Parse;
    jobID.pid = parseID;

    // std::cout << "getJob? " << (completedJobs.get(jobID, job) && job !=0) << std::endl;
    // std::cout << "getManager? " << (engineManagers.get(job->getEngineID(), manager) && manager != 0) << std::endl;
    // std::cout << "EngineID=" << job->getEngineID().getNumber() << std::endl;

    // std::cout << "EngineIDs: ";
    // std::set<STI::Engine::EngineID> ids;
    // engineManagers.getKeys(ids);
    // for (auto id : ids) {
    //     std::cout << id.getNumber() << " ";
    // }
    // std::cout << std::endl;


    // if (queuedJobs.get(jobID, job) && job !=0 
    //     && engineManagers.get(job->getEngineID(), manager) && manager != 0) {
    if (completedJobs.get(jobID, job) && job !=0 
        && engineManagers.get(job->getEngineID(), manager) && manager != 0) {
            
        manager->getEngine(engine);
        
        return engine != 0 && engine->getLastParseID() == parseID;
    }

    return false;
}

bool LocalEventEngineScheduler::getParsedEvents(const ParseID& parseID, DeviceEventMap& events) const
{
    std::shared_ptr<LocalEventEngine> engine;

    if (getParsedEngine(parseID, engine)){
        events = engine->getParsedEvents();
        return engine->getLastParseID() == parseID;
    }

    return false;
}

bool LocalEventEngineScheduler::getParsingMessages(const ParseID& parseID, std::vector<EngineParsingMessage>& messages) const
{

    // {
    //     std::unique_lock<std::mutex> jobLock(jobMutex);
    //     std::cout << "Jobs: " << queuedJobs.size() 
    //         << " : " <<  runningJobs.size() 
    //         << " : " <<  completedJobs.size() << std::endl;
    // }


    std::shared_ptr<EventEngineJob> job;

    if (findJob(parseID, job)) {
        messages = job->getParsingMessages();
        return true;
    }


    // std::shared_ptr<LocalEventEngine> engine;

    // if (getParsedEngine(parseID, engine)){
    //     messages = engine->getParsingMessages();

    //     std::cout << "parsing Messages length: " << messages.size() << std::endl;
        
    //     return engine->getLastParseID() == parseID;
    // }

    return false;
}

bool LocalEventEngineScheduler::getParsedTree(const ParseID& parseID, std::shared_ptr<EventEngineDependencyTree>& tree) const
{
    std::shared_ptr<LocalEventEngine> engine;

    if (getParsedEngine(parseID, engine)){
        tree = engine->getParsedTree();
        return engine->getLastParseID() == parseID;
    }

    return false;
}

