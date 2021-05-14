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
		if (type == rhs.type) {
			return name.compare(rhs.name) < 0;
		}

		return type < rhs.type;

		// return type < rhs.type && name.compare(rhs.name) < 0;
	}

	bool operator==(const DeviceMessageListenerID& rhs) const
	{ 
		return type == rhs.type && (name.compare(rhs.name) == 0);
	}

	bool operator!=(const DeviceMessageListenerID& rhs) const
	{
		return !((*this) == rhs);
	}

};

template<class Message>
class DeviceMessageListener
{
public:

	virtual ~DeviceMessageListener() {}
	
	virtual void handleMessage(const std::shared_ptr<Message>& mess) = 0;
};


// template<class Message>
// class DeviceMessageListenerRAII : public DeviceMessageListener<Message>
// {
// 	virtual ~DeviceMessageListenerRAII()
// 	{
// 		if (receiver != 0) {
// 			receiver->removeListener(serverID, schedulerMessageLID);
// 		}
// 	}

// 	std::shared_ptr<DeviceMessageReceiver> receiver;
// };

} //Device
} //STI


#endif
