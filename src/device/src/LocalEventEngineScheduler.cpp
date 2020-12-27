

#include "LocalEventEngineScheduler.h"
#include "DeviceID.h"
#include "SynchronizedMap.h"
#include "LocalEventEngineJob.h"
#include "DeviceEvent.h"
#include "EventEngineManager.h"
#include "fwd/RawEvent_fwd.h"
#include "ParseID.h"
#include "EventEngineJob.h"
#include "ParsedShot.h"
#include "LocalEventEngineJob.h"
#include "EventEngineFactory.h"
#include "LocalEventEngineFactory.h"

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
using STI::Engine::ParsedShot;
using STI::Engine::LocalEventEngineJob;
using STI::Engine::ShotID;
using STI::Engine::EngineJobID;
using STI::Engine::EventEngineManager;
using STI::Engine::LocalEventEngineFactory;


LocalEventEngineScheduler::LocalEventEngineScheduler(STI::Device::LocalDevice* localDevice)
: localDevice(localDevice), completedJobs(3)
{
    completedJobs.setMaxSize(3);

    running = true;
    schedulerThread = std::thread(&LocalEventEngineScheduler::assignJobs, this);

    auto engineFactory = std::make_shared<STI::Engine::LocalEventEngineFactory>();
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

    for (auto& id : ids) {
        addEngine(id);  //replaces existing engine with newly created engine (using new factory)
    }

}

void LocalEventEngineScheduler::addEngine(const EngineID& engineID)
{
    std::shared_ptr<STI::Device::DeviceEventDispatcher> dispatcher;
    localDevice->getEventDispatcher(dispatcher);
    std::shared_ptr<STI::Device::DeviceCollection> collection;
    localDevice->getCollection(collection);

    std::shared_ptr<LocalEventEngine> engine = 
        eventEngineFactory->createEngine(localDevice->getID(), localDevice->localChannels, localDevice, dispatcher, collection);
    
    auto manager = std::make_shared<EventEngineManager>(engineID, engine, this);

    engineManagers.add(engineID, manager);
}

void LocalEventEngineScheduler::parse(const ParseID& parseID, const std::shared_ptr<ParsedShot>& shot)
{
    if(shot == 0) return;

    //Resolve all parsed events, generating RawEvent list (in python?)

    //Get list unique device targets
    std::set<DeviceID> eventTargets;

    std::shared_ptr<STI::Engine::RawEventVector> events;
    shot->getEvents(events);

    if (events != 0) {
        for(auto& evt : *events) {
            eventTargets.insert(evt.targetDevice());
        }
    }
    
    STI::Device::DeviceTrace trace;     //Trace needed to avoid infinite recursion while mapping the network

    //Create dependency tree
    auto tree = std::make_shared<EventEngineDependencyTree>();
    
    //Could wrap this in a loop for multipass searches.  Keep calling until the tree is static, or missing targets is static.  tree.getNodes() size?
    std::set<DeviceID> missingTargets;
//    getDependants(eventTargets, *tree, missingTargets, trace);
    getDependants(eventTargets, *tree, missingTargets, 5);  //max 5 passes

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
    }

    //check for circular dependencies
    std::vector<DeviceID> orderedNodes;
    bool isDAG = tree->sortTree(orderedNodes);
    
    if (!isDAG) {
        //Error: Circular dependency loop detected. The device network must be a directed acyclic graph (DAG).
        //The following devices specify event targets that form the illegal loop:
        
        std::vector<DeviceID> cycle;
        if(tree->getCycle(cycle)) {
            //Throw error with cycle
        }

        return; //abort
    }

    //Make parse job
    auto job = std::make_shared<LocalEventEngineJob>(parseID, shot, tree, localDevice->getID(), diff);

    addJob(job);
}

void LocalEventEngineScheduler::play(const ShotID& shotID)
{
    //Make play job
    EngineJobID jobID;
    jobID.pid = shotID.parseID;
    jobID.sid = shotID;
    jobID.type = EventEngineJobType::Play;
    auto job = std::make_shared<LocalEventEngineJob>(jobID, localDevice->getID());

    addJob(job);
}

bool LocalEventEngineScheduler::loopDetected(const DeviceTrace& trace, DeviceTrace& newTrace)
{
    //Avoid infinite loop by using DeviceTrace
    if (trace.includesID(localDevice->getID())) {
        //This node has already been visited; short circuit the call (the network graph has a loop)
        return true;
    }

    //Append this ID to the trace before passing down the graph to avoid infinite loop
    newTrace = trace;
    newTrace.addID(localDevice->getID());

    return false;
}

void LocalEventEngineScheduler::addDeviceEventTargets(EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace)
{
	STI::Device::DeviceTrace newTrace;
    if (loopDetected(trace, newTrace)) {
        return;     //avoid infinite recursion if the network graph has a loop
    }

    EventEngineDependencyTree subtree;
    std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    std::shared_ptr<STI::Device::DeviceCollection> localCollection;

    localDevice->getCollection(localCollection);

    //Start the new tree by adding local device
    tree.clear();
    tree.addVertex(localDevice->getID());
        
    //Get all event targets that were explicitly declared by this device
    std::set<STI::Device::DeviceID> targetIDs;
    localDevice->getEventTargets(targetIDs);
        
    //Add local event targets (the local device can generate events for these)
    for (auto& targetID : targetIDs) {

        subtree.clear();

        tree.addEdge(localDevice->getID(), targetID);

        //Attempt to get this device reference and then get it's subtree
        if(localCollection->get(targetID, device) && device != 0 && 
                device->getEngineScheduler(scheduler)) 
        {
            scheduler->addDeviceEventTargets(subtree, newTrace);
            tree.addTree(subtree);
        }
        else {
            //Warning, event target device not connected
        }
    }
}

void LocalEventEngineScheduler::addToTargetsByServer(const std::set<DeviceID>& targets, const EventEngineDependencyTree& tree, std::map<std::string, std::set<DeviceID>>& targetsByServer)
{
    //Note: if the id is not in the tree, it will be trivially added to set
    for (auto& id : targets) {
        if (id !=localDevice->getID() && !tree.hasBranchToTarget(id, localDevice->getID())) {
            //this id has no server path to the local device.
            targetsByServer[id.getTargetServerID()].insert(id);
        }        
    }
}

void LocalEventEngineScheduler::getServerChainIDs(std::set<STI::Device::DeviceID>& serverIDs)
{
    serverIDs.clear();

    std::shared_ptr<STI::Device::DeviceCollection> localCollection;
    localDevice->getCollection(localCollection);

    std::set<STI::Device::DeviceID> ownedIDs;
    localCollection->getIDs(ownedIDs);
   
    for (auto& id : ownedIDs) {
        if (localDevice->getID().getID() == id.getTargetServerID()) {
            serverIDs.insert(id);
        }
    }
}

void LocalEventEngineScheduler::getPartnerDeviceDependants(const DeviceID& partnerID, const std::set<DeviceID>& targets, 
                                                    EventEngineDependencyTree& tree, std::set<DeviceID>& missingIDs, 
                                                    const DeviceTrace& trace)
{
    if (targets.size() == 0) {
        return;
    }

    std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    EventEngineDependencyTree subtree;
    //std::set<STI::Device::DeviceID> foundIDs;

    std::shared_ptr<STI::Device::DeviceCollection> localCollection;
    localDevice->getCollection(localCollection);

    if(localCollection->get(partnerID, device) && device != 0 && device->getEngineScheduler(scheduler)) {
                    
        subtree.clear();
        scheduler->getDependants(targets, subtree, missingIDs, trace);
           
        //Add found subtree to the tree.
        //Uses greater than 1 because the subtree always contains the server id, but we only add if there are also others.
        //(Unless the partnerID is in the target list)
        if (subtree.vertexCount() > 1 || targets.find(partnerID) != targets.end()) {
            tree.addTree(subtree);
            tree.addEdge(localDevice->getID(), partnerID);                
        }
    }
    else {
        //Could not contact the device; these targets cannot be reached
        missingIDs.insert(targets.begin(), targets.end());
    }
}

void LocalEventEngineScheduler::getDependants(const std::set<DeviceID>& evtTargets, EventEngineDependencyTree& tree, std::set<STI::Device::DeviceID>& missingTargets, unsigned maxRecursions)
{
    unsigned passes = 0;
    bool repeat = false;

    std::set<DeviceID> targets = evtTargets;

    do {
        ++passes;
        repeat = false;

        getDependants(targets, tree, missingTargets, STI::Device::DeviceTrace());

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
void LocalEventEngineScheduler::getDependants(const std::set<DeviceID>& evtTargets, EventEngineDependencyTree& tree, std::set<STI::Device::DeviceID>& missingTargets, const STI::Device::DeviceTrace& trace)
{
	STI::Device::DeviceTrace newTrace;
    if (loopDetected(trace, newTrace)) {
        return;     //avoid infinite recursion if the network graph has a loop
    }

    EventEngineDependencyTree subtree;

    //*** (1) Check for events targeting the local device ***//

    //If there are local events, add local ID *and* this device's event targets, since the local
    //device can generate events on its event targets.
    auto local_it = evtTargets.find(localDevice->getID());

    if (local_it != evtTargets.end()) {
        addDeviceEventTargets(subtree, STI::Device::DeviceTrace());
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
            getPartnerDeviceDependants(*found_it, it->second, tree, missingIDs, newTrace);  //it->second = target list
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
    auto it = targetsByServer.find(localDevice->getID().getID());
    if (it != targetsByServer.end()) {

        //Targets found that need this device as server
        tree.addVertex(localDevice->getID());   //add if not already added

        for (auto& id : it->second) {
            targetsForLocal.clear();
            targetsForLocal.insert(id);

            getPartnerDeviceDependants(id, targetsForLocal, tree, missingIDs, newTrace);
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
        getPartnerDeviceDependants(id, downstreamIDs, tree, missingIDs, newTrace);
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
        if (id != localDevice->getID() && !tree.hasBranchToTarget(localDevice->getID(), id)) {
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

void LocalEventEngineScheduler::cancelJob(const EngineJobID& jobID)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);
    
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
        //job->markCancelled();
        
        if (getManager(jobID, manager) && manager != 0) {
            manager->abortJob();
        }
        
        completedJobs.add(jobID, job);
    }

    jobCondition.notify_all();
}

std::shared_ptr<EventEngineJob> LocalEventEngineScheduler::createJob(const ParseID& parseID, 
                                          const std::shared_ptr<ParsedShot>& shot,
                                          const std::shared_ptr<EventEngineDependencyTree>& tree, 
                                          const STI::Device::DeviceID& owner, 
                                          const std::set<STI::Device::DeviceID>& missingTargets)
{
    auto job = std::make_shared<LocalEventEngineJob>(parseID, shot, tree, owner, missingTargets);
    return job;
}

void LocalEventEngineScheduler::jobComplete(const EngineJobID& jobID)
{
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
  
    std::shared_ptr<EventEngineManager> manager;
    EngineID engineID;

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

        if (freeEngines.size() > 0) {
            
            //First priority is to run a play event if a free engine is parsed for it, regardless of position in set.
            for (auto jobID : queuedJobIDs) {
                
                if (jobID.type == EventEngineJobType::Play 
                        && findParsedEngine(jobID.pid, freeEngines, engineID)
                        && assignJob(jobID, engineID)) 
                {
                    //play job assigned to engineID
                    freeEngines.erase(engineID);
                }
            }

            //assign parse jobs
            for (auto jobID : queuedJobIDs) {
                if (jobID.type == EventEngineJobType::Parse
                        && findOldestParsedEngine(freeEngines, engineID)
                        && assignJob(jobID, engineID)) 
                {
                    freeEngines.erase(engineID);
                } 
            }
        }

        if (queuedJobs.size() == 0 || freeEngines.size() == 0) {
            jobCondition.wait(jobLock);
        }

    } while (running);

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


bool LocalEventEngineScheduler::findParsedEngine(const STI::Engine::ParseID& parsedID, 
                                                 std::set<EngineID>& freeEngines, EngineID& engineID)
{
    std::shared_ptr<EventEngineManager> manager;
    bool found = false;

    for (auto& id : freeEngines) {
        if (engineManagers.get(id, manager) && manager != 0 && manager->isParsed(parsedID)) {
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
 
            //first time through, found == false, so we initial with this timestamp
            if (!found || manager->getLastParseID().parseTimestamp < oldest) {
                oldest = manager->getLastParseID().parseTimestamp;
                engineID = id;
                found = true;
            }
        }
    }

    return found;
}

void LocalEventEngineScheduler::handleEvent(const std::shared_ptr<EngineSchedulerMessage>& evt)
{
    typedef EngineSchedulerMessage::SchedulerMessageType MessageType;

    //ParseComplete, YieldParse, PartialParse, PlayReady, YieldPlay
    
    std::shared_ptr<EventEngineManager> manager;

    switch(evt->schedulerMessageType) {
        case MessageType::ParseComplete:
            if (getManager(evt->jobID, manager)) {
                manager->handleParseMessage(evt);
            }
        break;

        case MessageType::YieldParse:
        break;
        
        case MessageType::PlayReady:
            if (getManager(evt->jobID, manager)) {
                manager->handlePlayMessage(evt);
            }
        break;
    }
}

bool LocalEventEngineScheduler::getManager(const EngineJobID& jobID, std::shared_ptr<EventEngineManager>& manager)
{
    std::shared_ptr<EventEngineJob> job;

    if(runningJobs.get(jobID, job) && job != 0
        && engineManagers.get(job->getEngineID(), manager) && manager != 0) {
        return true;
    }
    return false;
}
