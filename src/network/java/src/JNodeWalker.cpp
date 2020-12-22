
#include "JNodeWalker.h"

#include <vector>
#include <set>
#include <memory>

using STI::Network::JNodeWalker;
using STI::Network::JHubGraphNode;
using STI::Network::JDeviceGraphNode;
using STI::Network::HubID;

////////// JNodeWalker //////////////

JNodeWalker::JNodeWalker(LocalDeviceHub::HubNodeWalker& root)
: node(root.node)
{
    for(auto& walker : root.connections) {
        connections.push_back(*walker);
    }
}

JNodeWalker::~JNodeWalker()
{
}

const JHubGraphNode& JNodeWalker::getNode() const
{
    return node;
}

const std::vector<JNodeWalker>& JNodeWalker::getConnections() const
{
    return connections;
}


////////// JHubGraphNode //////////////

JHubGraphNode::JHubGraphNode(const DirectedGraphHub<STI::Device::DeviceID, STI::Device::Device>& hub)
: id(hub.id)
{
    for(auto& node : hub.nodes) {
        nodes.push_back(*node);
    }
}

JHubGraphNode::~JHubGraphNode()
{
}

const HubID& JHubGraphNode::getHubID()
{
    return id;
}

const std::vector<JDeviceGraphNode>& JHubGraphNode::getNodes()
{
    return nodes;
}


////////// JDeviceGraphNode //////////////

JDeviceGraphNode::JDeviceGraphNode(const DirectedGraphNode<STI::Device::DeviceID, STI::Device::Device>& deviceNode)
: id(deviceNode.id), node(deviceNode.node)
{
    //Copy list of DeviceIDs
    for(auto& outID : deviceNode.outConnections) {
        outConnections.insert(outID);
    }
}

JDeviceGraphNode::~JDeviceGraphNode()
{
}

const STI::Device::DeviceID& JDeviceGraphNode::getID()
{
    return id;
}

STI::Device::JDevice JDeviceGraphNode::getNode()
{
    return node;
}

const std::set<STI::Device::DeviceID>& JDeviceGraphNode::getOutConnections()
{
    return outConnections;
}
