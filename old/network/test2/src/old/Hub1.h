#ifndef STI_UTILS_HUB_H
#define STI_UTILS_HUB_H

#include <sti/utils/Collector.h>
#include <sti/utils/Distributer.h>
#include <sti/network/Node.h>

#include <memory>

namespace STI
{
namespace Utils
{

//template<class ID, class T>
//class Node : public STI::Utils::Collector<ID, T>
//{
//public:
//};


template<class ID, class T>
class Hub;

//Hub is a collector of Devices.  So when a Hub is added to this hub, we use addCollector.

template<class ID, class T>
class Hub : public Node<ID, T>
{
public:
	//only callable locally, so we want to add ref and its collector ref
	bool addNode(const ID& id, const typename std::shared_ptr<T>& node) 
	{ 
		//dist.add needs to add a T and a Collector<ID, T>, which Device : Node<ID, Device> satisfies.
		//So Distributer should be a NodeDistributer, and should combine addNode and addCollector inot addNode;
		return dist.add(id, node); 
	}

	bool removeNode(const ID& id) { return dist.remove(id); }

	bool addHub(const ID& id, const typename std::shared_ptr<Hub<ID, T>>& hub)
	{
		dist.addCollector(id, hub);

		hubs.add(id, hub);
		//hub interconnect.  when one hub connects, addHub should be called on both with each other's hub reference

		//distribute all locally held nodes (in dist) to all Hubs in collection
		for (h : hubs) {
			for (c : collectors in dist) {
				c->getCollection(collection);
				for (id : collection) {
					h->addNode(id, collection->get(id));
				}
			}
		}
	}

	bool removeHub(const ID& id) { hubs.remove(id); }

	Distributer<ID, T> dist;
	Collection<ID, Hub<ID, T>> hubs;
};

//they're both Collection<Device>, it's just one is LocalDevice:Device and the other is RemoteDevice:Device
//So they have to know how to add themselves to the Hub
//LocalDevice must add a TRemoteDevice_i servant and pass LocalDevice to locals and RemoteDevice to remotes
//RemoteDevice must add RemoteDevice and pass RemoteDevice to locals and RemoteDevice to remotes.
//In both cases, low level Hub should just add Node<>, after generating appropriate object.
//Basically just write custom virtual void DerivedHub::adder(SpecialedNode)=0, which delegates to Hub::addNode(()
// SpecialedNode is either LocalDevice or RemoteDevice



} //Utils
} //STI

#endif

