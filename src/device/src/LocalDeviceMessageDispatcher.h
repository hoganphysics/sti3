#ifndef STI_DEVICE_LOCALDEVICEMESSAGEDISPATCHER_H
#define STI_DEVICE_LOCALDEVICEMESSAGEDISPATCHER_H

#include "DeviceMessageDispatcher.h"
#include "DeviceMessage.h"
#include "EventQueue.h"
#include "SynchronizedMap.h"

#include <memory>


namespace STI
{
namespace Device
{

class DeviceMessageHandler;

/// Devices push all locally generated messages to the DeviceMessageDispatcher where they are queued.  
/// The dispatcher holds references to remote message handlers that subscribe to specific message types.
//class DeviceMessageDispatcher //: public STI::Utils::EventQueue<std::shared_ptr<DeviceMessage>>
class LocalDeviceMessageDispatcher : public DeviceMessageDispatcher
{
public:

	LocalDeviceMessageDispatcher();
	~LocalDeviceMessageDispatcher();

	void addMessageHandler(const DeviceID& targetID, const std::shared_ptr<DeviceMessageHandler>& handler);
	void removeMessageHandler(const DeviceID& targetID);
	bool makeMessageHandler(std::shared_ptr<DeviceMessageHandler>& handler);

	void addMessage(const std::shared_ptr<DeviceMessage>& mess);
	void clearMessages();

private:

	class DispatcherEventQueue : public STI::Utils::EventQueue<std::shared_ptr<DeviceMessage>>
	{
	public:
		DispatcherEventQueue(LocalDeviceMessageDispatcher* dispatcher) : dispatcher(dispatcher) {}

	private:
		void handleEvent(const std::shared_ptr<DeviceMessage>& mess)
		{
			dispatcher->handleMessage(mess);
		}

		LocalDeviceMessageDispatcher* dispatcher;
	};

	DispatcherEventQueue eventQueue;

	// DispatcherEventQueue : EventQueue implementation
	void handleMessage(const std::shared_ptr<DeviceMessage>& mess);

	STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<DeviceMessageHandler>> handlers;
};

} //Device
} //STI


#endif

