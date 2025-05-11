
#ifndef STI_PYTHON_NODEWALKERPY_H
#define STI_PYTHON_NODEWALKERPY_H

#include <sti/LocalDeviceHub.h>
#include <sti/network/NodeWalker.h>
#include <sti/network/HubID.h>
#include <sti/device/DeviceID.h>
#include <sti/device/Device.h>

#include "DevicePy.h"

#include <set>
#include <vector>

#include <pybind11/pybind11.h>

namespace STI
{
namespace Python
{

class NodeWalkerPy;
class HubGraphNodePy;
class DeviceGraphNodePy;
    


//Python HubGraphNode wrapper
class HubGraphNodePy
{
public:
    
    HubGraphNodePy(const STI::Network::DirectedGraphHub<STI::Device::DeviceID, STI::Device::Device>& hub);
    ~HubGraphNodePy();

    const STI::Network::HubID& getHubID() const;
    const std::vector<DeviceGraphNodePy>& getNodes() const;

private:

    STI::Network::HubID id;
    std::vector<DeviceGraphNodePy> nodes;

};


//Python NodeWalker wrapper
class NodeWalkerPy
{
public:
	
    NodeWalkerPy(STI::Network::LocalDeviceHub::HubNodeWalker& root);
	~NodeWalkerPy();

    const HubGraphNodePy& getNode() const;
    const std::vector<NodeWalkerPy>& getConnections() const;

private:

    HubGraphNodePy node;
    std::vector<NodeWalkerPy> connections;

};


//Python DeviceGraphNode wrapper
class DeviceGraphNodePy
{
public:

    DeviceGraphNodePy(const STI::Network::DirectedGraphNode<STI::Device::DeviceID, STI::Device::Device>& deviceNode);
    ~DeviceGraphNodePy();

    const STI::Device::DeviceID& getID() const;
    std::shared_ptr<STI::Python::DevicePy> getNode() const;
    const std::vector<STI::Device::DeviceID>& getOutConnections() const;

private:

    STI::Device::DeviceID id;
    std::shared_ptr<STI::Python::DevicePy> node;
    std::vector<STI::Device::DeviceID> outConnections;
};


} //Python
} //STI

#endif
