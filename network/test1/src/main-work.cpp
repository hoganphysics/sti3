
#include <iostream>
using std::cout;
using std::endl;


#include "Hub.h"
using namespace STI::Utils;

int main(int argc, char **argv)
{
	LocalDevice local;

	TRemoteDevice_ptr rd;
	RemoteDevice remote(rd);

	auto l = local.clone();

	return 0;
}


template<class ID, class T>
class Node : public STI::Utils::Collector<ID, T>
{
public:
};


template<class ID, class T>
class Hub;

template<class ID, class T>
class Hub : public Node<ID, T>
{
public:
	bool addNode(const ID& id, const typename shared_ptr<T>& node) { return dist.add(id, node);  }
	//dist.add needs to add a T and a Collector<T>, which Device : Node<ID, Device> satisfies.
	//So Distributer should be a NodeDistributer, and should combine addNode and addCollector inot addNode;
	
	bool removeNode(const ID& id) { return dist.remove(id); }

	bool addHub(const ID& id, const typename shared_ptr<Hub<ID, T>>& hub) 
	{
		hubs.add(id, hub);

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



class NetworkDeviceHub : public Hub<DeviceID, Device>
{
	void adder(LocalDevice)
	{
		//make servant, and store(?)
		//make LocalDevice wrapper
		Hub::addNode(...);
	}
	void adder(RemoteDevice);

	//separate Collector of RemoteDevice_i	
};










/////////////
class LocalDeviceHub : public Hub<DeviceID, LocalDevice>
{

};

class LocalDeviceHub : public Hub<DeviceID, RemoteDevice>
{

};