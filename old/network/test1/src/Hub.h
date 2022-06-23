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

template<class ID, class T>
class DistributingCollection : public Collection<ID, T>
{
public:
	DistributingCollection(Distributer<ID, T>& distributer) : targetDistributer(distributer) {}
	virtual bool add(const ID& id, const T_ptr& node) { return targetDistributer.addNode(id, node); }	//just Node, no collector
	virtual bool remove(const ID& id) { return targetDistributer.removeNode(id); }

	virtual bool contains(const ID& id) const { return false; }

	virtual bool get(const ID& id, T_ptr& node) const { return false; }
	virtual void getIDs(std::set<ID>& ids) const { return; }

	virtual void cleanup(const std::set<ID>& ids) { }
	virtual void cleanup() { targetDistributer.cleanup(); }

	virtual void clear() {}

private:
	Distributer<ID, T>& targetDistributer;
};


//template<class ID, class T>
//class Hub;

//Hub is a collector of Devices.  So when a Hub is added to this hub, we use addCollector.

template<class ID, class T>
class Hub : public Node<ID, T>
{
public:
	Hub() //: collection(dist) 
	{ localCollection = std::make_shared<DistributingCollection<ID, T>>(dist); }
	//only callable locally, so we want to add ref and its collector ref
	bool addNode(const ID& id, const typename std::shared_ptr<T>& node)
	{
		//dist.add needs to add a T and a Collector<ID, T>, which Device : Node<ID, Device> satisfies.
		//So Distributer should be a NodeDistributer, and should combine addNode and addCollector inot addNode;
		return dist.add(id, node);
	}
	virtual const ID& getID() = 0;
	virtual bool refresh() { return true; }

	bool removeNode(const ID& id) { return dist.remove(id); }

	bool addHub(const ID& id, const typename std::shared_ptr<Hub<ID, T>>& hub)
	{
		return dist.addCollector(id, hub);
	}


	bool removeHub(const ID& id) { return dist.removeCollector(id); }

	void getCollection(std::shared_ptr<STI::Utils::Collection<ID, T>>& collection)
	{
		collection = localCollection;
	}

	Distributer<ID, T> dist;
	std::shared_ptr<DistributingCollection<ID, T>> localCollection;		//collection(dist);		takes dist as arguement and pushs to it.

};


} //Utils
} //STI

#endif

