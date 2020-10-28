

#include "EventEngineScheduler.h"
#include "DeviceID.h"
#include "SynchronizedMap.h"
#include "EventEngineJob.h"
#include "DeviceEvent.h"
#include "EventEngineManager.h"

#include <set>
#include <vector>
#include <memory>
#include <algorithm>


using STI::Engine::EventEngineJob;
using STI::Engine::EventEngineScheduler;
using STI::Engine::EventEngineDependencyTree;
using STI::Device::DeviceID;
using STI::Device::EngineSchedulerMessage;
using STI::Engine::EngineID;
using STI::Engine::EventEngine;

EventEngineScheduler::EventEngineScheduler(STI::Device::LocalDevice* localDevice)
: localDevice(localDevice), completedJobs(3)
{
    completedJobs.setMaxSize(3);

    running = true;
    schedulerThread = std::thread(&EventEngineScheduler::assignJobs, this);
}

EventEngineScheduler::~EventEngineScheduler()
{
    stop();
    schedulerThread.join();
}

void EventEngineScheduler::stop()
{
    std::unique_lock<std::mutex> jobLock(jobMutex);
    running = false;

    jobCondition.notify_all();
}

void EventEngineScheduler::addEngine(const EngineID& engineID, const std::shared_ptr<EventEngine>& engine)
{
    auto manager = std::make_shared<EventEngineManager>(engine, this);

    engineManagers.add(engineID, manager);
}

void EventEngineScheduler::parse(const ParseID& parseID, const std::shared_ptr<ParsedShot>& shot)
{
    if(shot == 0) return;

    //Resolve all parsed events, generating RawEvent list (in python?)

    //Get list unique device targets
    std::set<DeviceID> eventTargets;

    for(auto& evt : shot->events) {
        eventTargets.insert(evt.targetDevice());
    }
    
    STI::Device::DeviceTrace trace;     //Trace needed to avoid infinite recursion while mapping the network

    //Create dependency tree
    auto tree = std::make_shared<EventEngineDependencyTree>();
    getDependants(eventTargets, *tree, trace);

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
    auto job = std::make_shared<EventEngineJob>(parseID, shot, tree, localDevice->getID(), diff);

    addJob(job);

}

/// Generates a graph of the network that contains all devices needed to parse the events.
/// Does this in three steps:  (1) Local device, (2) Direct device decendents, (3) Full depth recursive search of graph.
void EventEngineScheduler::getDependants(std::set<DeviceID>& evtTargets, EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace)
{
    //getDependants() is recursive; avoid infinite loop by using DeviceTrace
    if (trace.includesID(localDevice->getID())) {
        //getDependants has already been to this device; short circuit the call (the network graph has a loop)
        return;
    }


    //*** (1) Events targeting the local device ***//

    //If there are local events, add local ID *and* this device's event targets
    bool localVertexAdded = false;
    auto local_it = evtTargets.find(localDevice->getID());

    if (local_it != evtTargets.end()) {
        
        //Add local device to the tree
        tree.addVertex(localDevice->getID());
        localVertexAdded = tree.hasVertex(localDevice->getID());
        
        //Get all event targets that were explicitly declared by this device
        std::set<STI::Device::DeviceID> targetIDs;
        localDevice->getEventTargets(targetIDs);
        
        //Add local event targets (the local device can generate events for these)
        for (auto& targetID : targetIDs) {
            tree.addEdge(localDevice->getID(), targetID);
        }
    }


    //*** (2) Events targeting devices that directly list the local device is their server ***//

   	bool isServerOfEventTarget;
    std::set<DeviceID> missingTargets;  //for event targets that are not directly connected to this device

    for(auto& id : evtTargets) {
        
        if (id != localDevice->getID()) {

        //    auto it = targetIDs.find(id);

            //Devices that have this device as target server
            isServerOfEventTarget = localDevice->getID().getID() == id.getTargetServerID();
        //                || (it != targetIDs.end());

            if (isServerOfEventTarget) {
                
                if (!localVertexAdded) {
                    tree.addVertex(localDevice->getID());
                    localVertexAdded = tree.hasVertex(localDevice->getID());
                }
                tree.addEdge(localDevice->getID(), id);
            }
            else {
                missingTargets.insert(id);      //Event targets not found at this graph location
            }
        }
	}

    //*** (3) Search recursively down the network graph for any missing target devices  ***//

    findMissingTarget(missingTargets, tree, trace);     //Note: modifies 'tree' by reference when targets are found

    // if (!localEventsFound) {     //First time only
    //     for (auto& targetID : targetIDs) {
    //         tree.addEdge(localDevice->getID(), targetID);
    //     }
    // }


}

void EventEngineScheduler::findMissingTarget(std::set<DeviceID>& missingTargets, EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace)
{
    if (missingTargets.size() == 0) {
        return;
    }

    EventEngineDependencyTree subtree;
    std::set<STI::Device::DeviceID> ownedIDs;
    std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    //Need to search all currently connected devices (full graph search) because the events targets
    //could be several layers deep.
    std::shared_ptr<STI::Device::DeviceCollection> localCollection;
    localDevice->getCollection(localCollection);
    localCollection->getIDs(ownedIDs);

	//Append this ID to the trace before passing down the graph to avoid infinite loop
	STI::Device::DeviceTrace newTrace = trace;
    newTrace.addID(localDevice->getID());

    //Resolve missing targets by passing them down the network.
    for(auto& id : ownedIDs) {
        subtree.clear();

        if(localCollection->get(id, device) && device != 0 && 
                device->getEngineScheduler(scheduler) ) {
            
            scheduler->getDependants(missingTargets, subtree, newTrace);

            if(subtree.hasVertex(id)) {     //The subtree will only have id if _some_ device in the subtree is a missing target 
                tree.addTree(subtree);
                tree.addEdge(localDevice->getID(), id);     //temp!!  Need to conditionally add the local vertex too, based on whether event targets are found below it in the graph
            }

            //Remove any missing targets found in subtree
            // missingTargets.erase(
            //     std::remove_if(missingTargets.begin(), missingTargets.end(), 
            //     [&subtree](const DeviceID& id){ return subtree.hasVertex(id); }),
            //     missingTargets.end());
            
            //Remove any missing targets found in subtree to speedup subsequent searching
            for (auto it = missingTargets.begin(); it != missingTargets.end(); ) {
                if (subtree.hasVertex(id)) {
                    it = missingTargets.erase(it);
                }
                else {
                    ++it;
                }
            }
        }
    }

        // //Stop the search once all targets have been found
        // if(missingTargets.size() == 0) {
        //     break;
        // }
}

void EventEngineScheduler::addJob(const std::shared_ptr<EventEngineJob>& newJob)
{
    std::unique_lock<std::mutex> jobLock(jobMutex);
        
    if (newJob != 0) {
        queuedJobs.add(newJob->getJobID(), newJob);
    }

    jobCondition.notify_all();
}

void EventEngineScheduler::jobComplete(const EngineJobID& jobID)
{
    std::shared_ptr<EventEngineJob> job;
    
    if (runningJobs.get(jobID, job) && job != 0) {
        runningJobs.remove(jobID);
        completedJobs.add(jobID, job);
    }

    jobCondition.notify_all();
}

void EventEngineScheduler::assignJobs()
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

bool EventEngineScheduler::assignJob(const EngineJobID& jobID, const EngineID& engineID)
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

bool EventEngineScheduler::findParsedEngine(const STI::Engine::ParseID& parsedID, 
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

                                                 
bool EventEngineScheduler::findOldestParsedEngine(std::set<EngineID>& freeEngines, EngineID& engineID)
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

void EventEngineScheduler::handleEvent(const std::shared_ptr<EngineSchedulerMessage>& evt)
{
    typedef EngineSchedulerMessage::SchedulerMessageType MessageType;

    //ParseComplete, YieldParse, PartialParse, PlayReady, YieldPlay
    
    std::shared_ptr<EventEngineJob> job;
    std::shared_ptr<EventEngineManager> manager;

    switch(evt->type) {
        case MessageType::ParseComplete:
            
            if(runningJobs.get(evt->jobID, job) && job != 0
                && engineManagers.get(job->getEngineID(), manager) && manager != 0) {
                manager->handleParseMessage(evt);
            }
            
        break;
        case MessageType::YieldParse:
        break;
    }
}