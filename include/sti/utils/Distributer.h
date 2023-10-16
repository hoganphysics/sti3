#ifndef STI_UTILS_DISTRIBUTER_H
#define STI_UTILS_DISTRIBUTER_H

#include <sti/utils/SynchronizedMap.h>
#include <sti/utils/Collection.h>
#include <sti/utils/LocalCollection.h>
#include <sti/utils/Collector.h>
#include <sti/network/Node.h>

#include <memory>

namespace STI
{
namespace Utils
{


template<class ID, class T>
class Distributer
{
	typedef Collection<ID, T> CollectionT;
	typedef std::shared_ptr<CollectionT> CollectionT_ptr;
	typedef Collector<ID, T> CollectorT;
	typedef std::shared_ptr<CollectorT> CollectorT_ptr;
	typedef std::shared_ptr<T> T_ptr;

public:

	Distributer()
	{
		nodes = std::make_shared<LocalCollection<ID, T>>();
	}
	Distributer(const CollectionT_ptr& nodeCollector)
	{
		nodes = nodeCollector;
	}

	~Distributer()
	{
		//removeAll();		//Remove all references from collectors to break any circular dependencies between Nodes.
							//Otherwise Node destructors will never be called because of shared_ptr loops.

		clearAll();
	}

	bool add(const ID& id, const T_ptr& node)
	{
		//T_ptr should be both a T and a CollectorT_ptr, assuming it's a Node<ID,T>
		bool success = addNode(id, node);
		success &= addCollector(id, node);

		return success;
	}

	bool remove(const ID& id)
	{
		bool success = true;

		success &= removeCollector(id);
		success &= removeNode(id);

		return success;
	}

	bool contains(const ID& id) const
	{
		return nodes->contains(id);
	}

	unsigned numberOfNodes() const { return nodes->size(); }

	bool getNode(const ID& id, T_ptr& node) const
	{
		return nodes->get(id, node);
	}	
	
	void getIDs(std::set<ID>& ids) const
	{
		return nodes->getIDs(ids);
	}

	void distribute()
	{
		cleanup();

		std::set<ID> nodeIDs;
		nodes->getIDs(nodeIDs);

		std::set<ID> collectorIDs;
		collectors.getKeys(collectorIDs);

		distribute(nodeIDs, collectorIDs);
	}

	void distributeNode(const ID& id, const T_ptr& node)
	{
		distributeAdd(id, node);	//distribute Node locally
		distribute(id, node);		//offer locally owned Nodes to this Node
	}

	//Check that all stored Nodes are alive, and that all references they have collected are also alive.
	//Remove dead references.
	//void refresh()
	//{
	//	typename std::set<ID> nodeIDs;
	//	nodes.getKeys(nodeIDs);

	//	for (auto& id : nodeIDs) {
	//		
	//	}

	//	//Collector references
	//	CollectorT_ptr collector;
	//	CollectionT_ptr collection;

	//	typename std::set<ID> collectorIDs;
	//	collectors.getKeys(collectorIDs);
	//	for (auto& collectorID : collectorIDs) {
	//		if (collectors.get(collectorID, collector) && collector != 0) {
	//			collector->getCollection(collection);
	//			if (collection != 0) {
	//				collection->refresh();
	//			}
	//		}
	//	}

	//}

	void clearAll()
	{
		typename std::set<ID> ids;
		collectors.getKeys(ids);

		CollectorT_ptr collector;
		CollectionT_ptr collection;

		for (auto& id : ids) {
			if (collectors.get(id, collector) && collector != 0) {
				collector->getCollection(collection);
				if (collection != 0) {
					collection->clear();
				}
			}
		}

	}


private:

	void cleanup()
	{
		nodes->cleanup();		//ensure the owned Nodes match the Collection policy

		std::set<ID> collectorIDs;
		collectors.getKeys(collectorIDs);

		//Make all stored Collectors enforce their collection policies.
		cleanup(collectorIDs);
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
		if (collector != 0 && collectors.add(id, collector)) {
			distribute(id, collector);		//was distribute(collector)
			return true;
		}
		return false;
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

	void distribute(const ID& collectorID, const CollectorT_ptr& targetCollector)
	{
		typename std::set<ID> nodeIDs;
		nodes->getIDs(nodeIDs);

		distribute(collectorID, targetCollector, nodeIDs);
	}

	void distribute(const ID& collectorID, const CollectorT_ptr& targetCollector, const std::set<ID>& nodeIDs)
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
				distribute(*id, collector, nodeIDs);
			}
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

	void removeAll()
	{
		std::set<ID> nodeIDs;
		nodes->getIDs(nodeIDs);

		for (typename std::set<ID>::const_iterator id = nodeIDs.begin(); id != nodeIDs.end(); ++id) {
			removeNode(*id);
		}
	}


	void cleanup(const std::set<ID>& collectorIDs)
	{
		CollectorT_ptr collector;
		CollectionT_ptr collection;

		for (typename std::set<ID>::const_iterator id = collectorIDs.begin(); id != collectorIDs.end(); ++id) {
			if (collectors.get(*id, collector) && collector != 0) {
				collector->getCollection(collection);
				if (collection != 0) {
					collection->cleanup();			//Make Collections enforce their collection policies.
					//collection->cleanup(nodeIDs);	//Force all Collections to remove nodes not found in the distributer's collection
				}
			}
		}
	}

	CollectionT_ptr nodes;
	
	typename STI::Utils::SynchronizedMap<ID, CollectorT_ptr> collectors;
};


} //Utils
} //STI

#endif
