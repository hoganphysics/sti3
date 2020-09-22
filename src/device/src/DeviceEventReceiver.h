#ifndef STI_DEVICE_DEVICEEVENTRECEIVER_H
#define STI_DEVICE_DEVICEEVENTRECEIVER_H

#include "DeviceID.h"
#include "DeviceEventHandler.h"
#include "SynchronizedMap.h"
#include "LocalCollection.h"

#include <memory>
#include <string>


namespace STI
{
namespace Device
{

class Device;


class DeviceEventReceiver
{
public:

	DeviceEventReceiver(const DeviceID& localID, const std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>>& deviceCollection);
	~DeviceEventReceiver();

	void addDeviceEventHandler(const DeviceID& sourceDeviceID);
	void removeDeviceEventHandler(const DeviceID& sourceDeviceID);

	template<typename T>
	void addListener(const DeviceID& sourceDeviceID, const std::string& listenerID, const std::shared_ptr<DeviceEventListener<T>>& listener)
	{
		makeNewHandler(sourceDeviceID);	//conditionally add if it doesn't exist

		std::shared_ptr<DeviceEventHandler> handler;

		if (handlers.get(sourceDeviceID, handler) && handler != 0) {
			handler->addListener(listenerID, listener);

			addHandlerToDispatcher(sourceDeviceID);		//refresh, overwriting old handler
		}
	}

	void removeListener(const DeviceID& sourceDeviceID, const std::string& listenerID);

private:

	class CollectionListener : public STI::Utils::LocalCollectionListener<DeviceID>
	{
	public:
		CollectionListener(DeviceEventReceiver* receiver) : receiver(receiver) {}

		void add(const DeviceID& id) { receiver->addDeviceEventHandler(id); }
		void remove(const DeviceID& id) { receiver->removeDeviceEventHandler(id); }
		void refresh() {}
	
	private:
		DeviceEventReceiver* receiver;
	};


	void makeNewHandler(const DeviceID& sourceDeviceID);
	void addHandlerToDispatcher(const DeviceID& sourceDeviceID);

	const DeviceID localID;		//this device's DeviceID

	//DeviceID refers to a remote device
	STI::Utils::SynchronizedMap<DeviceID, std::shared_ptr<DeviceEventHandler>> handlers;

	std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>> deviceCollection;

};


} //Device
} //STI


#endif

