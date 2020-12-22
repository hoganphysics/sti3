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

/*
The STI network graph can be decomposed into two subgraphs: a bidirected graph describing the Hub connections, 
and a directed graph describing the Device connections.  Since every Device node is hosted by a Hub, these
graphs are tightly related.

The NodeWalker data structure contains both subgraphs.  The top level of NodeWalker is made up of BidirectedGraphNodes.
Each BidirectedGraphNode contains node data, as well as a list of connections to other BidirectedGraphNodes.
The NodeWalker stores the Hub graph in the BidirectedGraphNode layer.  The list of other BidirectedGraphNodes represent
the connections to other hubs.  The node data payload of each BidirectedGraphNode contains a given hub's information.  
Specifically, the nodes realized as instances of DirectedGraphHub objects, which contain the hub's HubID as well
as a list of the hub's attached devices. The attached devices are represented by DirectedGraphNode objects. Each
DirectedGraphNode contains a DeviceID, a Device reference, and a list of directed edges representing the connections 
to other Devices ("outConnections"). The Device subgraph is therefore stored by the DirectedGraphHubs.

To be consistent with the generic Hub class, NodeWalker is implement here generically using templates. In practice,
NodeWalker is instantiated as BidirectedGraphNode<DirectedGraphHub<DeviceID, Device>>.

NodeWalker psuedocode:

BidirectedGraphNode		//Hub subgraph
{
	DirectedGraphHub	//T node
	{
		HubID;
		vector<DirectedGraphNode> nodes;	//This Hub's attached devices
	};
	vector<BidirectedGraphNode> connections;	//connections to other Hubs
}

DirectedGraphNode		//Device subgraph
{
	DeviceID;
	Device;
	vector<DeviceID> outConnections;
}

*/

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

