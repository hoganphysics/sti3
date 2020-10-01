#ifndef STI_DEVICE_JNODEWALKER_H
#define STI_DEVICE_JNODEWALKER_H

#include "LocalDeviceHub.h"
#include "NodeWalker.h"
#include "JDevice.h"
#include <memory>
#include <vector>
#include <set>

namespace STI
{
namespace Network
{

class JNodeWalker;
class JHubGraphNode;
class JDeviceGraphNode;



class JHubGraphNode
{
public:
    
    JHubGraphNode(const STI::Network::DirectedGraphHub<STI::Device::DeviceID, STI::Device::Device>& hub);
    ~JHubGraphNode();

    const STI::Network::HubID& getHubID();
    const std::vector<JDeviceGraphNode>& getNodes();

private:

    HubID id;
    std::vector<JDeviceGraphNode> nodes;

};


//Java NodeWalker wrapper
class JNodeWalker
{
public:
	
	JNodeWalker(STI::Network::LocalDeviceHub::HubNodeWalker& root);
	~JNodeWalker();

    const JHubGraphNode& getNode() const;
    const std::vector<JNodeWalker>& getConnections() const;

private:

    JHubGraphNode node;
    std::vector<JNodeWalker> connections;

};



class JDeviceGraphNode
{
public:

    JDeviceGraphNode(const STI::Network::DirectedGraphNode<STI::Device::DeviceID, STI::Device::Device>& deviceNode);
    ~JDeviceGraphNode();

    const STI::Device::DeviceID& getID();
    STI::Device::JDevice getNode();
    const std::set<STI::Device::DeviceID>& getOutConnections();

private:

    STI::Device::DeviceID id;
    STI::Device::JDevice node;
    std::set<STI::Device::DeviceID> outConnections;
};

} //Device
} //STI

#endif
