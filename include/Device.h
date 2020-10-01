#ifndef STI_DEVICE_DEVICE_H
#define STI_DEVICE_DEVICE_H

#include "Node.h"
//#include "DeviceNode.h"
#include "DeviceID.h"

namespace STI
{
namespace Device
{
//
//template<class T>
//class Convertable
//{
//	T& convert() { return static_cast<T&>(*this); }		//static polymorphism via CRTP
//};
//
//template<class T>
//class Device2 : public STI::Network::Node<DeviceID, Device2<T>>, public Convertable<T>
//{
//	virtual void write(unsigned input) = 0;
//	virtual bool refresh() = 0;
//
//};

class DeviceEventDispatcher;

class Device;
//typedef STI::Network::Node<DeviceID, Device> DeviceNode;




//typedef STI::Network::Node<DeviceID, Device2> DeviceNode;

//CRTP
//pure interface for node elements
//class Device : public DeviceNode
class Device : public STI::Network::Node<DeviceID, Device>
{
public:
	virtual ~Device() {}

	virtual DeviceID getID() = 0;

	virtual void write(unsigned input) = 0;

	//virtual bool refresh() = 0;

	virtual void getEventDispatcher(std::shared_ptr<DeviceEventDispatcher>& dispatcher) = 0;

};

} //Device
} //STI

#endif
