#ifndef STI_DEVICE_DEVICEEVENTRECEIVER_H
#define STI_DEVICE_DEVICEEVENTRECEIVER_H

#include "DeviceID.h"
#include "LocalDeviceEventHandler.h"
#include "SynchronizedMap.h"
#include "LocalCollection.h"
#include "DeviceEventListenerGroup.h"

#include <memory>
#include <string>


namespace STI
{
namespace Device
{

class Device;
class DeviceEventDispatcher;

class DeviceEventReceiver
{
public:

	DeviceEventReceiver(const DeviceID& localID, 
		const std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>>& deviceCollection);
	~DeviceEventReceiver();

	template<typename T>
	void addListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID, 
		const std::shared_ptr<DeviceEventListener<T>>& listener)
	{

		bool success = false;
		std::shared_ptr<DeviceEventListenerGroup<T>> listenerGroup;

		switch (listenerID.type) {
		case DeviceEventType::Refresh:
			success = getListenerGroup(sourceDeviceID, refreshListners, listenerGroup);
			break;
		case DeviceEventType::ChannelUpdate:
			success = getListenerGroup(sourceDeviceID, channelUpdateListners, listenerGroup);
			break;
		}

		if (success) {
			listenerGroup->addListener(listenerID, listener);

			std::shared_ptr<DeviceEventHandler> handler;

			//refresh any installed handlers with updated listener group
			if (handlers.get(sourceDeviceID, handler) && handler != 0) {
				refreshListenerGroups(sourceDeviceID, handler);
			}
		}
	}

	void removeListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID);

private:

	class CollectionListener : public STI::Utils::LocalCollectionListener<DeviceID>
	{
	public:
		CollectionListener(DeviceEventReceiver* receiver) : receiver(receiver) {}

		void add(const DeviceID& id) { receiver->addDeviceEventHandler(id); }
		void remove(const DeviceID& id) { receiver->removeDeviceEventHandler(id); }
		void refresh() { }
	
	private:
		DeviceEventReceiver* receiver;
	};


	//Managing EventHandler references to the DeviceEventDispatcher of the remote device
	void addDeviceEventHandler(const DeviceID& sourceDeviceID);
	void removeDeviceEventHandler(const DeviceID& sourceDeviceID);
	bool getSourceDeviceEventDispatcher(const DeviceID& sourceDeviceID, std::shared_ptr<DeviceEventDispatcher>& dispatcher);
	void refreshListenerGroups(const DeviceID& sourceDeviceID, const std::shared_ptr<DeviceEventHandler>& handler);
	void removeAllHandlers();

	template <typename T>
	using ListenerGroupMap = STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<DeviceEventListenerGroup<T>>>;

	template<typename T, typename U>
	bool getListenerGroup(const DeviceID& sourceDeviceID, ListenerGroupMap<T>& listenerGroupMap, std::shared_ptr<DeviceEventListenerGroup<U>>& listenerGroup)
	{
		return false;	//catch mismatch (U != T)
	}

	template<typename T>
	bool getListenerGroup(const DeviceID& sourceDeviceID, ListenerGroupMap<T>& listenerGroupMap, std::shared_ptr<DeviceEventListenerGroup<T>>& listenerGroup)
	{
		if (!listenerGroupMap.contains(sourceDeviceID)) {
			listenerGroup = std::make_shared<DeviceEventListenerGroup<T>>();
			listenerGroupMap.add(sourceDeviceID, listenerGroup);
		}

		return listenerGroupMap.get(sourceDeviceID, listenerGroup) && listenerGroup != 0;
	}

	///Refresh the listener group held by the event handler for a particular event type T
	///associated with events sourced for sourceDeviceID.
	template<typename T>
	void refreshListenerGroup(const DeviceID& sourceDeviceID, typename DeviceEventReceiver::ListenerGroupMap<T>& listenerGroupMap, 
		const std::shared_ptr<DeviceEventHandler>& handler)
	{
		if (handler == 0) {
			return;
		}

		bool success = false;
		typename std::shared_ptr<DeviceEventListenerGroup<T>> listenerGroup;

		if (listenerGroupMap.contains(sourceDeviceID) && 
			getListenerGroup(sourceDeviceID, listenerGroupMap, listenerGroup) &&
			listenerGroup->size() > 0) {
			
			auto abstractListenerGroup = std::static_pointer_cast<AbstractEventListenerGroup>(listenerGroup);

			//add newest version of the group to the handler
			handler->addListenerGroup(T::getEventClassType(), abstractListenerGroup);
		}
		else {
			//remove the group from the handler
			handler->removeListenerGroup(T::getEventClassType());
		}
	}

	template<typename T>
	bool removeListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID,
		ListenerGroupMap<T>& listenerGroupMap)
	{
		bool success = false;
		std::shared_ptr<DeviceEventListenerGroup<T>> listenerGroup;
		
		if (listenerGroupMap.get(sourceDeviceID, listenerGroup) && listenerGroup != 0) {

			listenerGroup->removeListener(listenerID);

			success = !listenerGroup->contains(listenerID);
		}
		return success;
	}

	//All attached listeners are collected in groups, based on the type of event they listen to.
	//The event type is the template parameter of the ListenerGroupMap type.
	//These listener groups are stored here in a map, keyed by the DeviceID of the event's source
	//(that is, the remote device that they are listening to).
	ListenerGroupMap<RefreshDeviceEvent> refreshListners;
	ListenerGroupMap<ChannelUpdateDeviceEvent> channelUpdateListners;
	//...

	/**
	Each remote device gets a reference to a DeviceEventHandler, which accepts and queue all
	events from the remote device. DeviceEventHandlers can contain multiple listener groups 
	(depending on which type of events are listened to).  The set of all handlers for the local 
	device are stored here (for all remote devices it listens to).
	**/
	STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<DeviceEventHandler>> handlers;	//DeviceID refers to a remote device

	std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>> deviceCollection;
	const DeviceID localID;		//this device's DeviceID

};


} //Device
} //STI


#endif

