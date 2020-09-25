#ifndef STI_DEVICE_LOCALDEVICEEVENTHANDLER_H
#define STI_DEVICE_LOCALDEVICEEVENTHANDLER_H

#include "DeviceEventHandler.h"
#include "DeviceEvent.h"
#include "DeviceEventListener.h"
#include "SynchronizedMap.h"
#include "EventQueue.h"
#include "DeviceEventListenerGroup.h"

#include <set>
#include <typeinfo>
#include <vector>
#include <memory>

namespace STI
{
namespace Device
{


//For handling events from a single partner device
//class DeviceEventHandler //: public STI::Utils::EventQueue<std::shared_ptr<DeviceEvent>>
class LocalDeviceEventHandler : public DeviceEventHandler
{
public:

	LocalDeviceEventHandler();
	~LocalDeviceEventHandler();


	void addListenerGroup(const DeviceEventType& type, std::shared_ptr<AbstractEventListenerGroup>& listenerGroup)
	{
		if (eventListenerGroups.add(type, listenerGroup)) {
			listenersTypes[type] = listenerGroup->size();
		}
	}
	void removeListenerGroup(const DeviceEventType& type)
	{
		eventListenerGroups.remove(type);
		listenersTypes[type] = 0;
	}


	//template<typename T>
	//void addListener(const std::string& id, const const std::shared_ptr<DeviceEventListener<T>>& listener)
	//{
	//	refreshListeners.addListener(id, listener);
	//}

	//	template<typename T>
	////	void addListener(const T& listener)		//so derived classes with multiple interfaces can be added
	//	
	//	void addListener(const DeviceEventListener<T>& listener)
	//	{
	//		//attempt to cast to all event types?
	//		//should add to all matching listener groups
	//		listenersTypes.insert(DeviceEventType::Refresh);
	//		//...
	//
	//	}

	//template<>
	//void addListener<DeviceEvent>(const DeviceEventListener<DeviceEvent>& listener)
	//{

	//}

//	void refresh() {}

	/*void addListener(const DeviceEventListenerID& id, const std::shared_ptr<DeviceEventListener<RefreshDeviceEvent>>& listener)
	{
		refreshListeners.addListener(id, listener);

		listenersTypes[DeviceEventType::Refresh] = refreshListeners.size();
		refresh();
	}

	void removeListener(const DeviceEventListenerID& id)
	{
		switch (id.type) {
		case DeviceEventType::Refresh:
			refreshListeners.removeListener(id);
			break;
		}

		listenersTypes[DeviceEventType::Refresh] = refreshListeners.size();
		refresh();
	}*/

	//void addListener(const std::string& id, const std::shared_ptr<DeviceEventListener<DeviceEvent>>& listener)
	//{
	//	genericListeners.addListener(id, listener);
	//}

	void addEvent(const std::shared_ptr<DeviceEvent>& evt);
	void clearEvents();

	bool hasListeners(const std::shared_ptr<DeviceEvent>& evt);

	void getListenerTypes(std::set<DeviceEventType>& types)
	{
		types.clear();

		for (auto& t : listenersTypes) {
			if (t.second > 0) {			//number of listeners
				types.insert(t.first);	//listener type
			}
		}
	}

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

	HandlerEventQueue eventQueue;

	//EventQueue implementation
	void handleEvent(const std::shared_ptr<DeviceEvent>& evt);


	//template<typename T>
	//static bool convert(const DeviceEvent& evt, T& derived)
	//{
	//	bool success;

	//	try {
	//		derived = dynamic_cast<const T&>(evt);
	//		success = true;
	//	}
	//	catch (const std::bad_cast& exp) {
	//		success = false;
	//	}
	//}

	//	std::set<DeviceEventType> listenersTypes;
	using ListenerTypes = std::map<DeviceEventType, unsigned>;
	ListenerTypes listenersTypes;

	//DeviceEventListenerGroup<RefreshDeviceEvent> refreshListeners;

	STI::Utils::SynchronizedMap<DeviceEventType, std::shared_ptr<AbstractEventListenerGroup>> eventListenerGroups;

	//DeviceEventHandlerGroup<DeviceEvent> genericListeners;

};


} //Device
} //STI


#endif

