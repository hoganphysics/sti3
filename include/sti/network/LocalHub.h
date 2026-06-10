#ifndef STI_NETWORK_LOCALHUB_H
#define STI_NETWORK_LOCALHUB_H

#include <sti/network/Hub.h>
//#include <sti/utils/Collector.h>
#include <sti/utils/Distributer.h>
//#include <sti/network/Node.h>
#include <sti/network/HubTrace.h>
#include <sti/network/HubID.h>

#include <memory>
#include <mutex>


namespace STI
{
namespace Network
{

/*!
Implements the Hub interface to realize a local Hub.  LocalHub stores references to nodes
running locally (in the same process). It can be connected to other Hubs (either local or remote).  When other
Hubs are connected, the LocalHub distributes its locally stored Node references to these other Hubs by 
offering their Node IDs. It also accepts references from connected Hubs and offers them to each of its stored 
Node references.

@tparam ID The Node ID used to uniquely identify Nodes in the network.
@tparam T The Node type stored and distributed by this Hub.
*/
template<class ID, class T>
class LocalHub : public Hub<ID, T>
{
public:

	LocalHub(const STI::Network::HubID& hubID);
	virtual ~LocalHub();

	//local only

	//Add a Node to this Hub.  The new Node will be added to the list of available Nodes
	//owned by this Hub and to the list of Node Collectors managed by this Hub.  The Node will
	//also be distributed to all the existing Collectors managed by this Hub.  Finally,
	//the Node will be distributed to all the Hubs connected to this Hub.
	bool addNode(const ID& id, const typename std::shared_ptr<T>& node);
	bool removeNode(const ID& id);
	bool getNode(const ID& id, typename std::shared_ptr<T>& node) const;

	///Get node IDs stored by this Hub.
	void getNodeIDs(std::set<ID>& ids) const;
	unsigned numberOfNodes() const { return nodeDistributer.numberOfNodes(); }
	bool hasNodeID(const ID& id) const;

	void getHubIDs(std::set<HubID>& ids) const { hubs.getKeys(ids); }
	bool containsHub(const HubID& hid) const { return hubs.contains(hid); }

	//local and remote, but non propagating (not trail tracked)

	//! Connect another Hub to this Hub.
	bool addHub(const HubID& id, const typename std::shared_ptr<Hub<ID, T>>& hub);
	bool removeHub(const HubID& id);
	bool getHub(const HubID& id, typename std::shared_ptr<Hub<ID, T>>& hub) const
	{
		return hubs.get(id, hub) && hub != nullptr;
	}

	//local and remote; trail tracked
	bool removeNode(const ID& id, const HubTrace& trace);
	bool refresh();		//for initiating a refresh
	bool refresh(const HubTrace& trace);		//local and remote	

												//remote; trail tracked
	bool distribute(const ID& id, const typename std::shared_ptr<T>& node, const HubTrace& trace, const HubID& first);	//remote add (called by other hubs offering a reference) � trail tracked
	
	//Request that Nodes be distributed on targetHub (if target hub is connected to this hub)
	bool distributeNodes(const HubID& targetHub); //, const HubTrace& trace);		//distribute all local nodes to target hub

												  //Force redistribution of all Nodes owned by this Hub to all connected Hubs.
	bool redistributeNodes(const HubTrace& trace);		//distribute all owned Nodes to all connected Hubs

	void clear();

	const HubID& getID() const { return hubID; }
	void setID(const STI::Network::HubID& newHubID) { hubID = newHubID; }

	void disconnect(const HubID& hid);
	void disconnect();

	bool ping() const { return true; }
	bool isConnectedTo(const HubID& id) const;

	void walk(typename LocalHub<ID, T>::HubNodeWalker& root) const;
	void walk(NodeWalker<ID, T>& root, const HubTrace& trace) const;

private:

	bool refreshNodeReferences(const ID& id, const typename std::shared_ptr<T>& node);
	bool distributeToConnectedHubs(const ID& id, const typename std::shared_ptr<T>& node, const HubTrace& trace, const HubID& first);

	STI::Utils::Distributer<ID, T> nodeDistributer;		///< The hub is built around a NodeDistributer.

	STI::Utils::SynchronizedMap <HubID, std::shared_ptr<Hub<ID, T>>> hubs;

	STI::Network::HubID hubID;

	mutable std::mutex distributerMutex;

};


} //Network
} //STI


//Implementation


template<class ID, class T>
STI::Network::LocalHub<ID, T>::LocalHub(const STI::Network::HubID& hubID)
: hubID(hubID)
{
}

template<class ID, class T>
STI::Network::LocalHub<ID, T>::~LocalHub()
{
	clear();
}

template<class ID, class T>
void STI::Network::LocalHub<ID, T>::walk(typename STI::Network::LocalHub<ID, T>::HubNodeWalker& root) const
{
	HubTrace trace;
	walk(root, trace);
}


template<class ID, class T>
void STI::Network::LocalHub<ID, T>::walk(STI::Network::NodeWalker<ID, T>& root, const HubTrace& trace) const
{
	if (trace.includesHubID(getID())) {
		//this call has already been to this hub; short circuit the call
		return;
	}

	std::set<HubID> hubIDs;
	getHubIDs(hubIDs);

	std::set<ID> nodeIDs;
	getNodeIDs(nodeIDs);

	std::shared_ptr<T> node;

	root.node.id = getID();

	//Nodes
	for (auto& id : nodeIDs) {
		if (getNode(id, node) && node != 0) {
			root.node.nodes.push_back(
				STI::Network::DirectedGraphNode<ID, T>::makeNode(id, node)
			);
		}
	}

	//Forward call to network (with appended trace)
	HubTrace newTrace = trace;
	newTrace.addHubID(getID());

	HubTrace foundHubs;
	std::shared_ptr<Hub<ID, T>> hub;
	
	//Hubs
	for (auto& hid : hubIDs) {
		
		auto hubGraph = std::make_unique<STI::Network::NodeWalker<ID, T>>();

		hubGraph->node.id = hid;
		
		//Check if HubID was already walked because of a loop.
		if (!foundHubs.includesHubID(hid)) {
			if (getHub(hid, hub) && hub != 0) {
				hub->walk(*hubGraph, newTrace);
			}
		}

		//Check for loops; if the next hub was already found, don't walk again.
		auto& lastHubConnections = hubGraph->connections;
		
		if (lastHubConnections.size() > 0) {
			foundHubs.addHubID(lastHubConnections.back()->node.id);
		}

		root.connections.push_back(std::move(hubGraph));
	}
}


template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::addNode(const ID& id, const typename std::shared_ptr<T>& node)
{
	if (node == 0) return false;

	bool success;
	{
		std::unique_lock<std::mutex> distributerLock(distributerMutex);
		success = nodeDistributer.add(id, node);
	}	//locked section

	node->activate();

	//set callback to remove this Node from the LocalHub
	node->setRemoveCB( [this, id]() { 
		// Use a thread to ensure the Node has time to finish its work before removing it
		std::thread([this, id]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give the Node time to finish call
			removeNode(id);
		}).detach();
	} );

	if (success) {
		{
			std::unique_lock<std::mutex> distributerLock(distributerMutex);
			success = distributeToConnectedHubs(id, node, HubTrace(), getID());
		}

		if (success) {
			refresh(HubTrace());
		}
		return success;
	}
	return false;
}

template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::removeNode(const ID& id)
{
	return removeNode(id, HubTrace());
}

template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::getNode(const ID& id, typename std::shared_ptr<T>& node) const
{
	return nodeDistributer.getNode(id, node);
}


template<class ID, class T>
void STI::Network::LocalHub<ID, T>::getNodeIDs(std::set<ID>& ids) const
{
	nodeDistributer.getIDs(ids);
}

template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::hasNodeID(const ID& id) const
{
	return nodeDistributer.contains(id);
}

//template<class ID, class T>
//void STI::Network::LocalHub<ID, T>::getHubIDs(std::set<HubID>& ids) const
//{
//	hubs.getKeys(ids);
//}


template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::addHub(const HubID& id, const typename std::shared_ptr<Hub<ID, T>>& hub)
{
	if (getID() == id) {
		//not allowed; hub id can't match this hub's id
		return false;
	}

	return hubs.add(id, hub);		//in hub list, but nodes have not been distributed yet.  Need to wait for mutual link
}

template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::removeHub(const HubID& id)
{
	return hubs.remove(id);
}

template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::removeNode(const ID& id, const HubTrace& trace)
{
	if (trace.includesHubID(getID())) {
		//this call has already been to this hub; short circuit the call
		return true;
	}

	if (trace.size() > 0) {
		//If Node ID being removed by the network is stored by this Hub, then it means that
		//another Hub had a dead reference and is pushing a removeNode to the network.
		//Check if the Node in question is owned by this Hub and that the Node is alive.
		//If so, distrubute a fresh reference to the network.

		std::shared_ptr<T> node;
		if (nodeDistributer.contains(id) && nodeDistributer.getNode(id, node)
			&& node != 0 && node->refresh()) {

			//Force the distribute call to originate at the source of the removeNode call to avoid
			//a race condition.  The removing Hub will then distribute the replacement Node when
			//the removeNode call completes.
			return distribute(id, node, HubTrace(), trace.first());
		}
	}

	std::unique_lock<std::mutex> distributerLock(distributerMutex);

	//kill
	std::shared_ptr<T> node;
	if (nodeDistributer.contains(id) && nodeDistributer.getNode(id, node) && node != 0) {
		node->disable();
	}

	nodeDistributer.remove(id);	//remove locally

	//Forward call to network (with appended trace)
	HubTrace newTrace = trace;
	newTrace.addHubID(getID());

	std::set<HubID> hubIDs;
	hubs.getKeys(hubIDs);
	std::shared_ptr<Hub<ID, T>> hub;

	//Remove from all connected Hubs
	for (auto& hid : hubIDs) {
		if (!newTrace.includesHubID(hid)) {
			//found a hub that has not received the call yet
			if (hubs.get(hid, hub) && hub != 0) {
				hub->removeNode(id, newTrace);
			}
		}
	}

	return true;
}

template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::refresh()
{
	//local hub refresh

	std::set<ID> nodeIDs;
	nodeDistributer.getIDs(nodeIDs);		//Nodes owned by this Hub
	std::shared_ptr<T> node;

	for (auto& id : nodeIDs) {
		if (nodeDistributer.getNode(id, node) && node != 0) {
			if (!node->refresh()) {
				removeNode(id);
			}
			else {
				//check this Node's stored references
				refreshNodeReferences(id, node);
			}
		}
	}

	return true;
}

template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::refreshNodeReferences(const ID& id, const typename std::shared_ptr<T>& node)
{
	bool success;
	std::shared_ptr<STI::Utils::Collection<ID, T>> collection;

	std::set<ID> refIDs;
	std::shared_ptr<T> nodeRef;

	if (node == 0) return false;

	node->getCollection(collection);

	if (collection != 0) {
		collection->getIDs(refIDs);

		//check all the references that this Node has
		for (auto& refID : refIDs) {

			success = collection->get(refID, nodeRef) && nodeRef != 0
				&& nodeRef->refresh();		//check that the reference is alive

			if (!success) {
				collection->remove(refID);
				removeNode(refID, HubTrace());		//remove broken reference from network
			}
		}
	}

	return true;
}

template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::refresh(const HubTrace& trace)
{
	if (trace.includesHubID(getID())) {
		//this call has already been to this hub; short circuit the call
		return true;
	}

	refresh();

	//Forward call to network (with appended trace)
	HubTrace newTrace = trace;
	newTrace.addHubID(getID());

	std::set<HubID> hubIDs;
	hubs.getKeys(hubIDs);
	std::shared_ptr<Hub<ID, T>> hub;

	for (auto& hid : hubIDs) {
		if (!newTrace.includesHubID(hid)) {
			//found a hub that has not received the call yet
			if (hubs.get(hid, hub) && hub != 0) {
				hub->refresh(newTrace);
			}
		}
	}

	return true;
}

template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::distribute(const ID& id, const typename std::shared_ptr<T>& node, const HubTrace& trace, const HubID& first)
{
	if (trace.includesHubID(getID())) {
		//this call has already been to this hub; short circuit the call
		return true;
	}
	if (!trace.includesHubID(first)) {
		//This indicates we are at the very beginning of the distribute call.
		//Only the Hub designated by the 'first' parameter should respond at this point.
		if (getID() != first) {
			return true;	//short-circuit (this is not the first hub target by the call)
		}
	}

	std::unique_lock<std::mutex> distributerLock(distributerMutex);

	//Distribute Node to the Nodes owned by this Hub.
	//Also, offer all owned Nodes to this Node.
	nodeDistributer.distributeNode(id, node);

	return distributeToConnectedHubs(id, node, trace, first);
}

template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::distributeToConnectedHubs(const ID& id, const typename std::shared_ptr<T>& node, const HubTrace& trace, const HubID& first)
{
	//Forward call to network (with appended trace)
	HubTrace newTrace = trace;
	newTrace.addHubID(getID());		//ensures this Hub will not respond again


	std::set<HubID> hubIDs;
	hubs.getKeys(hubIDs);
	std::shared_ptr<Hub<ID, T>> hub;

	for (auto& hid : hubIDs) {
		if (!newTrace.includesHubID(hid)) {
			//Found a hub that has not received the call yet.

			// Condition call to distribute(..) on addto(...) so nodes can optionally localize 
			// (optimization to avoid unneeded network calls)
			if (node->addto(hid) && hubs.get(hid, hub) && hub != 0) {

				hub->distribute(id, node, newTrace, first);
			}
		}
	}

	return true;
}

//distribute all owned Nodes to target Hub
template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::distributeNodes(const HubID& targetHub)
{
	//Forward call to network (with appended trace)
	HubTrace newTrace;
	newTrace.addHubID(getID());

	std::set<ID> nodeIDs;
	nodeDistributer.getIDs(nodeIDs);		//Nodes owned by this Hub
	std::shared_ptr<T> node;

	std::shared_ptr<Hub<ID, T>> hub;				//target Hub reference

	if (hubs.get(targetHub, hub) && hub != 0) {

		//Distribute all nodes owned by this Hub to the target Hub
		for (auto& id : nodeIDs) {
			if (nodeDistributer.getNode(id, node) && node != 0 && newTrace.size() > 0) {
				hub->distribute(id, node, newTrace, newTrace.first());
			}
		}
	}

	return true;
}


template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::redistributeNodes(const HubTrace& trace)
{
	if (trace.includesHubID(getID())) {
		//this call has already been to this hub; short circuit the call
		return true;
	}

	//Forward call to network (with appended trace)
	HubTrace newTrace = trace;
	newTrace.addHubID(getID());

	std::unique_lock<std::mutex> distributerLock(distributerMutex);

	nodeDistributer.distribute(); //Force redistribution of all owned Nodes to all owned Collectors

	std::set<HubID> hubIDs;
	hubs.getKeys(hubIDs);
	std::shared_ptr<Hub<ID, T>> hub;

	//Force redistribution of all Nodes owned by this Hub to all connected Hubs
	for (auto& hid : hubIDs) {
		distributeNodes(hid);		//Force redistribution of all Nodes owned by this Hub to hid

									//Pass along redistributeNodes call to connected Hubs, with tracer
		if (!newTrace.includesHubID(hid)) {
			//found a hub that has not received the call yet
			if (hubs.get(hid, hub) && hub != 0) {
				hub->redistributeNodes(newTrace);	//Request redistibution of all nodes owned by connected Hub
			}
		}
	}

	return true;
}

template<class ID, class T>
void STI::Network::LocalHub<ID, T>::clear()
{
	std::set<ID> ids;
	getNodeIDs(ids);

	for(auto id : ids) {
		removeNode(id);
	}

	disconnect();
	
	//These should be empty; clear just in case
	nodeDistributer.clearAll();
	hubs.clear();
}

template<class ID, class T>
void STI::Network::LocalHub<ID, T>::disconnect(const HubID& hid)
{
	std::shared_ptr<Hub<ID, T>> hub;
	
	if (getHub(hid, hub) && hub != 0 && hub->removeHub( getID() )) {
		removeHub(hid);
	}
}

template<class ID, class T>
void STI::Network::LocalHub<ID, T>::disconnect()
{
	std::set<HubID> hids;
	getHubIDs(hids);

	for(const auto& hid : hids) {
		disconnect(hid);
	}
}

template<class ID, class T>
bool STI::Network::LocalHub<ID, T>::isConnectedTo(const HubID& id) const
{
	return hubs.contains(id);
}

#endif
