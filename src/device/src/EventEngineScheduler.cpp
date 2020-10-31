

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
    
    //Could wrap this in a loop for multipass searches.  Keep calling until the tree is static, or missing targets is static.  tree.getNodes() size?
    std::set<DeviceID> missingTargets;
    getDependants(eventTargets, *tree, missingTargets, trace);

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

// bool addTarget(const DeviceID& targetID, EventEngineDependencyTree& tree, const std::set<DeviceID>& remainingTargets, const STI::Device::DeviceTrace& trace)
// {
//     tree.clear();

//     EventEngineDependencyTree subtree;
//     std::set<STI::Device::DeviceID> ownedIDs;
//     std::shared_ptr<STI::Device::Device> device;
//     std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

// //    remainingTargets.insert(targetID);

//     //attempt to get this device reference
//     std::shared_ptr<STI::Device::DeviceCollection> localCollection;
//     localDevice->getCollection(localCollection);
//     localCollection->getIDs(ownedIDs);

//     for(auto& id : ownedIDs) {
//         subtree.clear();

//         if(localCollection->get(id, device) && device != 0 && 
//                 device->getEngineScheduler(scheduler) ) {
            
//             scheduler->getDependants(missingTargets, subtree, newTrace);

//             if(subtree.hasVertex(id)) {     //The subtree will only have id if _some_ device in the subtree is a missing target 
//                 tree.addTree(subtree);
//                 tree.addEdge(localDevice->getID(), id);     //temp!!  Need to conditionally add the local vertex too, based on whether event targets are found below it in the graph
//             }

//             //Remove any missing targets found in subtree
//             // missingTargets.erase(
//             //     std::remove_if(missingTargets.begin(), missingTargets.end(), 
//             //     [&subtree](const DeviceID& id){ return subtree.hasVertex(id); }),
//             //     missingTargets.end());
            
//             //Remove any missing targets found in subtree to speedup subsequent searching
//             for (auto it = missingTargets.begin(); it != missingTargets.end(); ) {
//                 if (subtree.hasVertex(id)) {
//                     it = missingTargets.erase(it);
//                 }
//                 else {
//                     ++it;
//                 }
//             }
//         }
//     }

//     //getDependants(remainingTargets, tree, trace)
//     //remove targetID from remainingTargets, if present
//     //get all ids in tree, and remove them (if present) from the remainingTargets
//     //go through all local connected devices and check recursively if they are the server of any id in the tree without a server
// }

void EventEngineScheduler::addDevice(EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace)
{
    //addTargetDevice() is recursive; avoid infinite loop by using DeviceTrace
    if (trace.includesID(localDevice->getID())) {
        //addTargetDevice has already been to this device; short circuit the call (the network graph has a loop)
        return;
    }

    //Append this ID to the trace before passing down the graph to avoid infinite loop
	STI::Device::DeviceTrace newTrace = trace;
    newTrace.addID(localDevice->getID());

//    bool success = false;

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
            scheduler->addDevice(subtree, newTrace);
            tree.addTree(subtree);
        }
        else {
            //Warning, event target device not connected
        }
    }

//    return success;
}

void EventEngineScheduler::addToTargetsByServer(const std::set<DeviceID>& targets, const EventEngineDependencyTree& tree, std::map<std::string, std::set<DeviceID>>& targetsByServer)
{
    //Note: if the id is not in the tree, it will be trivially added to set
    for (auto& id : targets) {
        if (id !=localDevice->getID() && !tree.hasBranchToTarget(id, localDevice->getID())) {
            //this id has no server path to the local device.
//            idsWithMissingServers.insert(id);
            targetsByServer[id.getTargetServerID()].insert(id);
        }        
    }
}

void EventEngineScheduler::getServerIDs(std::set<STI::Device::DeviceID>& serverIDs)
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

/// Generates a graph of the network that contains all devices needed to parse the events.
/// Does this in three steps:  (1) Local device, (2) Direct device decendents, (3) Full depth recursive search of graph.
void EventEngineScheduler::getDependants(const std::set<DeviceID>& evtTargets, EventEngineDependencyTree& tree, std::set<STI::Device::DeviceID>& missingTargets, const STI::Device::DeviceTrace& trace)
{
    //getDependants() is recursive; avoid infinite loop by using DeviceTrace
    if (trace.includesID(localDevice->getID())) {
        //getDependants has already been to this device; short circuit the call (the network graph has a loop)
        return;
    }

    //Append this ID to the trace before passing down the graph to avoid infinite loop
	STI::Device::DeviceTrace newTrace = trace;
    newTrace.addID(localDevice->getID());

    EventEngineDependencyTree subtree;

//    std::set<DeviceID> remainingTargets = evtTargets;

    //*** (1) Events targeting the local device ***//

    //If there are local events, add local ID *and* this device's event targets
    bool localVertexAdded = false;
    auto local_it = evtTargets.find(localDevice->getID());

    if (local_it != evtTargets.end()) {
        addDevice(subtree, STI::Device::DeviceTrace());
        tree.addTree(subtree);
    }


    //Now all partners for local device are added; but there are two problems:
    // 1) There are event targets in evtTargets that have not been found via a server chain
    // 2) There are targets in the current tree that don't have their server chain added

// construnct targetsWeNeedToAddServerChainFor = evtTargets + tree.nodes - localDeviceID
// desend via connected devices that have localdevice as server, sending targetsWeNeedToAddServerChainFor
// first add any connected device like this and remove from targetsWeNeedToAddServerChainFor
// Then send remaining list down each server recursively

//complication:  for evtTargets, we need to addTargetDevice (partner events), for server chains, we just need to addEdge.

    std::map<std::string, std::set<DeviceID>> targetsByServer;   // ["server", {devices}]
    std::set<DeviceID> localTargets;
    tree.getNodes(localTargets);

//     std::set<DeviceID> idsWithMissingServers;

//     //Note: if the id is not in the tree, it will be trivially added to set
//     for (auto& id : localTargets) {
//         if (id !=localDevice->getID() && !tree.hasBranchToTarget(id, localDevice->getID())) {
//             //this id has no server path to the local device.
// //            idsWithMissingServers.insert(id);
//             targetsByServer[id.getTargetServerID()].insert(id);
//         }        
//     }
   
//     for (auto& id : evtTargets) {
//         if (id !=localDevice->getID() && !tree.hasBranchToTarget(id, localDevice->getID())) {
//             //this id has no server path to the local device.
// //            idsWithMissingServers.insert(id);
//             targetsByServer[id.getTargetServerID()].insert(id);


//         }        
//     }
    
    addToTargetsByServer(localTargets, tree, targetsByServer);
    addToTargetsByServer(evtTargets, tree, targetsByServer);


    //std::set<STI::Device::DeviceID> ownedIDs;
    std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
	
    //Need to search all currently connected devices (full graph search) because the events targets
    //could be several layers deep.
    std::shared_ptr<STI::Device::DeviceCollection> localCollection;
    localDevice->getCollection(localCollection);
    //localCollection->getIDs(ownedIDs);
   
   
    std::set<STI::Device::DeviceID> missingIDs;
    missingIDs.clear();


    std::set<STI::Device::DeviceID> serverIDs;  //all the local ids that are connected to this device as their server (server path)
    getServerIDs(serverIDs);
    
    // for (auto& id : ownedIDs) {
    //     if (localDevice->getID().getID() == id.getTargetServerID()) {
    //         serverIDs.insert(id);
    //     }
    // }

    //Check if the target servers are owned by this device; if so, pass along these references
    for (auto it = targetsByServer.begin(); it != targetsByServer.end(); ) {

        auto found_it = std::find_if(serverIDs.begin(), serverIDs.end(),
                        [&](const DeviceID& id) { return (it->first == id.getID()); });
        
        if (found_it != serverIDs.end()) {

            //Local device is the server of some targets; send these targets to the device
            if(localCollection->get(*found_it, device) && device != 0 && device->getEngineScheduler(scheduler)) {

                subtree.clear();
                //bool tmp = subtree.hasVertex(localDevice->getID());
                scheduler->getDependants(it->second, subtree, missingIDs, newTrace);
                tree.addTree(subtree);
                tree.addEdge(localDevice->getID(), *found_it);
            }
            else {
                //Warning, event target device not connected

                //Could not contact server; these targets cannot be reached
                missingTargets.insert(it->second.begin(), it->second.end());
            }
            
            //After attempt to transfer these targets, remove from map.
            it = targetsByServer.erase(it);
        }
        else {
            ++it;
        }
    }

    addToTargetsByServer(missingIDs, tree, targetsByServer);


    std::set<DeviceID> targetsForServer;    
    missingIDs.clear();

    //Add any targets that declare this as their server
    auto it = targetsByServer.find(localDevice->getID().getID());
    if (it != targetsByServer.end()) {

        //Targets found that need this device as server
        tree.addVertex(localDevice->getID());   //add if not already added

        for (auto& id : it->second) {
            targetsForServer.clear();

            if (!tree.hasBranchToTarget(localDevice->getID(), id)) {    //not currently connected in tree
                if(localCollection->get(id, device) && device != 0 && device->getEngineScheduler(scheduler)) {
                    
                    subtree.clear();
                    targetsForServer.insert(id);
                    scheduler->getDependants(targetsForServer, subtree, missingIDs, newTrace);
                    tree.addTree(subtree);
                    tree.addEdge(localDevice->getID(), id);
                    
                }
                else {
                    //couldn't connect
                    missingIDs.insert(id);
                }
            }
        }
        
        targetsByServer.erase(it);
    }

    addToTargetsByServer(missingIDs, tree, targetsByServer);

    //Any targets in targetsByServer could not be found at this layer.  Pass them downstream to the network, following server path.

    std::set<STI::Device::DeviceID> downstreamIDs;
    std::set<STI::Device::DeviceID> foundIDs;
    std::set<DeviceID> diff;

    for (auto& it : targetsByServer) {
        downstreamIDs.insert(it.second.begin(), it.second.end());
    }
    targetsByServer.clear(); 

    //Add any with missing server path
    std::set<STI::Device::DeviceID> allIDs;
    tree.getNodes(allIDs);

    for(auto& id : allIDs) {
        if (id != localDevice->getID() && !tree.hasBranchToTarget(localDevice->getID(), id)) {
            downstreamIDs.insert(id);
        }
    }

   

    for (auto& id : serverIDs) {    // Follow server path through graph
        if(localCollection->get(id, device) && device != 0 && device->getEngineScheduler(scheduler)) {

            subtree.clear();
            missingIDs.clear();
            foundIDs.clear();
            diff.clear();

            scheduler->getDependants(downstreamIDs, subtree, missingIDs, newTrace);
            missingTargets.insert(missingIDs.begin(), missingIDs.end());    //anything not found now is declared missing (on this pass)

            //remove any found ids from downstreamIDs
            subtree.getNodes(foundIDs);
           
            if (foundIDs.size() > 1) {
                tree.addTree(subtree);
                tree.addEdge(localDevice->getID(), id);                
            }
            
            std::set_difference(downstreamIDs.begin(), downstreamIDs.end(), 
                                foundIDs.begin(), foundIDs.end(),
                                std::inserter(diff, diff.end()));
            downstreamIDs.swap(diff);
            


        }
    }

    //Anything left is missing; may be reachable with another pass
    missingTargets.insert(downstreamIDs.begin(), downstreamIDs.end());


// ////////////////old////////////////////

//     for (auto& id : ownedIDs) {
        
//         if (localDevice->getID().getID() == id.getTargetServerID()) {   // Follow server path through graph

//             if(localCollection->get(id, device) && device != 0 && 
//                 device->getEngineScheduler(scheduler)) {

//                     subtree.clear();
//                     scheduler->getDependants(evtTargets, subtree, newTrace);
//                     tree.addTree(subtree);
//             }
//             else {
//                 //Warning, event target device not connected
//             }




//             auto it = evtTargets.find(id);
//             if (it != evtTargets.end()) {
                

//             }

//             // // ****** this part may even be redundant -- it happens at the start of getDependants, call right after this...
//             // auto it = evtTargets.find(id);
//             // if (it != evtTargets.end()) {    //id is an event target
                
//             //     tree.addEdge(localDevice->getID(), id);

//             //     // if(addTargetDevice(id, subtree, STI::Device::DeviceTrace())) {
//             //     //     tree.addTree(subtree);
//             //     // }
                
//             //     subtree.clear();

//             //     //Attempt to get this device reference and then get it's subtree
//             //     if(localCollection->get(id, device) && device != 0 && 
//             //             device->getEngineScheduler(scheduler) &&
//             //             scheduler->addTargetDevice(id, subtree, STI::Device::DeviceTrace())) 
//             //     {
//             //         tree.addTree(subtree);
//             //     }
//             //     else {
//             //         //Warning, event target device not connected
//             //     }
//             // }

//             //look for targetsWeNeedToAddServerChainFor down the graph

//             //construct devicesMissingServers == all evt targets plus any id in tree not connected to localDevice via servers
//             scheduler->getDependants(devicesMissingServers, subtree, newTrace);
//             if (devices found) { //subtree has nodes
//                 //add subtree and this id
//                 tree.addEdge(localDevice->getID(), id);
//                 tree.addTree(subtree);
//             }
//         }
//     }


    // if (local_it != evtTargets.end()) {

    //     remainingTargets.erase(local_it);
        
    //     //Add local device to the tree
    //     tree.addVertex(localDevice->getID());
    //     localVertexAdded = tree.hasVertex(localDevice->getID());
        
    //     //Get all event targets that were explicitly declared by this device
    //     std::set<STI::Device::DeviceID> targetIDs;
    //     localDevice->getEventTargets(targetIDs);
        
    //     //Add local event targets (the local device can generate events for these)
    //     for (auto& targetID : targetIDs) {

    //         if (addTarget(targetID, subtree, remainingTargets, newTrace)) {     //modifies remainingTargets by reference
    //             tree.addTree(subtree);
    //             tree.addEdge(localDevice->getID(), targetID);
    //         }
    //         else {
    //             //Warning, event target device not connected
    //         }

    //         //tree.addEdge(localDevice->getID(), targetID);
    //     }
    // }


    // //*** (2) Events targeting devices that directly list the local device is their server ***//

   	// bool isServerOfEventTarget;
    // std::set<DeviceID> missingTargets;  //for event targets that are not directly connected to this device

    // for(auto& id : evtTargets) {
        
    //     if (id != localDevice->getID()) {

    //     //    auto it = targetIDs.find(id);

    //         //Devices that have this device as target server
    //         isServerOfEventTarget = localDevice->getID().getID() == id.getTargetServerID();
    //     //                || (it != targetIDs.end());

    //         if (isServerOfEventTarget) {
                
    //             if (!localVertexAdded) {
    //                 tree.addVertex(localDevice->getID());
    //                 localVertexAdded = tree.hasVertex(localDevice->getID());
    //             }
    //             tree.addEdge(localDevice->getID(), id);
    //         }
    //         else {
    //             missingTargets.insert(id);      //Event targets not found at this graph location
    //         }
    //     }
	// }

    // //*** (3) Search recursively down the network graph for any missing target devices  ***//

    // findMissingTarget(missingTargets, tree, trace);     //Note: modifies 'tree' by reference as targets are found

    // // if (!localEventsFound) {     //First time only
    // //     for (auto& targetID : targetIDs) {
    // //         tree.addEdge(localDevice->getID(), targetID);
    // //     }
    // // }


}

// void EventEngineScheduler::findMissingTarget(const std::set<DeviceID>& missingTargets, EventEngineDependencyTree& tree, const STI::Device::DeviceTrace& trace)
// {
//     if (missingTargets.size() == 0) {
//         return;
//     }

//     EventEngineDependencyTree subtree;
//     std::set<STI::Device::DeviceID> ownedIDs;
//     std::shared_ptr<STI::Device::Device> device;
//     std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
	
//     //Append this ID to the trace before passing down the graph to avoid infinite loop
// 	STI::Device::DeviceTrace newTrace = trace;
//     newTrace.addID(localDevice->getID());

//     //Need to search all currently connected devices (full graph search) because the events targets
//     //could be several layers deep.
//     std::shared_ptr<STI::Device::DeviceCollection> localCollection;
//     localDevice->getCollection(localCollection);
//     localCollection->getIDs(ownedIDs);

//     //Resolve missing targets by passing them down the network.
//     for(auto& id : ownedIDs) {
//         subtree.clear();

//         if(localCollection->get(id, device) && device != 0 && 
//                 device->getEngineScheduler(scheduler) ) {
            
//             scheduler->getDependants(missingTargets, subtree, newTrace);

//             if(subtree.hasVertex(id)) {     //The subtree will only have id if _some_ device in the subtree is a missing target 
//                 tree.addTree(subtree);
//                 tree.addEdge(localDevice->getID(), id);     //temp!!  Need to conditionally add the local vertex too, based on whether event targets are found below it in the graph
//             }

//             //Remove any missing targets found in subtree
//             // missingTargets.erase(
//             //     std::remove_if(missingTargets.begin(), missingTargets.end(), 
//             //     [&subtree](const DeviceID& id){ return subtree.hasVertex(id); }),
//             //     missingTargets.end());
            
//             //Remove any missing targets found in subtree to speedup subsequent searching
//             for (auto it = missingTargets.begin(); it != missingTargets.end(); ) {
//                 if (subtree.hasVertex(id)) {
//                     it = missingTargets.erase(it);
//                 }
//                 else {
//                     ++it;
//                 }
//             }
//         }
//     }

//         // //Stop the search once all targets have been found
//         // if(missingTargets.size() == 0) {
//         //     break;
//         // }
// }

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