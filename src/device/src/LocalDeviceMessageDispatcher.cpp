
#include "LocalDeviceMessageDispatcher.h"
#include "LocalDeviceMessageHandler.h"
#include "DeviceMessage.h"

#include <set>
#include <memory>

using STI::Device::LocalDeviceMessageDispatcher;
using STI::Device::DeviceMessage;
using STI::Device::DeviceID;
using STI::Device::LocalDeviceMessageHandler;
using STI::Device::DeviceMessageHandler;


LocalDeviceMessageDispatcher::LocalDeviceMessageDispatcher() : eventQueue(this)
{
	eventQueue.start();
}

LocalDeviceMessageDispatcher::~LocalDeviceMessageDispatcher()
{
	eventQueue.stop();
	clearMessages();
	handlers.clear();
}


void LocalDeviceMessageDispatcher::addMessage(const std::shared_ptr<DeviceMessage>& mess)
{
	if (mess != 0) {
		eventQueue.addEvent(mess);		
	}
}


void LocalDeviceMessageDispatcher::clearMessages()
{
	eventQueue.clearEvents();

	std::set<DeviceID> ids;
	handlers.getKeys(ids);

	std::shared_ptr<DeviceMessageHandler> handler;

	for (auto& id : ids) {
		if (handlers.get(id, handler) && handler != 0) {
			handler->clearMessages();
		}
	}
}


void LocalDeviceMessageDispatcher::addMessageHandler(const DeviceID& targetID, const std::shared_ptr<DeviceMessageHandler>& handler)
{
	handlers.remove(targetID);
	handlers.add(targetID, handler);
}


void LocalDeviceMessageDispatcher::removeMessageHandler(const DeviceID& targetID)
{
	handlers.remove(targetID);
}


bool LocalDeviceMessageDispatcher::makeMessageHandler(std::shared_ptr<DeviceMessageHandler>& handler)
{
	handler = std::make_shared<LocalDeviceMessageHandler>();

	return (handler != 0);
}


void LocalDeviceMessageDispatcher::handleMessage(const std::shared_ptr<DeviceMessage>& mess)
{
	std::set<DeviceID> ids;
	handlers.getKeys(ids);

	std::shared_ptr<DeviceMessageHandler> handler;

	for (auto& id : ids) {
		//filter so only handlers with listeners for this event type receive the event
		if (handlers.get(id, handler) && handler != 0 && handler->hasListeners(mess)) {
			handler->addMessage(mess);		//adds event to the handler's queue
		}
	}
}

