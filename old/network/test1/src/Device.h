#ifndef STI_DEVICE_DEVICE_H
#define STI_DEVICE_DEVICE_H

#include "Node.h"
#include "DeviceID.h"

namespace STI
{
namespace Device
{

//CRTP
//pure interface for node elements
class Device : public STI::Utils::Node<DeviceID, Device>
{
public:
	virtual void write(unsigned input) = 0;

	virtual bool refresh() { return true; }	//temp

};

} //Device
} //STI

#endif
