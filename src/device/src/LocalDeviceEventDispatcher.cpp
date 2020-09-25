
#include "LocalDeviceEventDispatcher.h"
#include "DeviceEvent.h"
#include "LocalDeviceEventHandler.h"

#include <set>
#include <memory>

using STI::Device::LocalDeviceEventDispatcher;
using STI::Device::DeviceEvent;
using STI::Device::DeviceID;
using STI::Device::LocalDeviceEventHandler;
using STI::Device::DeviceEventHandler;

LocalDeviceEventDispatcher::LocalDeviceEventDispatcher() : eventQueue(this)
{
	eventQueue.start();
}

LocalDeviceEventDispatcher::~LocalDeviceEventDispatcher()
{
}

void LocalDeviceEventDispatcher::addEvent(const std::shared_ptr<DeviceEvent>& evt)
{
	eventQueue.addEvent(evt);
}

void LocalDeviceEventDispatcher::clearEvents()
{
	eventQueue.clearEvents();

	std::set<DeviceID> ids;
	handlers.getKeys(ids);

	std::shared_ptr<DeviceEventHandler> handler;

	for (auto& id : ids) {
		if (handlers.get(id, handler) && handler != 0) {
			handler->clearEvents();
		}
	}
}


void LocalDeviceEventDispatcher::addEventHandler(const DeviceID& targetID, const std::shared_ptr<DeviceEventHandler>& handler)
{
	handlers.remove(targetID);
	handlers.add(targetID, handler);
}

void LocalDeviceEventDispatcher::removeEventHandler(const DeviceID& targetID)
{
	handlers.remove(targetID);
}

bool LocalDeviceEventDispatcher::makeEventHandler(std::shared_ptr<DeviceEventHandler>& handler)
{
	handler = std::make_shared<LocalDeviceEventHandler>();

	return (handler != 0);
}


void LocalDeviceEventDispatcher::handleEvent(const std::shared_ptr<DeviceEvent>& evt)
{
	std::set<DeviceID> ids;
	handlers.getKeys(ids);

	std::shared_ptr<DeviceEventHandler> handler;

	for (auto& id : ids) {
		//filter so only handlers with listeners for this event type receive the event
		if (handlers.get(id, handler) && handler != 0 && handler->hasListeners(evt)) {
			handler->addEvent(evt);		//adds event to the handler's queue
		}
	}
}

