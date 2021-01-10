#ifndef STI_DEVICE_DEVICEMESSAGELISTENERGROUP_H
#define STI_DEVICE_DEVICEMESSAGELISTENERGROUP_H

#include "DeviceMessage.h"
#include "DeviceMessageListener.h"
#include "SynchronizedMap.h"

#include <memory>

namespace STI
{
namespace Device
{

class AbstractMessageListenerGroup
{
public:

	virtual ~AbstractMessageListenerGroup() {}

	virtual unsigned size() = 0;
	virtual void handleMessage(const std::shared_ptr<DeviceMessage>& mess) = 0;

};

template<class Message>
class DeviceMessageListenerGroup : public AbstractMessageListenerGroup
{
public:

	unsigned size() { return static_cast<unsigned>(listeners.size()); }

	void addListener(const DeviceMessageListenerID& id, const std::shared_ptr<DeviceMessageListener<Message>>& listener)
	{
		listeners.add(id, listener);
	}

	template<typename T>
	void addListener(const DeviceMessageListenerID& id, T listener) {}	//catch

	void removeListener(const DeviceMessageListenerID& id)
	{
		listeners.remove(id);
	}

	bool contains(const DeviceMessageListenerID& id) const
	{
		return listeners.contains(id);
	}

	void handleMessage(const std::shared_ptr<DeviceMessage>& mess)
	{
		std::shared_ptr<Message> convertedMessage;

		if (mess->getType() == Message::getMessageClassType() && 
			DeviceMessage::convert<Message>(mess, convertedMessage)) 
		{
			handleMessage(convertedMessage);
		}
	}

private:

	void handleMessage(const std::shared_ptr<Message>& mess)
	{
		std::set<DeviceMessageListenerID> ids;
		listeners.getKeys(ids);

		std::shared_ptr<DeviceMessageListener<Message>> listener;

		for (auto& id : ids) {
			if (listeners.get(id, listener) && listener != 0) {

				listener->handleMessage(mess);
			}
		}
	}

	STI::Utils::SynchronizedMap<DeviceMessageListenerID, std::shared_ptr<DeviceMessageListener<Message>>> listeners;

};

} //Device
} //STI


#endif

