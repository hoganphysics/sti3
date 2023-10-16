#include "LocalDeviceMessageHandler.h"
#include <sti/device/DeviceMessageListenerGroup.h>
#include <sti/device/DeviceMessage.h>

#include <set>

using STI::Device::LocalDeviceMessageHandler;
using STI::Device::AbstractMessageListenerGroup;
using STI::Device::DeviceMessageType;
using STI::Device::DeviceMessage;


LocalDeviceMessageHandler::LocalDeviceMessageHandler() : eventQueue(this)
{
	eventQueue.start();
}

LocalDeviceMessageHandler::~LocalDeviceMessageHandler()
{
	eventQueue.stop();
	eventQueue.clearEvents();
}

void LocalDeviceMessageHandler::addListenerGroup(const DeviceMessageType& type, 
	std::shared_ptr<AbstractMessageListenerGroup>& listenerGroup)
{
	if (messageListenerGroups.add(type, listenerGroup)) {
		listenersTypes[type] = listenerGroup->size();
	}
}

void LocalDeviceMessageHandler::removeListenerGroup(const DeviceMessageType& type)
{
	messageListenerGroups.remove(type);
	listenersTypes[type] = 0;
}

void LocalDeviceMessageHandler::addMessage(const std::shared_ptr<DeviceMessage>& mess)
{
	if (mess == 0) return;

	eventQueue.addEvent(mess);
}

void LocalDeviceMessageHandler::clearMessages()
{
	eventQueue.clearEvents();
}

bool LocalDeviceMessageHandler::hasListeners(const std::shared_ptr<DeviceMessage>& mess)
{
	if (mess == 0) return false;

	auto it = listenersTypes.find(mess->getType());

	return (it != listenersTypes.end() && it->second > 0);
}

///List of event type that this handler responds to (based on which listeners are currently attached)
void LocalDeviceMessageHandler::getListenerTypes(std::set<DeviceMessageType>& types)
{
	types.clear();

	for (auto& t : listenersTypes) {
		if (t.second > 0) {			//number of listeners
			types.insert(t.first);	//listener type
		}
	}
}

void LocalDeviceMessageHandler::handleMessage(const std::shared_ptr<DeviceMessage>& mess)
{
	if (mess == 0) return;

	std::shared_ptr<AbstractMessageListenerGroup> messageListenerGroup;
	if (messageListenerGroups.get(mess->getType(), messageListenerGroup) && messageListenerGroup != 0) {
		messageListenerGroup->handleMessage(mess);
	}

}
