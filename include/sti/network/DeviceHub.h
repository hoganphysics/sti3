#ifndef STI_NETWORK_DEVICEHUB_H
#define STI_NETWORK_DEVICEHUB_H

#include <sti/network/Hub.h>
#include <sti/device/Device.h>
#include <sti/device/DeviceID.h>

namespace STI
{
namespace Network
{

typedef Hub<STI::Device::DeviceID, STI::Device::Device> DeviceHub;

//NodeWalker
typedef DirectedGraphHub<STI::Device::DeviceID, STI::Device::Device> DeviceDirectedGraphHub;
typedef DirectedGraphNode<STI::Device::DeviceID, STI::Device::Device> DeviceDirectedGraphNode;



} //Network
} //STI

#endif
