
#include "DeviceEventReceiver.h"
#include "DeviceID.h"
#include "Device.h"
#include "DeviceEventDispatcher.h"

#include <string>

using STI::Device::DeviceEventReceiver;
using STI::Device::DeviceEventDispatcher;
using STI::Device::DeviceID;
using STI::Device::Device;
using STI::Utils::LocalCollection;

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

void DeviceEventReceiver::addHandlerToDispatcher(const DeviceID& sourceDeviceID)
{
	//Add the DeviceEventHandler to the remote DeviceEventDispatcher
	std::shared_ptr<Device> device;
	std::shared_ptr<DeviceEventDispatcher> dispatcher;
	std::shared_ptr<DeviceEventHandler> handler;

	if (deviceCollection->get(sourceDeviceID, device) && device != 0) {

		device->getEventDispatcher(dispatcher);

		if (dispatcher != 0 && handlers.get(sourceDeviceID, handler) && handler != 0) {
			dispatcher->addEventHandler(localID, handler);		//add this handler to the remote dispatcher under the local ID
		}
	}
}

void DeviceEventReceiver::makeNewHandler(const DeviceID& sourceDeviceID)
{
	if (!handlers.contains(sourceDeviceID)) {
		std::shared_ptr<DeviceEventHandler> newHandler = std::make_shared<DeviceEventHandler>();

		if (newHandler != 0) {
			handlers.add(sourceDeviceID, newHandler);
		}
	}
}

void DeviceEventReceiver::addDeviceEventHandler(const DeviceID& sourceDeviceID)
{
	makeNewHandler(sourceDeviceID);

	addHandlerToDispatcher(sourceDeviceID);
}

void DeviceEventReceiver::removeDeviceEventHandler(const DeviceID& sourceDeviceID)
{
	handlers.remove(sourceDeviceID);

	//Remove the DeviceEventHandler from the remote DeviceEventDispatcher
	std::shared_ptr<Device> device;
	std::shared_ptr<DeviceEventDispatcher> dispatcher;

	if (deviceCollection->get(sourceDeviceID, device) && device != 0) {

		device->getEventDispatcher(dispatcher);

		if (dispatcher != 0) {
			dispatcher->removeEventHandler(localID);
		}
	}
}


void DeviceEventReceiver::removeListener(const DeviceID& sourceDeviceID, const std::string& listenerID)
{
	std::shared_ptr<DeviceEventHandler> handler;

	if (handlers.get(sourceDeviceID, handler) && handler != 0) {
		handler->removeListener(listenerID);

		addHandlerToDispatcher(sourceDeviceID);		//refresh, overwriting old handler
	}
}

