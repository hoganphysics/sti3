#ifndef STI_UTILS_DISTRIBUTER_H
#define STI_UTILS_DISTRIBUTER_H

#include "SynchronizedMap.h"
#include "Collection.h"
#include "LocalCollection.h"
#include "Collector.h"
#include "Node.h"

#include <memory>

namespace STI
{
namespace Utils
{

//template<class ID>
//class DistributerListener
//{
//public:
//	virtual void addNode(const ID& id) = 0;
//	virtual void removeNode(const ID& id) = 0;
//	virtual void refresh() = 0;
//};

template<class ID, class T>
class Distributer
{
protected:

	//typedef std::shared_ptr<Collection<ID, Node<ID, T>>> NodeTCollection_ptr;

	typedef Collection<ID, T> CollectionT;
	typedef std::shared_ptr<CollectionT> CollectionT_ptr;
	typedef Collector<ID, T> CollectorT;
	typedef std::shared_ptr<CollectorT> CollectorT_ptr;
	typedef std::shared_ptr<T> T_ptr;

public:

	Distributer()
	{
		//nodes = CollectionT_ptr(new LocalCollection<ID, T>());
		nodes = std::make_shared<LocalCollection<ID, T> >();
	}
	Distributer(const CollectionT_ptr& nodeCollector)
	{
		nodes = nodeCollector;
	}

	~Distributer()
	{
		removeAll();		//Remove all references from collectors to break any circular dependencies between Nodes.
							//Otherwise Node destructors will never be called because of shared_ptr loops.
	}


	//Add a Node to this Hub.  The new Node will be added to the list of available Nodes
	//owned by this Hub and to the list of Node Collectors managed by this Hub.  The Node will
	//also be distributed to all the existing Collectors managed by this Hub.  Finally,
	//the Node will be distributed to all the Hubs connected to this Hub.
	bool add(const ID& id, const T_ptr& node)
	{
		//T inherits from Node<ID, T>, so it is both a T and a Collector<ID, T>
		bool success = addNode(id, node);		//add as T
		success = addCollector(id, node);		//add as Collector<ID, T>

		auto collNode = std::static_pointer_cast<Node<ID, T>>(node);
		//success = addCollector(id, collNode);
		return success;
	}

	bool addNode(const ID& id, const T_ptr& node)
	{
		if (node != 0 && nodes->add(id, node)) {
			distributeAdd(id, node);
			return true;
		}
		return false;
	}

	bool addCollector(const ID& id, const CollectorT_ptr& collector) {
//	bool addCollector(const ID& id, const NodeTCollection_ptr& collector) {
		if (collector != 0 && collectors.add(id, collector)) {
			distribute(id, collector);		//was distribute(collector)
			return true;
		}
		return false;
	}

	bool remove(const ID& id)
	{
		bool success = true;

		success &= removeCollector(id);
		success &= removeNode(id);

		return success;
	}

	bool removeNode(const ID& id)
	{
		if (nodes->remove(id)) {
			return distributeRemove(id);
		}
		return false;
	}
	bool removeCollector(const ID& id)
	{
		return collectors.remove(id);
	}

	void cleanup()
	{
		nodes->cleanup();

		std::set<ID> nodeIDs;
		nodes->getIDs(nodeIDs);

		std::set<ID> collectorIDs;
		collectors.getKeys(collectorIDs);

		cleanup(nodeIDs, collectorIDs);
	}

	void distribute()
	{
		std::set<ID> nodeIDs;
		nodes->getIDs(nodeIDs);

		std::set<ID> collectorIDs;
		collectors.getKeys(collectorIDs);

		distribute(nodeIDs, collectorIDs);
	}

	void refresh()
	{
		nodes->cleanup();

		std::set<ID> nodeIDs;
		nodes->getIDs(nodeIDs);

		std::set<ID> collectorIDs;
		collectors.getKeys(collectorIDs);

		cleanup(nodeIDs, collectorIDs);
		distribute(nodeIDs, collectorIDs);
	}

private:

	void removeAll()
	{
		std::set<ID> nodeIDs;
		nodes->getIDs(nodeIDs);

		for (typename std::set<ID>::const_iterator id = nodeIDs.begin(); id != nodeIDs.end(); ++id) {
			removeNode(*id);
		}
	}

	void distributeAdd(const ID& id, const T_ptr& node)
	{
		typename std::set<ID> ids;
		collectors.getKeys(ids);
		CollectorT_ptr collector;
		CollectionT_ptr collection;

		for (typename std::set<ID>::iterator collectorID = ids.begin(); collectorID != ids.end(); ++collectorID) {
			if (*collectorID != id			//Collectors cannot hold references to themselves
				&& collectors.get(*collectorID, collector) 
				&& collector != 0) {
				collector->getCollection(collection);
				if (collection != 0) {
					collection->add(id, node);	//conditionally adds the node, based on collector policy
				}
			}
		}
	}

	bool distributeRemove(const ID& id)
	{
		//attempts to remove id from all collectors
		bool success = true;
		typename std::set<ID> ids;
		collectors.getKeys(ids);

		CollectorT_ptr collector;
		CollectionT_ptr collection;

		for (typename std::set<ID>::iterator collectorID = ids.begin(); collectorID != ids.end(); ++collectorID) {
			if (collectors.get(*collectorID, collector) && collector != 0) {
				collector->getCollection(collection);
				if (collection != 0) {
					success &= collection->remove(id);	//returns true if id is no longer in collection
				}
			}
		}
		return success;
	}

	void distribute(const ID& collectorID, const CollectorT_ptr& targetCollector)
	{
		typename std::set<ID> nodeIDs;
		nodes->getIDs(nodeIDs);

		distribute(nodeIDs, targetCollector, collectorID);
	}

	void distribute(const std::set<ID>& nodeIDs, const CollectorT_ptr& targetCollector, const ID& collectorID)
	{
		T_ptr node;
		CollectionT_ptr targetCollection;

		for (typename std::set<ID>::const_iterator id = nodeIDs.begin(); id != nodeIDs.end(); ++id) {
			if (collectorID != *id && nodes->get(*id, node)) {		//Collector not allowed to have a reference to itself
				targetCollector->getCollection(targetCollection);
				if (targetCollection != 0) {
					targetCollection->add(*id, node);	//conditionally add based on Collection policy
				}
			}
		}
	}

	void distribute(const std::set<ID>& nodeIDs, const std::set<ID>& collectorIDs)
	{
		CollectorT_ptr collector;
		//		CollectionT_ptr collector;

		for (typename std::set<ID>::const_iterator id = collectorIDs.begin(); id != collectorIDs.end(); ++id) {
			if (collectors.get(*id, collector) && collector != 0) {
				distribute(nodeIDs, collector, *id);
			}
		}
	}

	void cleanup(const std::set<ID>& nodeIDs, const std::set<ID>& collectorIDs)
	{
		CollectorT_ptr collector;
		CollectionT_ptr collection;
		//		CollectorT_ptr collection;

		for (typename std::set<ID>::const_iterator id = collectorIDs.begin(); id != collectorIDs.end(); ++id) {
			if (collectors.get(*id, collector) && collector != 0) {
				collector->getCollection(collection);
				if (collection != 0) {
					collection->cleanup();			//Make Collections enforce their collection policies.
					collection->cleanup(nodeIDs);	//Force all Collections to remove nodes not found in the distributer's collection
				}
			}
		}
	}

	CollectionT_ptr nodes;
//	STI::Utils::SynchronizedMap<ID, T_ptr> nodes;
	
	typename STI::Utils::SynchronizedMap<ID, CollectorT_ptr> collectors;
	//typename STI::Utils::SynchronizedMap<ID, std::shared_ptr<Collection<ID, Node<ID, T>>>> collectors;

};




} //Utils
} //STI


#endif
