#ifndef STI_DEVICE_DEVICEMESSAGERECEIVER_H
#define STI_DEVICE_DEVICEMESSAGERECEIVER_H

#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessageHandler.h>
#include <sti/utils/SynchronizedMap.h>
#include <sti/utils/LocalCollection.h>
#include <sti/device/DeviceMessageListenerGroup.h>

#include <functional>
#include <memory>
#include <string>


namespace STI
{
namespace Device
{


class Device;
class DeviceMessageDispatcher;

class DeviceMessageReceiver
{
public:

	DeviceMessageReceiver(const DeviceID& localID, 
		const std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>>& deviceCollection,
		const std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
	~DeviceMessageReceiver();

	template<typename T>
	void addListener(const DeviceID& sourceDeviceID, const std::string& listenerName, 
		const std::function<void (const std::shared_ptr<T>&)>& handler)
	{
		addListener(sourceDeviceID, DeviceMessageListenerID(T::getMessageClassType(), listenerName), handler);
	}

	template<typename T>
	void addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
		const std::function<void (const std::shared_ptr<T>&)>& handler)
	{
		auto listener = std::make_shared<DeviceMessageListenerLambda<T>>(handler);
		addListener(sourceDeviceID, listenerID, std::static_pointer_cast<DeviceMessageListener<T>>(listener));
	}

	template<typename T, typename U>
	void addListener(const DeviceID& sourceDeviceID, const std::string& listenerName, const std::shared_ptr<U>& listener)
	{
		addListener(sourceDeviceID, DeviceMessageListenerID(T::getMessageClassType(), listenerName), 
			std::static_pointer_cast<DeviceMessageListener<T>>(listener));
	}

	template<typename T>
	void addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
		const std::shared_ptr<DeviceMessageListener<T>>& listener)
	{
		bool success = false;
		std::shared_ptr<DeviceMessageListenerGroup<T>> listenerGroup;

		switch (listenerID.type) {
		case DeviceMessageType::Refresh:
			success = getListenerGroup(sourceDeviceID, refreshListeners, listenerGroup);
			break;
		case DeviceMessageType::ChannelUpdate:
			success = getListenerGroup(sourceDeviceID, channelUpdateListeners, listenerGroup);
			break;
		case DeviceMessageType::AttributeUpdate:
			success = getListenerGroup(sourceDeviceID, attributeUpdateListeners, listenerGroup);
			break;
		case DeviceMessageType::MonitorUpdate:
			success = getListenerGroup(sourceDeviceID, monitorUpdateListeners, listenerGroup);
			break;
		case DeviceMessageType::MonitorStatusUpdate:
			success = getListenerGroup(sourceDeviceID, monitorStatusUpdateListeners, listenerGroup);
			break;
		case DeviceMessageType::EngineScheduler:
			success = getListenerGroup(sourceDeviceID, engineSchedulerListeners, listenerGroup);
			break;
		case DeviceMessageType::EngineParser:
			success = getListenerGroup(sourceDeviceID, engineParserListeners, listenerGroup);
			break;
		case DeviceMessageType::CollectionUpdate:
			success = getListenerGroup(sourceDeviceID, collectionUpdateListeners, listenerGroup);
			break;
		case DeviceMessageType::EngineStatus:
			success = getListenerGroup(sourceDeviceID, engineStateListeners, listenerGroup);
			break;
		case DeviceMessageType::EngineJobUpdate:
			success = getListenerGroup(sourceDeviceID, engineJobUpdateListeners, listenerGroup);
			break;
		case DeviceMessageType::PostProcessingComplete:
			success = getListenerGroup(sourceDeviceID, postProcessingCompleteListeners, listenerGroup);
			break;
		}


		if (success) {
			listenerGroup->addListener(listenerID, listener);

			std::shared_ptr<DeviceMessageHandler> handler;

			//refresh any installed handlers with updated listener group
			if (handlers.get(sourceDeviceID, handler) && handler != 0) {
				refreshListenerGroups(sourceDeviceID, handler);
			}
		}
	}

	void removeListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID);

	void clearListeners();

private:

	class CollectionListener : public STI::Utils::LocalCollectionListener<DeviceID>
	{
	public:
		CollectionListener(DeviceMessageReceiver* receiver) : receiver(receiver) {}

		void add(const DeviceID& id) { receiver->addDeviceMessageHandler(id); }
		void remove(const DeviceID& id) { receiver->removeDeviceMessageHandler(id); }
		void refresh() { }
	
	private:
		DeviceMessageReceiver* receiver;
	};


	//Managing EventHandler references to the DeviceMessageDispatcher of the remote device
	void addDeviceMessageHandler(const DeviceID& sourceDeviceID);
	void removeDeviceMessageHandler(const DeviceID& sourceDeviceID);
	bool getSourceDeviceMessageDispatcher(const DeviceID& sourceDeviceID, std::shared_ptr<DeviceMessageDispatcher>& dispatcher);
	void refreshListenerGroups(const DeviceID& sourceDeviceID, const std::shared_ptr<DeviceMessageHandler>& handler);
	void removeAllHandlers();

	template <typename T>
	using ListenerGroupMap = STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<DeviceMessageListenerGroup<T>>>;

	template<typename T, typename U>
	bool getListenerGroup(const DeviceID& sourceDeviceID, ListenerGroupMap<T>& listenerGroupMap, 
							std::shared_ptr<DeviceMessageListenerGroup<U>>& listenerGroup)
	{
		return false;	//catch mismatch (U != T)
	}

	template<typename T>
	bool getListenerGroup(const DeviceID& sourceDeviceID, ListenerGroupMap<T>& listenerGroupMap, 
							std::shared_ptr<DeviceMessageListenerGroup<T>>& listenerGroup)
	{
		if (!listenerGroupMap.contains(sourceDeviceID)) {
			listenerGroup = std::make_shared<DeviceMessageListenerGroup<T>>();
			listenerGroupMap.add(sourceDeviceID, listenerGroup);
		}

		return listenerGroupMap.get(sourceDeviceID, listenerGroup) && listenerGroup != 0;
	}

	///Refresh the listener group held by the event handler for a particular event type T
	///associated with events sourced for sourceDeviceID.
	template<typename T>
	void refreshListenerGroup(const DeviceID& sourceDeviceID, typename DeviceMessageReceiver::ListenerGroupMap<T>& listenerGroupMap, 
								const std::shared_ptr<DeviceMessageHandler>& handler)
	{
		if (handler == 0) {
			return;
		}

		bool success = false;
		typename std::shared_ptr<DeviceMessageListenerGroup<T>> listenerGroup;

		if (listenerGroupMap.contains(sourceDeviceID) && 
			getListenerGroup(sourceDeviceID, listenerGroupMap, listenerGroup) &&
			listenerGroup->size() > 0) {
			
			auto abstractListenerGroup = std::static_pointer_cast<AbstractMessageListenerGroup>(listenerGroup);

			//add newest version of the group to the handler
			handler->addListenerGroup(T::getMessageClassType(), abstractListenerGroup);
		}
		else {
			//remove the group from the handler
			handler->removeListenerGroup(T::getMessageClassType());
		}
	}

	template<typename T>
	bool removeListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID,
		ListenerGroupMap<T>& listenerGroupMap)
	{
		bool success = false;
		std::shared_ptr<DeviceMessageListenerGroup<T>> listenerGroup;
		
		if (listenerGroupMap.get(sourceDeviceID, listenerGroup) && listenerGroup != 0) {

			listenerGroup->removeListener(listenerID);

			success = !listenerGroup->contains(listenerID);
		}
		return success;
	}

	template<typename T>
	void clearListenerGroups(ListenerGroupMap<T>& listenerGroupMap)
	{
		std::shared_ptr<DeviceMessageListenerGroup<T>> listenerGroup;

		std::set<DeviceID> ids;
		listenerGroupMap.getKeys(ids);

		for (auto& id : ids) {
			if (listenerGroupMap.get(id, listenerGroup) && listenerGroup != 0) {
				listenerGroup->clear();
			}
		}
	}

	void clearAllListenerGroups();

	//All attached listeners are collected in groups, based on the type of event they listen to.
	//The event type is the template parameter of the ListenerGroupMap type.
	//These listener groups are stored here in a map, keyed by the DeviceID of the event's source
	//(that is, the remote device that they are listening to).
	//***CAREFUL: Do not declare multiple ListenerGroupMap<T> of the same T!
	ListenerGroupMap<RefreshDeviceMessage> refreshListeners;
	ListenerGroupMap<ChannelUpdateMessage> channelUpdateListeners;
	ListenerGroupMap<AttributeUpdateMessage> attributeUpdateListeners;
	ListenerGroupMap<MonitorUpdateMessage> monitorUpdateListeners;
	ListenerGroupMap<MonitorStatusUpdateMessage> monitorStatusUpdateListeners;
	ListenerGroupMap<EngineSchedulerMessage> engineSchedulerListeners;
	ListenerGroupMap<EngineParserDeviceMessage> engineParserListeners;
	ListenerGroupMap<CollectionUpdateMessage> collectionUpdateListeners;
	ListenerGroupMap<EngineStateMessage> engineStateListeners;
	ListenerGroupMap<EngineJobUpdateDeviceMessage> engineJobUpdateListeners;
	ListenerGroupMap<PostProcessingCompleteMessage> postProcessingCompleteListeners;
	//...

	/**
	Each remote device gets a reference to a DeviceMessageHandler, which accepts and queue all
	events from the remote device. DeviceMessageHandlers can contain multiple listener groups 
	(depending on which type of events are listened to).  The set of all handlers for the local 
	device are stored here (for all remote devices it listens to).
	**/
	STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<DeviceMessageHandler>> handlers;	//DeviceID refers to a remote device

	std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>> deviceCollection;
	std::shared_ptr<DeviceMessageDispatcher> localDispatcher;	//this device's dispatcher (for intra device messages)
	const DeviceID localID;		//this device's DeviceID

};


} //Device
} //STI


#endif

