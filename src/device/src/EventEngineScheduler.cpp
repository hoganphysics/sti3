

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

void EventEngineScheduler::parse(const ParseID& parseID, const std::shared_ptr<ParsedShot>& shot)
{
    if(shot == 0) return;

    //Resolve all parsed events, generating RawEvent list (in python?)

    //Get list unique device targets
    std::set<DeviceID> eventTargets;

    for(auto& evt : shot->events) {
        eventTargets.insert(evt.targetDevice());
    }
    
    //Create dependency tree
    auto tree = std::make_shared<EventEngineDependencyTree>();
    getDependants(eventTargets, *tree);

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

void EventEngineScheduler::getDependants(std::set<DeviceID>& evtTargets, EventEngineDependencyTree& tree)
{
    tree.addVertex(localDevice->getID());

    std::set<DeviceID> missingTargets;  //for event targets that are not directly connected to this device

    //All the event targets that were explicitly declared by this device
	std::set<STI::Device::DeviceID> targetIDs;
    localDevice->getEventTargets(targetIDs);

   	bool isTarget;

    //First add all targets that are connected locally
    for(auto& id : evtTargets) {

        auto it = targetIDs.find(id);

        //Explicitly declared targets, or devices that have this device as target server
		isTarget = localDevice->getID().getID() == id.getTargetServerID() 
					|| (it != targetIDs.end());

		if (isTarget) {
			tree.addEdge(localDevice->getID(), id);
		}
		else {
			//call to other devices
            missingTargets.insert(id);
		}
	}
    
    std::shared_ptr<STI::Device::DeviceCollection> localCollection;
    localDevice->getCollection(localCollection);

    
    EventEngineDependencyTree subtree;
    std::set<STI::Device::DeviceID> ownedIDs;
    std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    //Need to search all currently connected devices (full graph search) because the events targets
    //could be several layers deep.
    localCollection->getIDs(ownedIDs);

    //Resolve missing targets by passing them down the network.
    for(auto& id : ownedIDs) {
        subtree.clear();

        if(localCollection->get(id, device) && device != 0 && 
                device->getEngineScheduler(scheduler) ) {
            
            scheduler->getDependants(missingTargets, subtree);

            tree.addTree(subtree);

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

        //Stop the search once all targets have been found
        if(missingTargets.size() == 0) {
            break;
        }
    }
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