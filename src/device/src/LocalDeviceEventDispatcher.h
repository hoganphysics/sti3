#ifndef STI_DEVICE_LOCALDEVICEEVENTDISPATCHER_H
#define STI_DEVICE_LOCALDEVICEEVENTDISPATCHER_H

#include "DeviceEventDispatcher.h"
#include "DeviceEvent.h"
#include "EventQueue.h"
#include "SynchronizedMap.h"

#include <memory>


namespace STI
{
namespace Device
{

class DeviceEventHandler;

/// Devices push all locally generated events to the DeviceEventDispatcher where they are queued.  
/// The dispatcher holds references to remote event handlers that subscribe to specific event types.
//class DeviceEventDispatcher //: public STI::Utils::EventQueue<std::shared_ptr<DeviceEvent>>
class LocalDeviceEventDispatcher : public DeviceEventDispatcher
{
public:

	LocalDeviceEventDispatcher();
	~LocalDeviceEventDispatcher();

	void addEventHandler(const DeviceID& targetID, const std::shared_ptr<DeviceEventHandler>& handler);
	void removeEventHandler(const DeviceID& targetID);
	bool makeEventHandler(std::shared_ptr<DeviceEventHandler>& handler);

	void addEvent(const std::shared_ptr<DeviceEvent>& evt);
	void clearEvents();

private:

	class DispatcherEventQueue : public STI::Utils::EventQueue<std::shared_ptr<DeviceEvent>>
	{
	public:
		DispatcherEventQueue(LocalDeviceEventDispatcher* dispatcher) : dispatcher(dispatcher) {}

	private:
		void handleEvent(const std::shared_ptr<DeviceEvent>& evt)
		{
			dispatcher->handleEvent(evt);
		}

		LocalDeviceEventDispatcher* dispatcher;
	};

	DispatcherEventQueue eventQueue;

	//EventQueue implementation
	void handleEvent(const std::shared_ptr<DeviceEvent>& evt);

	STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<DeviceEventHandler>> handlers;
};

} //Device
} //STI


#endif

