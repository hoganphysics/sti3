#ifndef STI_DEVICE_DEVICEEVENTHANDLER_H
#define STI_DEVICE_DEVICEEVENTHANDLER_H

#include "DeviceEvent.h"
#include "DeviceEventListener.h"
#include "SynchronizedMap.h"
#include "EventQueue.h"

#include <set>
#include <typeinfo>
#include <vector>
#include <memory>

namespace STI
{
namespace Device
{

template<class Event>
class DeviceEventHandlerGroup
{
public:

	unsigned size() { return static_cast<unsigned>(listeners.size()); }

	void addListener(const std::string& id, const std::shared_ptr<DeviceEventListener<Event>>& listener)
	{
		listeners.add(id, listener);
//		listeners.push_back(listener);
	}

	template<typename T>
	void addListener(const std::string& id, T listener) {}	//catch

	void removeListener(const std::string& id)
	{
		listeners.remove(id);
	}


	void handleEvent(const Event& evt)
	{
		std::set<std::string> ids;
		listeners.getKeys(ids);

		std::shared_ptr<DeviceEventListener<Event>> listener;

		for (auto& id : ids) {
			if (listeners.get(id, listener) && listener != 0) {

				listener->handleEvent(evt);
			}
		}
	}

private:

//	std::vector<DeviceEventListener<Event>> listeners;
	STI::Utils::SynchronizedMap<std::string, std::shared_ptr<DeviceEventListener<Event>>> listeners;

};

//For handling events from a single partner device
class DeviceEventHandler : public STI::Utils::EventQueue<DeviceEvent>
{
public:

	DeviceEventHandler();
	~DeviceEventHandler();

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

	void refresh() {}

	void addListener(const std::string& id, const std::shared_ptr<DeviceEventListener<RefreshDeviceEvent>>& listener)
	{
		refreshListeners.addListener(id, listener);
		
		listenersTypes[DeviceEventType::Refresh] = refreshListeners.size();
		refresh();
	}

	void removeListener(const std::string& id)
	{
		refreshListeners.removeListener(id);

		listenersTypes[DeviceEventType::Refresh] = refreshListeners.size();
		refresh();
	}

	//void addListener(const std::string& id, const std::shared_ptr<DeviceEventListener<DeviceEvent>>& listener)
	//{
	//	genericListeners.addListener(id, listener);
	//}

	bool hasListeners(const DeviceEvent& evt);

private:

	//EventQueue implementation
	void handleEvent(const DeviceEvent& evt);


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
	std::map<DeviceEventType, unsigned> listenersTypes;

	DeviceEventHandlerGroup<RefreshDeviceEvent> refreshListeners;
	
	//DeviceEventHandlerGroup<DeviceEvent> genericListeners;

};


} //Device
} //STI


#endif

