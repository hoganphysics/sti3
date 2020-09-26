
#include "LocalDeviceEventHandler.h"
#include "DeviceEventListenerGroup.h"
#include "DeviceEvent.h"

#include <set>

using STI::Device::LocalDeviceEventHandler;
using STI::Device::AbstractEventListenerGroup;
using STI::Device::DeviceEventType;
using STI::Device::DeviceEvent;


LocalDeviceEventHandler::LocalDeviceEventHandler() : eventQueue(this)
{
	eventQueue.start();
}


LocalDeviceEventHandler::~LocalDeviceEventHandler()
{
	eventQueue.stop();
	eventQueue.clearEvents();
}


void LocalDeviceEventHandler::addListenerGroup(const DeviceEventType& type, 
	std::shared_ptr<AbstractEventListenerGroup>& listenerGroup)
{
	if (eventListenerGroups.add(type, listenerGroup)) {
		listenersTypes[type] = listenerGroup->size();
	}
}


void LocalDeviceEventHandler::removeListenerGroup(const DeviceEventType& type)
{
	eventListenerGroups.remove(type);
	listenersTypes[type] = 0;
}


void LocalDeviceEventHandler::addEvent(const std::shared_ptr<DeviceEvent>& evt)
{
	if (evt == 0) return;

	eventQueue.addEvent(evt);
}


void LocalDeviceEventHandler::clearEvents()
{
	eventQueue.clearEvents();
}


bool LocalDeviceEventHandler::hasListeners(const std::shared_ptr<DeviceEvent>& evt)
{
	if (evt == 0) return false;

	auto it = listenersTypes.find(evt->getType());

	return (it != listenersTypes.end() && it->second > 0);
}


///List of event type that this handler responds to (based on which listeners are currently attached)
void LocalDeviceEventHandler::getListenerTypes(std::set<DeviceEventType>& types)
{
	types.clear();

	for (auto& t : listenersTypes) {
		if (t.second > 0) {			//number of listeners
			types.insert(t.first);	//listener type
		}
	}
}


void LocalDeviceEventHandler::handleEvent(const std::shared_ptr<DeviceEvent>& evt)
{
	if (evt == 0) return;

	std::shared_ptr<AbstractEventListenerGroup> eventListenerGroup;
	if (eventListenerGroups.get(evt->getType(), eventListenerGroup) && eventListenerGroup != 0) {
		eventListenerGroup->handleEvent(evt);
	}

}

