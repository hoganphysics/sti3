#ifndef STI_DEVICE_LOCALDEVICEEVENTHANDLER_H
#define STI_DEVICE_LOCALDEVICEEVENTHANDLER_H

#include "DeviceEventHandler.h"
#include "DeviceEvent.h"

#include "EventQueue.h"
#include "SynchronizedMap.h"

#include <set>
#include <memory>
#include <map>


namespace STI
{
namespace Device
{

class LocalDeviceEventHandler;
class AbstractEventListenerGroup;

///For handling events from a single remote device.
///Holds incoming events in a queue and then distribute them to appropriate listeners.
class LocalDeviceEventHandler : public DeviceEventHandler
{
public:

	LocalDeviceEventHandler();
	~LocalDeviceEventHandler();

	void addListenerGroup(const DeviceEventType& type, std::shared_ptr<AbstractEventListenerGroup>& listenerGroup);
	void removeListenerGroup(const DeviceEventType& type);

	void addEvent(const std::shared_ptr<DeviceEvent>& evt);
	void clearEvents();

	bool hasListeners(const std::shared_ptr<DeviceEvent>& evt);
	void getListenerTypes(std::set<DeviceEventType>& types);

private:

	class HandlerEventQueue : public STI::Utils::EventQueue<std::shared_ptr<DeviceEvent>>
	{
	public:
		HandlerEventQueue(LocalDeviceEventHandler* handler) : handler(handler) {}

	private:
		void handleEvent(const std::shared_ptr<DeviceEvent>& evt)
		{
			handler->handleEvent(evt);
		}

		LocalDeviceEventHandler* handler;
	};

	void handleEvent(const std::shared_ptr<DeviceEvent>& evt);	//EventQueue implementation
	
	HandlerEventQueue eventQueue;

	using ListenerTypes = std::map<DeviceEventType, unsigned>;
	ListenerTypes listenersTypes;

	STI::Utils::SynchronizedMap<DeviceEventType, std::shared_ptr<AbstractEventListenerGroup>> eventListenerGroups;

};


} //Device
} //STI


#endif

