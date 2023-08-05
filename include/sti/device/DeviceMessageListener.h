#ifndef STI_DEVICE_DEVICEMESSAGELISTENER_H
#define STI_DEVICE_DEVICEMESSAGELISTENER_H

//#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageType.h>

#include <memory>
#include <string>
#include <functional>

namespace STI
{
namespace Device
{


class DeviceMessageListenerID
{
public:
	
	DeviceMessageListenerID()
	: DeviceMessageListenerID(DeviceMessageType::Unknown, "") {}

	DeviceMessageListenerID(const DeviceMessageType& type, const std::string& name) 
	: type(type), name(name) {}

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

	// DeviceMessageListener() {}

	virtual ~DeviceMessageListener() {}
	
	virtual void handleMessage(const std::shared_ptr<Message>& mess) = 0;
};

template<class Message>
class DeviceMessageListenerLambda : public DeviceMessageListener<Message>
{
public:

	DeviceMessageListenerLambda(const std::function<void (const std::shared_ptr<Message>&)>& handler)
	{
		_handler = handler;
	}

	~DeviceMessageListenerLambda() {}
	
	void handleMessage(const std::shared_ptr<Message>& mess)
	{
		_handler(mess);
	}

private:

	std::function<void (const std::shared_ptr<Message>&)> _handler;
};


} //Device
} //STI


#endif
