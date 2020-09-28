#ifndef STI_NETWORK_NODEWALKER_H
#define STI_NETWORK_NODEWALKER_H

/*
The NodeWalker datastruture is used to walk through the network systematically and record all 
the nodes of the network graph, as well as all directed connections between them (graph edges).
The 'nodes' in the graph consist of Hubs as well as the Nodes that the Hubs own.

The entry point is any Hub.  The Hubs are traversed by getting the HubID list at each Hub, collecting
all local Nodes, then walking to the next Hub, keeping a HubTrace to avoid loops.  NodeWalker has all 
of the strengths of a depth first search while avoiding the weaknesses of a recursive search through 
a possibly cyclic graph.

At each Hub, NodeWalker gets the HubID list of connections to other Hubs, as well as the following 
for each stored Node:
* NodeID
* Node reference
* NodeID list of outgoing connections

*/

#include "Node.h"

#include <vector>
#include <set>
#include <memory>

namespace STI
{
namespace Network
{


template<typename T>
class BidirectedGraphNode
{
public:
	T node;
	std::vector<std::unique_ptr<BidirectedGraphNode<T>>> connections;
};


template<class ID, class T> class DirectedGraphNode;

template<class ID, class T>
class DirectedGraphHub
{
public:
	HubID id;

	std::vector<std::unique_ptr<DirectedGraphNode<ID, T>>> nodes;
};


template<class ID, class T>
class DirectedGraphNode
{
public:
	ID id;
	std::shared_ptr<T> node;

	std::set<ID> outConnections;

	static std::unique_ptr<DirectedGraphNode<ID, T>> 
		makeNode(const ID& id, const std::shared_ptr<T>& node);
};


template<class ID, class T>
std::unique_ptr<DirectedGraphNode<ID, T>> DirectedGraphNode<ID, T>::makeNode(const ID& id, const std::shared_ptr<T>& node)
{
	auto directedNode = std::make_unique<DirectedGraphNode<ID, T>>();

	directedNode->id = id;
	directedNode->node = node;

	std::shared_ptr<STI::Utils::Collection<ID, T>> collection;

	node->getCollection(collection);		//type T : STI::Network::Node<ID, T>
	collection->getIDs(directedNode->outConnections);

	return std::move(directedNode);
}


template<class ID, class T>
using NodeWalker = BidirectedGraphNode<DirectedGraphHub<ID, T>>;


} //Network
} //STI


#endif

