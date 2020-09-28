#ifndef STI_NETWORK_HUB_H
#define STI_NETWORK_HUB_H

//#include "Collector.h"
//#include "Distributer.h"
//#include "Node.h"
#include "HubTrace.h"
#include "HubID.h"
#include "NodeWalker.h"

#include <memory>
//#include <mutex>
//#include <algorithm>

namespace STI
{
namespace Network
{

template<class ID, class T>
class Hub
{
public:
//	Hub() {}
//	Hub(Hub& other) {}

	virtual ~Hub() {}

	//local and remote, but non propagating (not trail tracked)
	virtual bool addHub(const HubID& id, const typename std::shared_ptr<Hub<ID, T>>& hub) = 0;
	virtual bool removeHub(const HubID& id) = 0;
	
//	virtual void getNodeIDs(std::set<ID>& ids) const = 0;
//	virtual void getHubIDs(std::set<HubID>& ids) const = 0;

	//local and remote; trail tracked
//	virtual bool refresh() = 0;		//for initiating a refresh
	virtual bool refresh(const HubTrace& trace) = 0;		//local and remote	

	//remote; trail tracked
	virtual bool distribute(const ID& id, const typename std::shared_ptr<T>& node, 
		const HubTrace& trace, const HubID& first) = 0;	//remote add (called by other hubs offering a reference); trail tracked
	virtual bool distributeNodes(const HubID& targetHub) = 0; //, const HubTrace& trace);		//distribute all local nodes to target hub
	//Force redistribution of all Nodes owned by this Hub to all connected Hubs.
	virtual bool redistributeNodes(const HubTrace& trace) = 0;		//distribute all owned Nodes to all connected Hubs
	virtual bool removeNode(const ID& id, const HubTrace& trace) = 0;
	
	virtual const HubID& getID() const = 0;

	typedef NodeWalker<ID, T> HubNodeWalker;

	virtual void walk(HubNodeWalker& root, const HubTrace& trace) const = 0;

	static bool connect(const std::shared_ptr<Hub<ID, T>>& hub1, const std::shared_ptr<Hub<ID, T>>& hub2);

};


} //Network
} //STI


//Implementation

template<class ID, class T>
bool STI::Network::Hub<ID, T>::connect(const std::shared_ptr<Hub<ID, T>>& hub1, const std::shared_ptr<Hub<ID, T>>& hub2)
{
	if (hub1 != 0 && hub2 != 0 
		&& hub1->addHub(hub2->getID(), hub2)
		&& hub2->addHub(hub1->getID(), hub1)) {

		//mutual link established.  Distribute nodes.
		//Either of these options is equivallent; both are redundant.
		hub1->distributeNodes(hub2->getID());// , HubTrace());
		//hub2->distributeNodes(hub1->getID());// , HubTrace());	//including this works, but it's redundant
		return true;
	}
	return false;
}


#endif

