#ifndef STI_UTILS_HUB_H
#define STI_UTILS_HUB_H

#include "Collector.h"
#include "Distributer.h"
#include "Node.h"

#include <memory>
#include <mutex>

namespace STI
{
namespace Utils
{


//template<class ID, class T>
//class Hub;

//Hub is a collector of Devices.  So when a Hub is added to this hub, we use addCollector.

class HubID
{
public:

	bool operator<(const HubID& rhs) const { return id().compare(rhs.id()) < 0; }
	bool operator==(const HubID& rhs) const { return id().compare(rhs.id()) == 0; }
	bool operator!=(const HubID& rhs) const { return !((*this) == rhs); }


	std::string name;
	std::string address;

private:
	std::string id() const
	{
		return name + address;
	}
};

class HubTrace
{
public:

	HubTrace() {}

	HubTrace(const HubID& first) { addHubID(first); }
	HubTrace(const HubTrace& src) { ids = src.ids; }

	void addHubID(const HubID& id) { ids.push_back(id); }
	bool includesHubID(const HubID& id) const { return std::find(ids.begin(), ids.end(), id) != ids.end(); }
	const HubID& first() const { return ids.at(0); }
	unsigned size() const { return static_cast<unsigned>(ids.size()); }

private:
	std::vector<HubID> ids;
};

template<class ID, class T>
class Hub
{
public:
	Hub();
	virtual ~Hub() {}

	//local only

	//Add a Node to this Hub.  The new Node will be added to the list of available Nodes
	//owned by this Hub and to the list of Node Collectors managed by this Hub.  The Node will
	//also be distributed to all the existing Collectors managed by this Hub.  Finally,
	//the Node will be distributed to all the Hubs connected to this Hub.
	bool addNode(const ID& id, const typename std::shared_ptr<T>& node);
	bool removeNode(const ID& id);

	//local and remote, but non propagating (not trail tracked)
	bool addHub(const HubID& id, const typename std::shared_ptr<Hub<ID, T>>& hub);
	bool removeHub(const HubID& id);
	
	//local and remote – trail tracked
	bool removeNode(const ID& id, const HubTrace& trace);
	bool refresh();		//for initiating a refresh
	bool refresh(const HubTrace& trace);		//local and remote	

	//remote – trail tracked
	bool distribute(const ID& id, const typename std::shared_ptr<T>& node, const HubTrace& trace, const HubID& first);	//remote add (called by other hubs offering a reference) – trail tracked
	bool distributeNodes(const HubID& targetHub); //, const HubTrace& trace);		//distribute all local nodes to target hub

	//Force redistribution of all Nodes owned by this Hub to all connected Hubs.
	bool redistributeNodes(const HubTrace& trace);		//distribute all owned Nodes to all connected Hubs
	
	void clear();

	virtual const HubID& getID() = 0;

	static bool connect(const std::shared_ptr<Hub<ID, T>>& hub1, const std::shared_ptr<Hub<ID, T>>& hub2);

private:

	bool refreshNodeReferences(const ID& id, const typename std::shared_ptr<T>& node);

	Distributer<ID, T> nodeDistributer;

	SynchronizedMap <HubID, std::shared_ptr<Hub>> hubs;
	//add listerned to hubs SynchMap; remove or add should trigger a refresh() on the network

	mutable std::mutex distributerMutex;

};


} //Utils
} //STI


//Implementation

template<class ID, class T>
STI::Utils::Hub<ID, T>::Hub()
{
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::addNode(const ID& id, const typename std::shared_ptr<T>& node)
{
//	return distribute(id, node, HubTrace(), getID());
	bool success;
	{
		std::unique_lock<std::mutex> distributerLock(distributerMutex);
		success = nodeDistributer.add(id, node);
	}	//locked section

	if (success) {			
		return distribute(id, node, HubTrace(), getID()); //includes redundant local distribute...
	}
	return false;
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::removeNode(const ID& id)
{
	return removeNode(id, HubTrace());
}


template<class ID, class T>
bool STI::Utils::Hub<ID, T>::connect(const std::shared_ptr<Hub<ID, T>>& hub1, const std::shared_ptr<Hub<ID, T>>& hub2)
{
	if (hub1->addHub(hub2->getID(), hub2) && hub2->addHub(hub1->getID(), hub1)) {
		//mutual link established.  Distribute nodes.
		hub1->distributeNodes(hub2->getID());// , HubTrace());
		hub2->distributeNodes(hub1->getID());// , HubTrace());
		return true;
	}
	return false;
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::addHub(const HubID& id, const typename std::shared_ptr<Hub<ID, T>>& hub)
{
	if (hubs.contains(id)) {
		//not allowed; can't add the same hub twice
		return false;
	}
	else if (getID() == id) {
		//not allowed; hub id can't match this hub's id
		return false;
	}
	
	return hubs.add(id, hub);		//in hub list, but nodes have not been distributed yet.  Need to wait for mutual link
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::removeHub(const HubID& id)
{
	return hubs.remove(id);
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::removeNode(const ID& id, const HubTrace& trace)
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
		//distribute(id, node, const HubTrace& trace, const HubID& first)
		std::shared_ptr<T> node;
		if (nodeDistributer.contains(id) && nodeDistributer.get(id, node)
			&& node != 0 && node->refresh()) {
		
			//Force the distribute call to originate at the source of the removeNode call to avoid
			//a race condition.  The removing Hub will then distribute the replacement Node when
			//the removeNode call completes.
			return distribute(id, node, HubTrace(), trace.first());
		}
	}

	std::unique_lock<std::mutex> distributerLock(distributerMutex);

	nodeDistributer.remove(id);	//remove locally

	//push call to connected hubs, with appended trace
	HubTrace newTrace = trace;
	newTrace.addHubID(getID());

	std::set<HubID> hubIDs;
	hubs.getKeys(hubIDs);
	std::shared_ptr<Hub> hub;

	//Remove from all connected Hubs
	for (auto& hubID : hubIDs) {
		if (!newTrace.includesHubID(hubID)) {
			//found a hub that has not received the call yet
			if (hubs.get(hubID, hub) && hub != 0) {
				hub->removeNode(id, newTrace);
			}
		}
	}

	return true;
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::refresh()
{
	//local hub refresh
	
	std::set<ID> nodeIDs;
	nodeDistributer.getIDs(nodeIDs);		//Nodes owned by this Hub
	std::shared_ptr<T> node;

	for (auto& id : nodeIDs) {
		if (nodeDistributer.get(id, node) && node != 0) {
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
bool STI::Utils::Hub<ID, T>::refreshNodeReferences(const ID& id, const typename std::shared_ptr<T>& node)
{
	bool success;
	std::shared_ptr<STI::Utils::Collection<ID, T>> collection;
	
	std::set<ID> refIDs;
	std::shared_ptr<T> nodeRef;

	node->getCollection(collection);
	if (collection != 0) {
		collection->getIDs(refIDs);
		
		//check all the references that this Node has
		for (auto& refID : refIDs) {

			success = collection->get(refID, nodeRef) && nodeRef != 0 
				&& nodeRef->refresh();		//check that the reference is alive
			
			if (!success) {
				removeNode(refID, HubTrace());		//remove broken reference from network
			}
		}
	}

	return true;
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::refresh(const HubTrace& trace)
{
	if (trace.includesHubID(getID())) {
		//this call has already been to this hub; short circuit the call
		return true;
	}

	refresh();

	//pass it on

	HubTrace newTrace = trace;
	newTrace.addHubID(getID());

	std::set<HubID> hubIDs;
	hubs.getKeys(hubIDs);
	std::shared_ptr<Hub> hub;

	for (auto& hubID : hubIDs) {
		if (!newTrace.includesHubID(hubID)) {
			//found a hub that has not received the call yet
			if (hubs.get(hubID, hub) && hub != 0) {
				hub->refresh(newTrace);
			}
		}
	}

	return true;
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::distribute(const ID& id, const typename std::shared_ptr<T>& node, const HubTrace& trace, const HubID& first)
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

	nodeDistributer.distributeNode(id, node);

	//distribute Node locally
//	nodeDistributer.distributeAdd(id, node);

	//add all the local Nodes to the remote Node's collection
//	remoteAdd(id, node);

	//push call with appended trace
	HubTrace newTrace = trace;
	newTrace.addHubID(getID());

	std::set<HubID> hubIDs;
	hubs.getKeys(hubIDs);
	std::shared_ptr<Hub> hub;

	for (auto& hubID : hubIDs) {
		if (!newTrace.includesHubID(hubID)) {
			//found a hub that has not received the call yet
			if (hubs.get(hubID, hub) && hub != 0) {
				hub->distribute(id, node, newTrace, first);
			}
		}
	}

	return true;
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::distributeNodes(const HubID& targetHub)//, const HubTrace& trace)		//distribute all owned Nodes to target Hub
{
	//if (trace.includesHubID(getID())) {
	//	//this call has already been to this hub; short circuit the call
	//	return true;
	//}

	//push call with appended trace
	HubTrace newTrace;// = trace;
	newTrace.addHubID(getID());

	std::set<ID> nodeIDs;
	nodeDistributer.getIDs(nodeIDs);		//Nodes owned by this Hub
	std::shared_ptr<T> node;

	std::shared_ptr<Hub> hub;				//target Hub reference

	if (hubs.get(targetHub, hub) && hub != 0) {
		
		//Distribute all nodes owned by this Hub to the target Hub
		for (auto& id : nodeIDs) {
			if (nodeDistributer.get(id, node) && node != 0 && newTrace.size() > 0) {
				hub->distribute(id, node, newTrace, newTrace.first());
			}
		}
	}

	return true;
}


template<class ID, class T>
bool STI::Utils::Hub<ID, T>::redistributeNodes(const HubTrace& trace)
{
	if (trace.includesHubID(getID())) {
		//this call has already been to this hub; short circuit the call
		return true;
	}

	HubTrace newTrace = trace;
	newTrace.addHubID(getID());

	std::unique_lock<std::mutex> distributerLock(distributerMutex);

	nodeDistributer.distribute(); //Force redistribution of all owned Nodes to all owned Collectors

	std::set<HubID> hubIDs;
	hubs.getKeys(hubIDs);
	std::shared_ptr<Hub> hub;

	//Force redistribution of all Nodes owned by this Hub to all connected Hubs
	for (auto& hubID : hubIDs) {
		distributeNodes(hubID);		//Force redistribution of all Nodes owned by this Hub to hubID

		//Pass along redistributeNodes call to connected Hubs, with tracer
		if (!newTrace.includesHubID(hubID)) {
			//found a hub that has not received the call yet
			if (hubs.get(hubID, hub) && hub != 0) {
				hub->redistributeNodes(newTrace);	//Request redistibution of all nodes owned by connected Hub
			}
		}
	}

	return true;
}

template<class ID, class T>
void STI::Utils::Hub<ID, T>::clear()
{
	////removeNode all owned references for connected Hubs?
	//std::set<HubID> hubIDs;
	//hubs.getKeys(hubIDs);
	//std::shared_ptr<Hub> hub;

	//for (auto& hubID : hubIDs) {
	//	hubs
	//}

	hubs.clear();
}


#endif

