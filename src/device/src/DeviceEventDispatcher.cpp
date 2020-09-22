
#include "DeviceEventDispatcher.h"
#include "DeviceEvent.h"
#include "DeviceEventHandler.h"

#include <set>
#include <memory>

using STI::Device::DeviceEventDispatcher;
using STI::Device::DeviceEvent;
using STI::Device::DeviceID;
using STI::Device::DeviceEventHandler;

DeviceEventDispatcher::DeviceEventDispatcher()
{
	start();
}

DeviceEventDispatcher::~DeviceEventDispatcher()
{
}

void DeviceEventDispatcher::clearAll()
{
	clearEvents();

	std::set<DeviceID> ids;
	handlers.getKeys(ids);

	std::shared_ptr<DeviceEventHandler> handler;

	for (auto& id : ids) {
		if (handlers.get(id, handler) && handler != 0) {
			handler->clearEvents();
		}
	}
}


void DeviceEventDispatcher::addEventHandler(const DeviceID& id, const std::shared_ptr<DeviceEventHandler>& handler)
{
	handlers.add(id, handler);
}

void DeviceEventDispatcher::removeEventHandler(const DeviceID& id)
{
	handlers.remove(id);
}


void DeviceEventDispatcher::handleEvent(const std::shared_ptr<DeviceEvent>& evt)
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

