#ifndef STI_UTILS_LOCALCOLLECTION_H
#define STI_UTILS_LOCALCOLLECTION_H

#include <sti/utils/Collection.h>
#include <sti/utils/SynchronizedMap.h>

#include <set>
#include <memory>


namespace STI
{
namespace Utils
{

template<class ID>
class LocalCollectionListener : public SynchronizedMapListener<ID>
{
public:
	typedef std::shared_ptr<LocalCollectionListener<ID> > _ptr;
	
	virtual ~LocalCollectionListener() {}
};

template<class ID>
class LocalCollectionListenerAdapter : public LocalCollectionListener<ID>
{
public:

	virtual ~LocalCollectionListenerAdapter() {}

	virtual void add(const ID& id) {}
	virtual void remove(const ID& id) {}
	virtual void refresh() {}
};

//The Delegate class forwards events to the listener target.  This allows the listener to be a local
//delegate object instead of a base class.
template<class ID>
class LocalCollectionListenerDelegate : public LocalCollectionListener<ID>
{
public:

	LocalCollectionListenerDelegate(LocalCollectionListener<ID>* target) : target(target) {}
	virtual ~LocalCollectionListenerDelegate() {}

	void add(const ID& id) { if( target != 0) target->add(id); }
	void remove(const ID& id) { if( target != 0) target->remove(id); }
	void refresh() { if( target != 0) target->refresh(); }

private:
	LocalCollectionListener<ID>* target;
};



template<class ID, class T>
class LocalCollection : public Collection<ID, T>
{
public:
	typedef STI::Utils::SynchronizedMapPolicy<ID> LocalCollectionPolicy;
	typedef std::shared_ptr<LocalCollectionPolicy> LocalCollectionPolicy_ptr;

	LocalCollection() {}
	LocalCollection(const LocalCollectionPolicy_ptr& policy) : nodes(policy) {}

	virtual ~LocalCollection() 
	{
		clear();
		clearListeners();
	}
	
	void addListener(const typename LocalCollectionListener<ID>::_ptr& listener) { nodes.addListener(listener); }
	void removeListener(const typename LocalCollectionListener<ID>::_ptr& listener) { nodes.removeListener(listener); }
	void clearListeners() { nodes.clearListeners(); }
	void setPolicy(const LocalCollectionPolicy_ptr& policy) { nodes->setPolicy(policy); }

	template<typename D>
	bool add(const ID& id, const std::shared_ptr<D>& node)		//add derived types that satisfy class D : public T.
	{
		typename Collection<ID, T>::T_ptr tNode = node;		//convert to type T
		return (tNode != 0) && nodes.add(id, tNode);
	}

	bool add(const ID& id, const typename Collection<ID, T>::T_ptr& node) { return (node != 0 && nodes.add(id, node)); }
	bool remove(const ID& id) { return nodes.remove(id); }

	bool contains(const ID& id) const { return nodes.contains(id); }
	unsigned size() const { return nodes.size(); }

	bool get(const ID& id, typename Collection<ID, T>::T_ptr& node) const { return nodes.get(id, node); }
	void getIDs(std::set<ID>& ids) const { nodes.getKeys(ids); }

	//void cleanup(const std::set<ID>& ids)  //Remove any collected nodes that are NOT in ids
	//{
	//	typename std::set<ID> storedIDs;
	//	getIDs(storedIDs);

	//	typename std::set<ID>::const_iterator clean;

	//	for (typename std::set<ID>::iterator id = storedIDs.begin(); id != storedIDs.end(); ++id) {
	//		clean = ids.find(*id);
	//		if (clean != ids.end()) {
	//			remove(*id);
	//		}
	//	}
	//}

	void cleanup()  //Remove any collected nodes that violate policy
	{
		nodes.cleanup();
	}
	void clear() { nodes.clear(); }

private:
	STI::Utils::SynchronizedMap<ID, typename Collection<ID, T>::T_ptr> nodes;
};

} //Utils
} //STI

#endif
