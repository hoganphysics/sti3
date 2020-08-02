#ifndef STI_UTILS_HUB_H
#define STI_UTILS_HUB_H

#include "Collector.h"
#include "Distributer.h"
#include "Node.h"

#include <memory>


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

	
	bool redistribute(const HubID& target, const HubTrace& trace);		//distribute all local nodes to target hub
	bool redistribute(const HubTrace& trace);		//distribute all local nodes to all connected hubs
	bool redistributeAll(const HubTrace& trace);	//send a redistribute command to the whole network

	void clear() { hubs.clear(); }

	virtual const HubID& getID() = 0;

	static bool connect(const std::shared_ptr<Hub<ID, T>>& hub1, const std::shared_ptr<Hub<ID, T>>& hub2);

private:

	Distributer<ID, T> nodeDistributer;

	SynchronizedMap <HubID, std::shared_ptr<Hub>> hubs;
	//add listerned to hubs SynchMap; remove or add should trigger a refresh() on the network

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

	if (nodeDistributer.add(id, node)) {			//includes redundant local distribute...
		return distribute(id, node, HubTrace(), getID());
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
		hub1->redistribute(hub2->getID(), HubTrace());
		hub2->redistribute(hub1->getID(), HubTrace());
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

	nodeDistributer.remove(id);	//remove locally

	//push call to connected hubs with appended trace
	HubTrace newTrace = trace;
	newTrace.addHubID(getID());

	std::set<HubID> hubIDs;
	hubs.getKeys(hubIDs);
	std::shared_ptr<Hub> hub;

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
	return false;
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::refresh(const HubTrace& trace)
{
	return false;
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
bool STI::Utils::Hub<ID, T>::redistribute(const HubID& target, const HubTrace& trace)		//distribute all local nodes to target hub
{
	if (trace.includesHubID(getID())) {
		//this call has already been to this hub; short circuit the call
		return true;
	}

	//push call with appended trace
	HubTrace newTrace = trace;
	newTrace.addHubID(getID());

	std::set<ID> nodeIDs;
	nodeDistributer.getIDs(nodeIDs);		//Nodes owned by this hub
	std::shared_ptr<T> node;

	std::shared_ptr<Hub> hub;
	if (hubs.get(target, hub) && hub != 0) {
		
		//share all nodes owned by this hub with the other hub
		for (auto& id : nodeIDs) {
			if (nodeDistributer.get(id, node) && node != 0) {
				hub->distribute(id, node, newTrace, newTrace.first());
			}
		}
	}

	return true;
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::redistribute(const HubTrace& trace)		//distribute all local nodes to all connected hubs
{
	return false;
}

template<class ID, class T>
bool STI::Utils::Hub<ID, T>::redistributeAll(const HubTrace& trace)	//send a redistribute command to the whole network
{
	return false;
}


#endif

