
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
    for (auto& rawTargetID : targetIDs) {

        auto targetID = findCanonicalDeviceID(rawTargetID);

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
                << "(device is likely missing from the network). Parsed shot may be forced to be abstract.";
        }
    }
}

std::string LocalEventEngineDependencyParser::findTargetServerID(const STI::Device::DeviceID& deviceID)
{
    auto localID = findCanonicalDeviceID(deviceID);
    auto targetServerID = localID.getTargetServerID();

    if (targetServerID.compare("") == 0) {
        targetServerID = deviceID.getTargetServerID();
    }

    return targetServerID;
}

DeviceID LocalEventEngineDependencyParser::findCanonicalDeviceID(const STI::Device::DeviceID& deviceID) const
{
    if (localCollection == 0) {
        return deviceID;
    }

    std::set<DeviceID> ids;
    localCollection->getIDs(ids);

    auto found_it = ids.find(deviceID);
    if (found_it != ids.end()) {
        return *found_it;
    }

    if (localDevice->isEventTarget(deviceID) && deviceID.getTargetServerID().empty()) {
        return DeviceID(deviceID.getName(), deviceID.getAddress(), deviceID.getModule(), localDeviceID.getID());
    }

    return deviceID;
}

bool LocalEventEngineDependencyParser::addToTargetsByServer(const std::set<DeviceID>& targets, 
    const EventEngineDependencyTree& tree, std::map<std::string, std::set<DeviceID>>& targetsByServer, std::set<DeviceID>& upstreamTargets,
    bool allowLocalPartnerOwnership)
{
    bool changes = false;
    //Sort (by server) all targets that are below this device in the graph.
    //That is, ignore targets that have a server path to the localDevice, since the localDevice
    //is not responsible for those targets.

    //Note: if the id is not in the tree, it will be trivially added to set
    for (auto& rawID : targets) {
        auto id = findCanonicalDeviceID(rawID);
        auto targetServerID = findTargetServerID(id);

        if (id == localDeviceID) continue;

        if (tree.hasBranchToTarget(id, localDeviceID)) {    //Found path: id -> ... -> ... -> localDevicID
            //this id has no path to the localDeviceID (it is not above localDeviceID in the tree).
            upstreamTargets.insert(id);
        }
        else if (allowLocalPartnerOwnership
            && targetServerID != localDeviceID.getID()
            && tree.hasGraphBranchToTarget(localDeviceID, id)) {
            //The root parser can act as server for its declared partner/event-target edge
            //when the target's declared server is elsewhere. Recursive parsers must keep
            //the targetServerID route so normal server-owned parsing can distribute events.
        }
        else if (!tree.hasBranchToTarget(DeviceID(targetServerID), id)) {    //Query path: id.targetServerID -> ... -> ... -> id
            //id's targretServerID is not in the tree, or is not connected to id
            targetsByServer[targetServerID].insert(id);
            changes = true;
        }
    }
    return changes;
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

void LocalEventEngineDependencyParser::getDeviceDependants(const DeviceID& deviceID, const std::set<DeviceID>& targets, 
                                                    EventEngineDependencyTree& tree, std::set<DeviceID>& unownedTargets,
                                                    std::vector<EngineParsingMessage>& messages, const DeviceTrace& trace)
{
    if (targets.size() == 0) {
        return;
    }

    std::shared_ptr<EventEngineDependencyParser> dependencyParser;

    EventEngineDependencyTree subtree;

    if (getTargetDependencyParser(deviceID, dependencyParser)) {
        
        subtree.clear();
        dependencyParser->getDependants(targets, subtree, unownedTargets, messages, trace);
           
        //Add found subtree to the tree.
        //Uses greater than 1 because the subtree always contains the server id, but we only add if there are also others.
        //(Unless the partnerID is in the target list)
        if (subtree.vertexCount() > 1 || targets.find(deviceID) != targets.end()) {
            tree.addTree(subtree);
            tree.addEdge(localDeviceID, deviceID);
        }
    }
    else {
        //Could not contact the device; these targets cannot be reached
        unownedTargets.insert(targets.begin(), targets.end());
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
    bool allowLocalPartnerOwnership = (trace.size() == 0);

	STI::Device::DeviceTrace newTrace;
    if (loopDetected(trace, newTrace)) {
        return;     //avoid infinite recursion if the network graph has a loop
    }

    //*** (1) Check for events targeting the local device ***//

    //If there are local events or events for a declared local event target, add this device's
    //event targets (partners). The local device can act as the server for those targets.
    bool includeLocalEventTargets = evtTargets.contains(localDeviceID);
    if (!includeLocalEventTargets) {
        for (auto& id : evtTargets) {
            if (localDevice->isEventTarget(id)) {
                includeLocalEventTargets = true;
                break;
            }
        }
    }

    if (includeLocalEventTargets) {
        EventEngineDependencyTree subtree;
        addDeviceEventTargets(subtree, messages, STI::Device::DeviceTrace());
        tree.addTree(subtree);
    }

    //Now all partners for local device are added; but there are two issues:
    // 1) There are event targets in evtTargets that have not been found via a server chain (which server owns them?)
    // 2) There are targets in the current tree that don't have their server chain added (partners owned by a different device than localDeviceID)


    //*** (2) Sort event targets by server ***//

    std::set<DeviceID> upstreamTargets;

    std::map<std::string, std::set<DeviceID>> targetsByServer;   // ["server", {devices}]
    addToTargetsByServer(evtTargets, tree, targetsByServer, upstreamTargets, allowLocalPartnerOwnership);

    std::set<DeviceID> localTargets;    //for targets originating from addDeviceEventTargets above that may have a different targetServerID
    tree.getNodes(localTargets);
    addToTargetsByServer(localTargets, tree, targetsByServer, upstreamTargets, allowLocalPartnerOwnership);

   
    //*** (3) Pass targets down the server chain, including localDeviceID if it is a targetServer ***//

    //Server chain: get all connected devices that declare the local device as server (first link in chain)
    std::set<DeviceID> serverChainIDs;
    getServerChainIDs(serverChainIDs);
    
    std::set<DeviceID> unownedIDs;
    std::set<DeviceID> targetsForLocal;
    
    //Pass all targetsByServer['ID'] target lists to any 'ID' found in serverChainIDs.
    for (auto it = targetsByServer.begin(); it != targetsByServer.end(); ) {
        unownedIDs.clear();

        // Check if it=targetsByServer['ID'] is in serverChainIDs:
        auto subServerID = std::find_if(serverChainIDs.begin(), serverChainIDs.end(),
                        [&](const DeviceID& id) { return (it->first == id.getID()); });
        
        if (it->first == localDeviceID.getID()) {
            //Targets were found that need localDeviceID as server
            tree.addVertex(localDeviceID);   //in case not already added

            for (auto& id : it->second) {
                targetsForLocal.clear();
                targetsForLocal.insert(id);
                
                if (tree.hasVertex(id)) {
                    tree.addEdge(localDeviceID, id);
                }
                else {
                    //call as getDeviceDependants(id, {id}, ...) to just add this id (and its partners...)
                    getDeviceDependants(id, targetsForLocal, tree, unownedIDs, messages, newTrace);
                }
            }
            it = targetsByServer.erase(it);     //remove after attempt to transfer
        }
        else if (subServerID != serverChainIDs.end()) {
            //Found targets for a subserver connected to localDeviceID
            //Pass targetsByServer['ID'] target list to the locally connected server
            getDeviceDependants(*subServerID, it->second, tree, unownedIDs, messages, newTrace);  //it->second = target list

            it = targetsByServer.erase(it);     //remove after attempt to transfer
        }
        else {
            ++it;
        }

        if (unownedIDs.size() != 0) {
            //Any unownedIDs may be new and might require another pass
            if (addToTargetsByServer(unownedIDs, tree, targetsByServer, upstreamTargets, allowLocalPartnerOwnership)) {
                //Tree has changed => new server found; start over
                it = targetsByServer.begin();
            }
        }

        //if (!parsing) break;    //abort
    }

    //*** (4) Search the full graph for any missing targets, following server chain ***//

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

    //Todo: filter serverChainIDs to get rid of STIpy, etc

    std::set<DeviceID> filteredServerChainIDs;
    
    std::copy_if(serverChainIDs.begin(), serverChainIDs.end(), std::inserter(filteredServerChainIDs, filteredServerChainIDs.end()),
        [](const DeviceID& id)->bool {
            std::size_t prefixPos = id.getName().find("STIPy:");
            return (prefixPos != 0);
        });
    
    for (auto& id : filteredServerChainIDs) {    // Follow server chain through the graph
        if (id.getTargetServerID() == localDeviceID.getTargetServerID()) {
            //obvious shortcircuit to save time
            continue;
        }
        unownedIDs.clear();
        getDeviceDependants(id, downstreamIDs, tree, unownedIDs, messages, newTrace);
        downstreamIDs.swap(unownedIDs);

        if (downstreamIDs.size() == 0) break;
    }

    missingTargets.insert(upstreamTargets.begin(), upstreamTargets.end());

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
}

