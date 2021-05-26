

#include "Convert_HubNodeWalker.h"
#include "NetworkConvert.h"

#include "Hub.h"
#include "DeviceHub.h"
#include "DeviceID.h"
#include "RemoteDevice.h"

#include "orbTypes.h"
#include "deviceNet.h"

using STI::Network::convert;
using STI::Network::DeviceHub;
using STI::TNetwork::TNodeWalker;

using STI::Network::DeviceDirectedGraphHub;
using STI::TNetwork::THubNode;
using STI::Network::HubID;
using STI::TNetwork::TDeviceHubID;

using STI::Network::DeviceDirectedGraphNode;
using STI::TNetwork::TDeviceNode;
using STI::TNetwork::TDeviceID;
using STI::Network::RemoteDevice;


template<>
bool STI::Network::convert<DeviceHub::HubNodeWalker, TNodeWalker>(
	const DeviceHub::HubNodeWalker& nodeWalker, TNodeWalker& tNodeWalker)
{
	bool success;
	success  = convert<DeviceDirectedGraphHub, THubNode>(nodeWalker.node, tNodeWalker.node);
	success &= convert(nodeWalker.connections, (_CORBA_Unbounded_Sequence_Forward<TNodeWalker>&) tNodeWalker.connections);

	return success;
}


template<>
bool STI::Network::convert<TNodeWalker, DeviceHub::HubNodeWalker>(
	const TNodeWalker& tNodeWalker, DeviceHub::HubNodeWalker& nodeWalker)
{
	bool success;
	success  = convert<THubNode, DeviceDirectedGraphHub>(tNodeWalker.node, nodeWalker.node);
	success &= convert((_CORBA_Unbounded_Sequence_Forward<TNodeWalker>&) tNodeWalker.connections, nodeWalker.connections);

	return success;
}


template<>
bool STI::Network::convert<std::unique_ptr<DeviceHub::HubNodeWalker>, TNodeWalker>(
	const std::unique_ptr<DeviceHub::HubNodeWalker>& nodeWalker, TNodeWalker& tNodeWalker)
{
	if (nodeWalker != 0) {
		return convert<DeviceHub::HubNodeWalker, TNodeWalker>(*nodeWalker, tNodeWalker);
	}
	return false;
}

template<>
bool STI::Network::convert<TNodeWalker, std::unique_ptr<DeviceHub::HubNodeWalker>>(
	const TNodeWalker& tNodeWalker, std::unique_ptr<DeviceHub::HubNodeWalker>& nodeWalker)
{
	if (nodeWalker != 0) {
		return convert<TNodeWalker, DeviceHub::HubNodeWalker>(tNodeWalker, *nodeWalker);
	}
	return false;
}

template<>
std::unique_ptr<DeviceHub::HubNodeWalker> STI::Network::convert<TNodeWalker, std::unique_ptr<DeviceHub::HubNodeWalker>>(const TNodeWalker& tNodeWalker)
{
	auto nodeWalker = std::make_unique<DeviceHub::HubNodeWalker>();
	convert<TNodeWalker, std::unique_ptr<DeviceHub::HubNodeWalker>>(tNodeWalker, nodeWalker);

	return std::move(nodeWalker);
}


// THubNode

template<>
bool STI::Network::convert<DeviceDirectedGraphHub, THubNode>(const DeviceDirectedGraphHub& graphDeviceHub, THubNode& tHubNode)
{
	bool success;
	success  = convert<HubID, TDeviceHubID>(graphDeviceHub.id, tHubNode.hubID);
	success &= convert(graphDeviceHub.nodes, (_CORBA_Unbounded_Sequence<TDeviceNode>&) tHubNode.nodes);
	
	return success;
}

template<>
bool STI::Network::convert<THubNode, DeviceDirectedGraphHub>(const THubNode& tHubNode, DeviceDirectedGraphHub& graphDeviceHub)
{
	bool success;
	success  = convert<TDeviceHubID, HubID>(tHubNode.hubID, graphDeviceHub.id);
	success &= convert((_CORBA_Unbounded_Sequence<TDeviceNode>&) tHubNode.nodes, graphDeviceHub.nodes);

	return success;
}


// TDeviceNode

template<>
bool STI::Network::convert<DeviceDirectedGraphNode, TDeviceNode>(
	const DeviceDirectedGraphNode& graphDeviceNode, TDeviceNode& tDeviceNode)
{
	bool success;
	success  = convert<STI::Device::DeviceID, TDeviceID>(graphDeviceNode.id, tDeviceNode.deviceID);
	success &= convert(graphDeviceNode.outConnections, (_CORBA_Unbounded_Sequence<TDeviceID>&) tDeviceNode.outConnections);

	if (!success) {
		return false;
	}

	success = false;	//reset

	//Conversion happens where the Devices are hosted.  All Device reference should therefore be
	//NetworkDeviceWrappers, so the TDeviceRefInterface is available.

	STI::TNetwork::TDevice_var tDevice;

	if (!TDeviceRefInterface::getTDeviceReference(graphDeviceNode.node, tDevice)) {
		tDeviceNode.node = tDevice;
		success = true;
	}

	return success;
}


template<>
bool STI::Network::convert<TDeviceNode, DeviceDirectedGraphNode>(
	const TDeviceNode& tDeviceNode, DeviceDirectedGraphNode& graphDeviceNode)
{
	bool success;
	success  = convert<TDeviceID, STI::Device::DeviceID>(tDeviceNode.deviceID, graphDeviceNode.id);
	success &= convert((_CORBA_Unbounded_Sequence<TDeviceID>&) tDeviceNode.outConnections, graphDeviceNode.outConnections);

	if (!success) {
		return false;
	}

	success = false;	//reset

	// if (!CORBA::is_nil(tDeviceNode.node)) {

		auto remoteDevice = std::make_shared<RemoteDevice>(tDeviceNode.node);	//std::shared_ptr<STI::Device::Device>
		graphDeviceNode.node = remoteDevice;

		success = (remoteDevice != 0);
	// }

	return success;
}


template<>
bool STI::Network::convert<std::unique_ptr<DeviceDirectedGraphNode>, TDeviceNode>(
	const std::unique_ptr<DeviceDirectedGraphNode>& graphDeviceNode, TDeviceNode& tDeviceNode)
{
	if (graphDeviceNode != 0) {
		return convert<DeviceDirectedGraphNode, TDeviceNode>(*graphDeviceNode, tDeviceNode);
	}
	return false;
}


template<>
bool STI::Network::convert<TDeviceNode, std::unique_ptr<DeviceDirectedGraphNode>>(
	const TDeviceNode& tDeviceNode, std::unique_ptr<DeviceDirectedGraphNode>& graphDeviceNode)
{
	if (graphDeviceNode != 0) {
		return convert<TDeviceNode, DeviceDirectedGraphNode>(tDeviceNode, *graphDeviceNode);
	}
	return false;
}


template<>
std::unique_ptr<DeviceDirectedGraphNode> STI::Network::convert<TDeviceNode, std::unique_ptr<DeviceDirectedGraphNode>>(const TDeviceNode& tDeviceNode)
{
	auto graphDeviceNode = std::make_unique<DeviceDirectedGraphNode>();
	convert<TDeviceNode, std::unique_ptr<DeviceDirectedGraphNode>>(tDeviceNode, graphDeviceNode);

	return std::move(graphDeviceNode);
}

