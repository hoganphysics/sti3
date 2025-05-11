#include "NodeWalkerPy.h"

using namespace STI::Python;

// HubGraphNodePy implementation
HubGraphNodePy::HubGraphNodePy(const STI::Network::DirectedGraphHub<STI::Device::DeviceID, STI::Device::Device>& hub)
{
    id = hub.id;

    for(auto& deviceNode : hub.nodes) {
        nodes.push_back(*deviceNode);
    }
}

HubGraphNodePy::~HubGraphNodePy()
{
}

const STI::Network::HubID& HubGraphNodePy::getHubID() const
{
    return id;
}

const std::vector<DeviceGraphNodePy>& HubGraphNodePy::getNodes() const
{
    return nodes;
}

// NodeWalkerPy implementation
NodeWalkerPy::NodeWalkerPy(STI::Network::LocalDeviceHub::HubNodeWalker& root)
: node(root.node)
{
    for(auto& walker : root.connections) {
        connections.push_back(*walker);
    }
}

NodeWalkerPy::~NodeWalkerPy() 
{
}

const HubGraphNodePy& NodeWalkerPy::getNode() const
{
    return node;
}

const std::vector<NodeWalkerPy>& NodeWalkerPy::getConnections() const
{
    return connections;
}

// DeviceGraphNodePy implementation
DeviceGraphNodePy::DeviceGraphNodePy(const STI::Network::DirectedGraphNode<STI::Device::DeviceID, STI::Device::Device>& deviceNode) 
: id(deviceNode.id)
{
    node = std::make_shared<STI::Python::DevicePy>(deviceNode.node);

    for(auto& deviceID : deviceNode.outConnections) {
        outConnections.push_back(deviceID);
    }
}

DeviceGraphNodePy::~DeviceGraphNodePy() 
{
}

const STI::Device::DeviceID& DeviceGraphNodePy::getID() const
{
    return id;
}

std::shared_ptr<STI::Python::DevicePy> DeviceGraphNodePy::getNode() const
{
    return node;
}

const std::vector<STI::Device::DeviceID>& DeviceGraphNodePy::getOutConnections() const
{
    return outConnections;
}
