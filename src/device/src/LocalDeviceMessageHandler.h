#ifndef STI_DEVICE_LOCALDEVICEMESSAGEHANDLER_H
#define STI_DEVICE_LOCALDEVICEMESSAGEHANDLER_H

#include <sti/device/DeviceMessageHandler.h>
#include <sti/device/DeviceMessage.h>

#include <sti/utils/EventQueue.h>
#include <sti/utils/SynchronizedMap.h>

#include <set>
#include <memory>
#include <map>
#include <mutex>


namespace STI
{
namespace Device
{

class LocalDeviceMessageHandler;
class AbstractMessageListenerGroup;

///For handling events from a single remote device.
///Holds incoming events in a queue and then distribute them to appropriate listeners.
class LocalDeviceMessageHandler : public DeviceMessageHandler
{
public:

	LocalDeviceMessageHandler();
	~LocalDeviceMessageHandler();

	void addListenerGroup(const DeviceMessageType& type, std::shared_ptr<AbstractMessageListenerGroup>& listenerGroup);
	void removeListenerGroup(const DeviceMessageType& type);

	void addMessage(const std::shared_ptr<DeviceMessage>& mess);
	void clearMessages();

	bool hasListeners(const std::shared_ptr<DeviceMessage>& mess);
	void getListenerTypes(std::set<DeviceMessageType>& types);

	void disable() {}

private:

	class HandlerEventQueue : public STI::Utils::EventQueue<std::shared_ptr<DeviceMessage>>
	{
	public:
		HandlerEventQueue(LocalDeviceMessageHandler* handler) : handler(handler) {}
		~HandlerEventQueue()
		{
			stop();
		}

	private:
		void handleEvent(const std::shared_ptr<DeviceMessage>& mess)
		{
			handler->handleMessage(mess);
		}

		LocalDeviceMessageHandler* handler;
	};

	void handleMessage(const std::shared_ptr<DeviceMessage>& mess);	//EventQueue implementation
	
	HandlerEventQueue eventQueue;

	using ListenerTypes = std::map<DeviceMessageType, unsigned>;
	ListenerTypes listenersTypes;
	mutable std::mutex listenersTypesMutex;

	STI::Utils::SynchronizedMap<DeviceMessageType, std::shared_ptr<AbstractMessageListenerGroup>> messageListenerGroups;

};


} //Device
} //STI

#endif
