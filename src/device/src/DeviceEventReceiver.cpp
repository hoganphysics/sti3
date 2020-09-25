
#include "DeviceEventReceiver.h"
#include "DeviceID.h"
#include "Device.h"
#include "DeviceEventDispatcher.h"
#include "DeviceEventHandler.h"

#include <string>


using STI::Device::DeviceEventReceiver;
using STI::Device::DeviceEventDispatcher;
using STI::Device::DeviceEventHandler;
using STI::Device::DeviceID;
using STI::Device::Device;
using STI::Utils::LocalCollection;
using STI::Device::DeviceEventListenerID;
using STI::Device::LocalDeviceEventHandler;

DeviceEventReceiver::DeviceEventReceiver(const DeviceID& localID, 
	const std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>>& deviceCollection)
	: deviceCollection(deviceCollection), localID(localID)
{
	//Add listener for add/remove of device references to LocalCollection.
	//This way when a new device is added, an event handler is automatically to the device's dispatcher.
	auto collectionListener = std::make_shared<CollectionListener>(this);
	deviceCollection->addListener(collectionListener);
}

DeviceEventReceiver::~DeviceEventReceiver()
{
}

bool DeviceEventReceiver::getSourceDeviceEventDispatcher(const DeviceID& sourceDeviceID, std::shared_ptr<DeviceEventDispatcher>& dispatcher)
{
	bool success = false;
	std::shared_ptr<Device> device;

	if (deviceCollection->get(sourceDeviceID, device) && device != 0) {

		device->getEventDispatcher(dispatcher);

		success = (dispatcher != 0);
	}

	return success;
}

void DeviceEventReceiver::addDeviceEventHandler(const DeviceID& sourceDeviceID)
{
	std::shared_ptr<DeviceEventDispatcher> dispatcher;
	std::shared_ptr<DeviceEventHandler> handler;

	if (getSourceDeviceEventDispatcher(sourceDeviceID, dispatcher) && 
		dispatcher->makeEventHandler(handler)) 
	{
		refreshListenerGroups(sourceDeviceID, handler);
//		dispatcher->removeEventHandler(localID);
		dispatcher->addEventHandler(localID, handler);	//add this handler to the remote dispatcher under the local ID
		handlers.add(sourceDeviceID, handler);
	}

}
//
//void DeviceEventReceiver::addHandlerToDispatcher(const DeviceID& sourceDeviceID)
//{
//	//Add the DeviceEventHandler to the remote DeviceEventDispatcher
//	std::shared_ptr<Device> device;
//	std::shared_ptr<DeviceEventDispatcher> dispatcher;
//	std::shared_ptr<DeviceEventHandler> handler;
//
//	if (deviceCollection->get(sourceDeviceID, device) && device != 0) {
//
//		device->getEventDispatcher(dispatcher);
//
//		if (dispatcher != 0 && dispatcher->makeEventHandler(handler)) {
//			refreshListenerGroups(handler);
//			dispatcher->addEventHandler(localID, handler);		//add this handler to the remote dispatcher under the local ID
//		}
//	}
//}

void DeviceEventReceiver::refreshListenerGroups(const DeviceID& sourceDeviceID, const std::shared_ptr<DeviceEventHandler>& handler)
{
	//mutex lock this to protect from add and remove listener
	//bool success = false;

	//std::shared_ptr<DeviceEventListenerGroup<RefreshDeviceEvent>> listenerGroup;
	//success = getListenerGroup(sourceDeviceID, refreshListners, listenerGroup);
	//if (listenerGroup->size() > 0) {
	//	handler->addListenerGroup(RefreshDeviceEvent::getEventClassType(), 
	//		std::static_pointer_cast<AbstractEventListenerGroup>(listenerGroup));
	//}

	refreshListenerGroup(sourceDeviceID, refreshListners, handler);
	refreshListenerGroup(sourceDeviceID, channelUpdateListners, handler);
}


//void DeviceEventReceiver::makeNewHandler(const DeviceID& sourceDeviceID)
//{
//	if (!handlers.contains(sourceDeviceID)) {
//		std::shared_ptr<LocalDeviceEventHandler> newHandler = std::make_shared<LocalDeviceEventHandler>();
//
//		if (newHandler != 0) {
//			handlers.add(sourceDeviceID, newHandler);
//		}
//	}
//}

//void DeviceEventReceiver::addDeviceEventHandler(const DeviceID& sourceDeviceID)
//{
//	makeNewHandler(sourceDeviceID);
//
//	addHandlerToDispatcher(sourceDeviceID);
//}

void DeviceEventReceiver::removeDeviceEventHandler(const DeviceID& sourceDeviceID)
{
	handlers.remove(sourceDeviceID);

	////Attempt to remove the DeviceEventHandler from the remote DeviceEventDispatcher
	//std::shared_ptr<Device> device;
	//std::shared_ptr<DeviceEventDispatcher> dispatcher;

	//if (deviceCollection->get(sourceDeviceID, device) && device != 0) {

	//	device->getEventDispatcher(dispatcher);

	//	if (dispatcher != 0) {
	//		dispatcher->removeEventHandler(localID);
	//	}
	//}
}


void DeviceEventReceiver::removeListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID)
{
	bool success = false;

	switch (listenerID.type) {
	case DeviceEventType::Refresh:
		success = removeListener(sourceDeviceID, listenerID, refreshListners);
		//refreshListners.add(sourceDeviceID, newListenerGroup);
		break;
	case DeviceEventType::ChannelUpdate:
		success = removeListener(sourceDeviceID, listenerID, channelUpdateListners);
		break;
	}
	
	if (success) {
		std::shared_ptr<DeviceEventHandler> handler;

		//refresh any installed handlers with updated listener group
		if (handlers.get(sourceDeviceID, handler) && handler != 0) {
			refreshListenerGroups(sourceDeviceID, handler);
		}
	}


	//std::shared_ptr<LocalDeviceEventHandler> handler;

	//if (handlers.get(sourceDeviceID, handler) && handler != 0) {
	//	handler->removeListener(listenerID);

	//	addHandlerToDispatcher(sourceDeviceID);		//refresh, overwriting old handler
	//}
}

