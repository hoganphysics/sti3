#ifndef STI_DEVICE_DEVICENODE_H
#define STI_DEVICE_DEVICENODE_H

#include "Node.h"
#include "DeviceID.h"

namespace STI
{
namespace Device
{

class Device;

class DeviceNode22 : public STI::Network::Node2<DeviceID, Device>
{
public:
	virtual ~DeviceNode() {}
};


} //Device
} //STI

#endif
