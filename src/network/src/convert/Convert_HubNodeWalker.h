#ifndef STI_NETWORK_CONVERT_HUBNODEWALKER_H
#define STI_NETWORK_CONVERT_HUBNODEWALKER_H

#include "NetworkConvert.h"
#include "generated/deviceNet.h"
#include <sti/network/DeviceHub.h>

namespace STI
{
//namespace Network
//{


//NodeWalker
template<>
bool Network::convert<Network::DeviceHub::HubNodeWalker, TNetwork::TNodeWalker>(const Network::DeviceHub::HubNodeWalker& nodeWalker, TNetwork::TNodeWalker& tNodeWalker);
template<>
bool Network::convert<TNetwork::TNodeWalker, Network::DeviceHub::HubNodeWalker>(const TNetwork::TNodeWalker& tNodeWalker, Network::DeviceHub::HubNodeWalker& nodeWalker);

template<>
bool Network::convert<std::unique_ptr<Network::DeviceHub::HubNodeWalker>, TNetwork::TNodeWalker>(const std::unique_ptr<Network::DeviceHub::HubNodeWalker>& nodeWalker, TNetwork::TNodeWalker& tNodeWalker);
template<>
bool Network::convert<TNetwork::TNodeWalker, std::unique_ptr<Network::DeviceHub::HubNodeWalker>>(const TNetwork::TNodeWalker& tNodeWalker, std::unique_ptr<Network::DeviceHub::HubNodeWalker>& nodeWalker);

//Needed for vector conversion
template<>
std::unique_ptr<Network::DeviceHub::HubNodeWalker> Network::convert<TNetwork::TNodeWalker, std::unique_ptr<Network::DeviceHub::HubNodeWalker>>(const TNetwork::TNodeWalker& tNodeWalker);


//THubNode
template<>
bool Network::convert<Network::DeviceDirectedGraphHub, TNetwork::THubNode>(const Network::DeviceDirectedGraphHub& graphDeviceHub, TNetwork::THubNode& tHubNode);
template<>
bool Network::convert<TNetwork::THubNode, Network::DeviceDirectedGraphHub>(const TNetwork::THubNode& tHubNode, Network::DeviceDirectedGraphHub& graphDeviceHub);


//TDeviceNode
template<>
bool Network::convert<Network::DeviceDirectedGraphNode, TNetwork::TDeviceNode>(const Network::DeviceDirectedGraphNode& graphDeviceNode, TNetwork::TDeviceNode& tDeviceNode);
template<>
bool Network::convert<TNetwork::TDeviceNode, Network::DeviceDirectedGraphNode>(const TNetwork::TDeviceNode& tDeviceNode, Network::DeviceDirectedGraphNode& graphDeviceNode);

template<>
bool Network::convert<std::unique_ptr<Network::DeviceDirectedGraphNode>, TNetwork::TDeviceNode>(const std::unique_ptr<Network::DeviceDirectedGraphNode>& graphDeviceNode, TNetwork::TDeviceNode& tDeviceNode);
template<>
bool Network::convert<TNetwork::TDeviceNode, std::unique_ptr<Network::DeviceDirectedGraphNode>>(const TNetwork::TDeviceNode& tDeviceNode, std::unique_ptr<Network::DeviceDirectedGraphNode>& graphDeviceNode);


template<>
std::unique_ptr<Network::DeviceDirectedGraphNode> Network::convert<TNetwork::TDeviceNode, std::unique_ptr<Network::DeviceDirectedGraphNode>>(const TNetwork::TDeviceNode& tDeviceNode);


//} //Network
} //STI

#endif

