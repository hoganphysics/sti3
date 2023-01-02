
#include "LocalEventEngineDependencyParser.h"

#include "EventEngineDependencyTree.h"

#include <sti/device/DeviceID.h>
#include <sti/device/DeviceTrace.h>

#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/EventEngineScheduler.h>

#include <memory>


using STI::Engine::LocalEventEngineDependencyParser;
using STI::Engine::EventEngineDependencyParser;
using STI::Engine::EventEngineDependencyTree;
using STI::Engine::EventEngineScheduler;

using STI::Device::DeviceID;
using STI::Device::DeviceTrace;


LocalEventEngineDependencyParser::LocalEventEngineDependencyParser(STI::Device::LocalDevice* localDevice)
: localDevice(localDevice)
{
    localDeviceID = localDevice->getID();
    localDevice->getCollection(localCollection);
}

LocalEventEngineDependencyParser::~LocalEventEngineDependencyParser()
{
}

bool LocalEventEngineDependencyParser::getTargetDependencyParser(const DeviceID& id, std::shared_ptr<EventEngineDependencyParser>& dependencyParser)
{
    std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<EventEngineScheduler> scheduler;

    return localCollection != 0 && localCollection->get(id, device) 
            && device != 0 && device->getEngineScheduler(scheduler) && scheduler != 0
            && scheduler->getDependencyParser(dependencyParser);
}



bool LocalEventEngineDependencyParser::loopDetected(const DeviceTrace& trace, DeviceTrace& newTrace)
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


void LocalEventEngineDependencyParser::addDeviceEventTargets(EventEngineDependencyTree& tree, std::vector<EngineParsingMessage>& messages, 
                                                        const STI::Device::DeviceTrace& trace)
{
	STI::Device::DeviceTrace newTrace;
    if (loopDetected(trace, newTrace)) {
        return;     //avoid infinite recursion if the network graph has a loop
    }

    EventEngineDependencyTree subtree;
    std::shared_ptr<EventEngineDependencyParser> dependencyParser;

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
        if (getTargetDependencyParser(targetID, dependencyParser)) {

            std::vector<EngineParsingMessage> partnerMessages;
            
            dependencyParser->addDeviceEventTargets(subtree, partnerMessages, newTrace);

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

std::string LocalEventEngineDependencyParser::findTargetServerID(const STI::Device::DeviceID& deviceID)
{
    auto targetServerID = deviceID.getTargetServerID();

    if (targetServerID.compare("") == 0) {
        //no target server ID specified; attempt to lookup in local device collection

        std::set<DeviceID> ids;
        localCollection->getIDs(ids);

        auto found_it = ids.find(deviceID);

        if (found_it != ids.end()) {
            targetServerID = found_it->getTargetServerID();     //replace with value from collection
        }
    }

    return targetServerID;
}

void LocalEventEngineDependencyParser::addToTargetsByServer(const std::set<DeviceID>& targets, const EventEngineDependencyTree& tree, std::map<std::string, std::set<DeviceID>>& targetsByServer)
{
    //Sort (by server) all targets that are below this device in the graph.
    //That is, ignore targets that have a server path to the localDevice, since the localDevice
    //is not responsible for those targets.

    //Note: if the id is not in the tree, it will be trivially added to set
    for (auto& id : targets) {
        if (id != localDeviceID && !tree.hasBranchToTarget(id, localDeviceID)) {
            //this id has no server path to the local device.
            targetsByServer[findTargetServerID(id)].insert(id);
        }        
    }
}

void LocalEventEngineDependencyParser::getServerChainIDs(std::set<STI::Device::DeviceID>& serverIDs)
{
    //returns list of ids in the localDevice's collection that declare it as their server
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

void LocalEventEngineDependencyParser::getPartnerDeviceDependants(const DeviceID& partnerID, const std::set<DeviceID>& targets, 
                                                    EventEngineDependencyTree& tree, std::set<DeviceID>& missingIDs, 
                                                    std::vector<EngineParsingMessage>& messages, const DeviceTrace& trace)
{
    if (targets.size() == 0) {
        return;
    }

    std::shared_ptr<EventEngineDependencyParser> dependencyParser;

    EventEngineDependencyTree subtree;

    if (getTargetDependencyParser(partnerID, dependencyParser)) {
        
        subtree.clear();
        dependencyParser->getDependants(targets, subtree, missingIDs, messages, trace);
           
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

void LocalEventEngineDependencyParser::getDependants(const std::set<DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
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

        //Remove direct partners from missingTargets, since the localDevice will act as their server
        for (auto it = missingTargets.begin(); it != missingTargets.end(); ) {
            if (localDevice->isEventTarget(*it)) {
                it = missingTargets.erase(it);
            }
            else {
                ++it;
            }
        }

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
void LocalEventEngineDependencyParser::getDependants(const std::set<DeviceID>& evtTargets, EventEngineDependencyTree& tree, 
                                                std::set<STI::Device::DeviceID>& missingTargets, std::vector<EngineParsingMessage>& messages,
                                                const STI::Device::DeviceTrace& trace)
{
	STI::Device::DeviceTrace newTrace;
    if (loopDetected(trace, newTrace)) {
        return;     //avoid infinite recursion if the network graph has a loop
    }

    EventEngineDependencyTree subtree;

    //*** (1) Check for events targeting the local device ***//

    //If there are local events, add local ID *and* this device's event targets (partners), since the local
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
            targetsForLocal.insert(id);     //call as getPartnerDeviceDependants(id, {id}, ...) to just add this id (and it's partners...)

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

void LocalEventEngineDependencyParser::getDownstreamIDs(const std::map<std::string, std::set<DeviceID>> targetsByServer, 
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

