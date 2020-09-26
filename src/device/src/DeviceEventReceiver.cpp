
#include "DeviceEventReceiver.h"
#include "DeviceID.h"
#include "Device.h"
#include "DeviceEventDispatcher.h"
#include "DeviceEventHandler.h"

#include <string>
#include <set>

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
	removeAllHandlers();	//remove remote handlers
}


void DeviceEventReceiver::addDeviceEventHandler(const DeviceID& sourceDeviceID)
{
	std::shared_ptr<DeviceEventDispatcher> dispatcher;
	std::shared_ptr<DeviceEventHandler> handler;

	if (getSourceDeviceEventDispatcher(sourceDeviceID, dispatcher) && 
		dispatcher->makeEventHandler(handler)) 
	{
		refreshListenerGroups(sourceDeviceID, handler);	//populate handler with existing listeners
		dispatcher->addEventHandler(localID, handler);	//add this handler to the remote dispatcher under the local ID
		handlers.add(sourceDeviceID, handler);			//store handler refernce locally
	}
}


void DeviceEventReceiver::removeDeviceEventHandler(const DeviceID& sourceDeviceID)
{
	std::shared_ptr<DeviceEventDispatcher> dispatcher;

	//Attempt to remove from the remote dispatcher
	if (getSourceDeviceEventDispatcher(sourceDeviceID, dispatcher)) {
		dispatcher->removeEventHandler(localID);	//remove this handler to the remote dispatcher
	}

	handlers.remove(sourceDeviceID);
}


void DeviceEventReceiver::removeAllHandlers()
{
	std::set<DeviceID> ids;
	handlers.getKeys(ids);

	for (auto& handerID : ids) {
		removeDeviceEventHandler(handerID);
	}
}


bool DeviceEventReceiver::getSourceDeviceEventDispatcher(const DeviceID& sourceDeviceID, 
	std::shared_ptr<DeviceEventDispatcher>& dispatcher)
{
	bool success = false;
	std::shared_ptr<Device> device;

	if (deviceCollection->get(sourceDeviceID, device) && device != 0) {

		device->getEventDispatcher(dispatcher);

		success = (dispatcher != 0);
	}

	return success;
}


void DeviceEventReceiver::refreshListenerGroups(const DeviceID& sourceDeviceID, 
	const std::shared_ptr<DeviceEventHandler>& handler)
{
	refreshListenerGroup(sourceDeviceID, refreshListners, handler);
	refreshListenerGroup(sourceDeviceID, channelUpdateListners, handler);
}


void DeviceEventReceiver::removeListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID)
{
	bool success = false;

	switch (listenerID.type) {
	case DeviceEventType::Refresh:
		success = removeListener(sourceDeviceID, listenerID, refreshListners);
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
}

