
#include "DeviceMessageReceiver.h"
#include "DeviceID.h"
#include "Device.h"
#include "DeviceMessageDispatcher.h"
#include "DeviceMessageHandler.h"

#include <string>
#include <set>

using STI::Device::DeviceMessageReceiver;
using STI::Device::DeviceMessageDispatcher;
using STI::Device::DeviceMessageHandler;
using STI::Device::DeviceID;
using STI::Device::Device;
using STI::Utils::LocalCollection;
using STI::Device::DeviceMessageListenerID;
using STI::Device::LocalDeviceMessageHandler;


DeviceMessageReceiver::DeviceMessageReceiver(const DeviceID& localID, 
	const std::shared_ptr<STI::Utils::LocalCollection<DeviceID, Device>>& deviceCollection)
	: deviceCollection(deviceCollection), localID(localID)
{
	//Add listener for add/remove of device references to LocalCollection.
	//This way when a new device is added, an event handler is automatically added to the device's dispatcher.
	auto collectionListener = std::make_shared<CollectionListener>(this);
	deviceCollection->addListener(collectionListener);
}


DeviceMessageReceiver::~DeviceMessageReceiver()
{
	removeAllHandlers();	//remove remote handlers
}


void DeviceMessageReceiver::addDeviceMessageHandler(const DeviceID& sourceDeviceID)
{
	std::shared_ptr<DeviceMessageDispatcher> dispatcher;
	std::shared_ptr<DeviceMessageHandler> handler;

	if (getSourceDeviceMessageDispatcher(sourceDeviceID, dispatcher) && 
		dispatcher->makeMessageHandler(handler)) 
	{
		refreshListenerGroups(sourceDeviceID, handler);	//populate handler with existing listeners
		dispatcher->addMessageHandler(localID, handler);	//add this handler to the remote dispatcher under the local ID
		handlers.add(sourceDeviceID, handler);			//store handler reference locally
	}
}


void DeviceMessageReceiver::removeDeviceMessageHandler(const DeviceID& sourceDeviceID)
{
	std::shared_ptr<DeviceMessageDispatcher> dispatcher;

	//Attempt to remove from the remote dispatcher
	if (getSourceDeviceMessageDispatcher(sourceDeviceID, dispatcher)) {
		dispatcher->removeMessageHandler(localID);	//remove this handler to the remote dispatcher
	}

	handlers.remove(sourceDeviceID);
}


void DeviceMessageReceiver::removeAllHandlers()
{
	std::set<DeviceID> ids;
	handlers.getKeys(ids);

	for (auto& handerID : ids) {
		removeDeviceMessageHandler(handerID);
	}
}


bool DeviceMessageReceiver::getSourceDeviceMessageDispatcher(const DeviceID& sourceDeviceID, 
	std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
{
	bool success = false;
	std::shared_ptr<Device> device;

	if (deviceCollection->get(sourceDeviceID, device) && device != 0) {

		device->getMessageDispatcher(dispatcher);

		success = (dispatcher != 0);
	}

	return success;
}


void DeviceMessageReceiver::refreshListenerGroups(const DeviceID& sourceDeviceID, 
	const std::shared_ptr<DeviceMessageHandler>& handler)
{
	refreshListenerGroup(sourceDeviceID, refreshListeners, handler);
	refreshListenerGroup(sourceDeviceID, channelUpdateListeners, handler);
	refreshListenerGroup(sourceDeviceID, attributeUpdateListeners, handler);
	refreshListenerGroup(sourceDeviceID, engineSchedulerListeners, handler);
}


void DeviceMessageReceiver::removeListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID)
{
	bool success = false;

	switch (listenerID.type) {
	case DeviceMessageType::Refresh:
		success = removeListener(sourceDeviceID, listenerID, refreshListeners);
		break;
	case DeviceMessageType::ChannelUpdate:
		success = removeListener(sourceDeviceID, listenerID, channelUpdateListeners);
		break;
	case DeviceMessageType::AttributeUpdate:
		success = removeListener(sourceDeviceID, listenerID, attributeUpdateListeners);
		break;
	case DeviceMessageType::EngineScheduler:
		success = removeListener(sourceDeviceID, listenerID, engineSchedulerListeners);
		break;
	}
	
	if (success) {
		std::shared_ptr<DeviceMessageHandler> handler;

		//refresh any installed handlers with updated listener group
		if (handlers.get(sourceDeviceID, handler) && handler != 0) {
			refreshListenerGroups(sourceDeviceID, handler);
		}
	}
}

