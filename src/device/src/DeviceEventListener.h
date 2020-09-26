#ifndef STI_DEVICE_DEVICEEVENTLISTENER_H
#define STI_DEVICE_DEVICEEVENTLISTENER_H

#include "DeviceEvent.h"

#include <memory>
#include <string>

namespace STI
{
namespace Device
{

struct DeviceEventListenerID
{
	DeviceEventType type;
	std::string name;

	bool operator<(const DeviceEventListenerID& rhs) const 
	{
		return type < rhs.type && name.compare(rhs.name) < 0;
	}

	bool operator==(const DeviceEventListenerID& rhs) const
	{ 
		return type == rhs.type && (name.compare(rhs.name) == 0);
	}

};

template<class Event>
class DeviceEventListener
{
public:
	virtual void handleEvent(const std::shared_ptr<Event>& evt) = 0;
};


} //Device
} //STI


#endif
