

#include "NetworkDeviceEventHandlerWrapper.h"
#include "LocalDeviceEventHandler.h"

#include <memory>

using STI::Network::NetworkDeviceEventHandlerWrapper;
using STI::Device::LocalDeviceEventHandler;
using STI::Device::DeviceEventType;
using STI::Device::DeviceEvent;

NetworkDeviceEventHandlerWrapper::NetworkDeviceEventHandlerWrapper(const std::shared_ptr<LocalDeviceEventHandler>& localHandler)
	: localEventHandler(localHandler), eventHandlerServant(localHandler)
{
}

NetworkDeviceEventHandlerWrapper::~NetworkDeviceEventHandlerWrapper()
{
}


void NetworkDeviceEventHandlerWrapper::addListenerGroup(const DeviceEventType& type, 
	std::shared_ptr<STI::Device::AbstractEventListenerGroup>& listenerGroup)
{
	if (localEventHandler != 0) {
		localEventHandler->addListenerGroup(type, listenerGroup);
		eventHandlerServant.refresh();		//Raises flag on RemoteDeviceEventHandler, 
											//indicating that this Handler has changed and need to be refreshed.
	}
}

void NetworkDeviceEventHandlerWrapper::removeListenerGroup(const DeviceEventType& type)
{
	if (localEventHandler != 0) {
		localEventHandler->removeListenerGroup(type);
		eventHandlerServant.refresh();		//Raises flag on RemoteDeviceEventHandler
	}
}


void NetworkDeviceEventHandlerWrapper::addEvent(const std::shared_ptr<DeviceEvent>& evt)
{
	if (localEventHandler != 0) {
		localEventHandler->addEvent(evt);
	}
}

void NetworkDeviceEventHandlerWrapper::clearEvents()
{
	if (localEventHandler != 0) {
		localEventHandler->clearEvents();
	}
}


bool NetworkDeviceEventHandlerWrapper::hasListeners(const std::shared_ptr<DeviceEvent>& evt)
{
	if (localEventHandler != 0) {
		return localEventHandler->hasListeners(evt);
	}
	return false;
}

