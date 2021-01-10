#ifndef STI_DEVICE_DEVICEMESSAGELISTENER_H
#define STI_DEVICE_DEVICEMESSAGELISTENER_H

#include "DeviceMessage.h"

#include <memory>
#include <string>

namespace STI
{
namespace Device
{

struct DeviceMessageListenerID
{
	DeviceMessageType type;
	std::string name;

	bool operator<(const DeviceMessageListenerID& rhs) const 
	{
		return type < rhs.type && name.compare(rhs.name) < 0;
	}

	bool operator==(const DeviceMessageListenerID& rhs) const
	{ 
		return type == rhs.type && (name.compare(rhs.name) == 0);
	}

};

template<class Message>
class DeviceMessageListener
{
public:

	virtual ~DeviceMessageListener() {}
	
	virtual void handleMessage(const std::shared_ptr<Message>& mess) = 0;
};


} //Device
} //STI


#endif
