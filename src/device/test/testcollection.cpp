
//#include "utils/Collection.h"
#include "utils/SynchronizedMap.h"
#include <sti/device/DeviceID.h>

#include <memory>
#include <iostream>


namespace STI
{
namespace Utils
{



//Note the above seems similar to a Distributer in some ways.  Maybe the concepts are related?
//Is a Distributer just a NetworkCollection<ID, T, T> ?

//Or maybe:  Distributer<ID, T> = NetworkCollection<ID, T, Collector<ID, T> >
//But then we need 
//class T : public Collector<ID, T>
//Example
//class Device : public Collector<DeviceID, Device>  
//I think that is true (Device is a collector of Devices).
//No, I think Distributer is rather different -- it assumes D is a collector and a template version would not.

//template<class ID, class T>
//class Distributer : public Collection<ID, T>	//so we can directly share the distributer with the above via the Collector interface.
//{
//	//interface should be Collection; implementation should include distribute() calls
//	//Maybe should add refresh() to the Collection (or replace cleanup() with refresh())
//
//};




template<class ID, class T>
class Collection
{
protected:
	typedef std::shared_ptr<T> T_ptr;

public:

	virtual ~Collection() {}

	virtual bool add(const ID& id, const T_ptr& node) = 0;
	virtual bool remove(const ID& id) = 0;
};

template<class ID, class T>
class Collector
{
public:

	virtual void getCollection(std::shared_ptr<STI::Utils::Collection<ID, T>>& collection) = 0;
};


template<class ID, class D, class B>
class NetworkCollection : public Collection<ID, D>
{
	
	NetworkCollection(const typename Collector<ID, B>::T_ptr& collector)
	{
		collector->getCollection(localCollection);
	}

	bool add(const ID& id, const typename Collection<ID, D>::T_ptr& node)
	{
		typename Collection<ID, B>::T_ptr baseNode = node;		//convert node of type D to type B, where class D : public B.
		if (!( localCollection->add(baseNode) && nodes->add(node) )) {	//add to local
			localCollection->remove(baseNode);
			nodes->remove(node);
			return false;
		}
		return true;
	}

	bool get(const ID& id, typename Collection<ID, D>::T_ptr& node) const 
	{
		typename Collection<ID, B>::T_ptr baseNode;

		localCollection->get(id, baseNode);	//get from local if posible

		//if not in nodes, but it is in localCollection, wrap:
		//can't since get is const.  get can ensure the two collections are synched.
		//add and remove can from this side.  From the other side, we could use update events.

		return nodes.get(id, node); 
	}

	typename Collection<ID, B>::T_ptr localCollection;
	SynchronizedMap<ID, typename Collection<ID, D>::T_ptr> nodes;	//Derived Nodes (servants)

};

template<class ID, class T>
class LocalCollection : public Collection<ID, T>
{
public:
	LocalCollection() {}
	~LocalCollection() {}

	bool add(const ID& id, const typename Collection<ID, T>::T_ptr& node) { return nodes.add(id, node); }
	bool remove(const ID& id) { return nodes.remove(id); }

private:
	STI::Utils::SynchronizedMap<ID, typename Collection<ID, T>::T_ptr> nodes;
};



} //Utils
} //STI




namespace STI
{
namespace Device
{
class Device;
typedef std::shared_ptr<Device> Device_ptr;

//CRTP
class Device : public STI::Utils::Collector<DeviceID, Device>
{
public:
	virtual void write(unsigned channel) = 0;

};

class LocalDevice : public Device
{
public:

	LocalDevice()
	{
		deviceCollection = std::make_shared<STI::Utils::LocalCollection<DeviceID, Device>>();
	}
	void write(unsigned channel)
	{
		std::cout << "Local" << std::endl;
	}
	void getCollection(std::shared_ptr<STI::Utils::Collection<DeviceID, Device>>& collection)
	{
		collection = deviceCollection;
	}

private:
	std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>> deviceCollection;

};

} //Utils
} //STI

//wrapper around an instance of a ORB object
//can be added to the NetworkCollection (it's a device)
class RemoteDevice : public Device
{
	//also a Collector<Device>
	void getCollection(std::shared_ptr<STI::Utils::Collection<DeviceID, Device>>& collection)
	{
		collection = remoteDeviceCollection;
	}

	void write(unsigned channel)
	{
		remoteDevice->write(channel);
	}

	TRemoteDevice_var remoteDevice;
	RemoteDeviceCollection remoteDeviceCollection;
};

class RemoteDeviceCollection : public STI::Utils::Collection<DeviceID, Device>
{
	bool add(const DeviceID& id, const T_ptr& node)
	{
		//try to add a Device

		remoteDeviceCollection->add(convert(id), ???);
		//Do we need a clone function? That returns a ORB ref?
	}
	TRemoteDeviceCollection_var remoteDeviceCollection;

};


void test()
{
	std::shared_ptr<STI::Device::LocalDevice> localdev1 = std::make_shared<STI::Device::LocalDevice>();
	localdev1->write(4);

}




//
//template<class ID>
//class Node;
//
////CRTP
//template<class ID>
//class Node : public STI::Utils::Collector<ID, Node<ID>>
//{
//};
