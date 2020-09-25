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

	DeviceEventReceiver(const DeviceID& localID, const std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>>& deviceCollection);
	~DeviceEventReceiver();


	template<typename T>
	void addListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID, 
		const std::shared_ptr<DeviceEventListener<T>>& listener)
	{
//		typedef ListenerGroupMap<T>::TMap::value_type::second_type::element_type TListenerGroup;
//		std::shared_ptr<TListenerGroup> newListenerGroup = std::make_shared<TListenerGroup>();
	
		bool success = false;
		std::shared_ptr<DeviceEventListenerGroup<T>> listenerGroup; //= std::make_shared<DeviceEventListenerGroup<T>>();

		

//		switch (T::getEventClassType()) {
		switch (listenerID.type) {
		case DeviceEventType::Refresh:
			success = getListenerGroup(sourceDeviceID, refreshListners, listenerGroup);
			//refreshListners.add(sourceDeviceID, newListenerGroup);
			break;
		case DeviceEventType::ChannelUpdate:
			success = getListenerGroup(sourceDeviceID, channelUpdateListners, listenerGroup);
			break;
		}

		//typedef STI::Utils::SynchronizedMap<DeviceID, 
		//	std::shared_ptr<DeviceEventListenerGroup<T>>>::TMap
		//	::value_type::second_type::element_type TListenerGroup;

		if (success) {
			listenerGroup->addListener(listenerID, listener);

			std::shared_ptr<DeviceEventHandler> handler;

			//refresh any installed handlers with updated listener group
			if (handlers.get(sourceDeviceID, handler) && handler != 0) {
				refreshListenerGroups(sourceDeviceID, handler);
			}
		}



		//makeNewHandler(sourceDeviceID);	//conditionally add if it doesn't exist

		//std::shared_ptr<LocalDeviceEventHandler> handler;

		//if (handlers.get(sourceDeviceID, handler) && handler != 0) {
		//	handler->addListener(listenerID, listener);

		//	addHandlerToDispatcher(sourceDeviceID);		//refresh, overwriting old handler
		//}
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

	void addDeviceEventHandler(const DeviceID& sourceDeviceID);
	void removeDeviceEventHandler(const DeviceID& sourceDeviceID);

	bool getSourceDeviceEventDispatcher(const DeviceID& sourceDeviceID, std::shared_ptr<DeviceEventDispatcher>& dispatcher);

	void refreshListenerGroups(const DeviceID& sourceDeviceID, const std::shared_ptr<DeviceEventHandler>& handler);

	//void makeNewHandler(const DeviceID& sourceDeviceID);
	//void addHandlerToDispatcher(const DeviceID& sourceDeviceID);

	//Store ListenerGroups instead; support addListener locally for all event types.
	//need something to hold all the DeviceEventListenerGroup instances, to replace the handler...

//	typedef STI::Utils::SynchronizedMap<DeviceEventType, std::shared_ptr<DeviceEventListenerGroup<...>>> ListenerGroupMap;
//	STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<ListenerGroupMap>> listeners;

	


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

	//refresh the listener group held by the event handler for a particular event type T
	template<typename T>
	void refreshListenerGroup(const DeviceID& sourceDeviceID, ListenerGroupMap<T>& listenerGroupMap, const std::shared_ptr<DeviceEventHandler>& handler)
	{
		if (handler == 0) {
			return;
		}

		std::shared_ptr<DeviceEventListenerGroup<T>> listenerGroup;

		bool success = false;

		if (listenerGroupMap.contains(sourceDeviceID) && 
			getListenerGroup(sourceDeviceID, listenerGroupMap, listenerGroup) &&
			listenerGroup->size() > 0) {
			
			//add newest version of the group to the handler
			handler->addListenerGroup(T::getEventClassType(),
				std::static_pointer_cast<AbstractEventListenerGroup>(listenerGroup));
		}
		else {
			//remove the group from the handler
			handler->removeListenerGroup(T::getEventClassType());
		}
	}


	////////////////////////////
	//The Receiver can hold all the listeners in a set of specific maps:


	ListenerGroupMap<RefreshDeviceEvent> refreshListners;
	ListenerGroupMap<ChannelUpdateDeviceEvent> channelUpdateListners;

//	std::vector<std::shared_ptr<AbstractEventListenerGroup>> eventListenerGroups;

//	STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<DeviceEventListenerGroup<RefreshDeviceEvent>>> refreshListners;
//	STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<DeviceEventListenerGroup<ChannelUpdateDeviceEvent>>> channelUpdateListners;
	//...
	//This is natural because the Reciever is the interface for adding Listeners, so it has direct access
	//via these maps, rather than through another step using the Handler.

	STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<DeviceEventHandler>> handlers;	//DeviceID refers to a remote device

	std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>> deviceCollection;
	const DeviceID localID;		//this device's DeviceID


};


} //Device
} //STI


#endif

