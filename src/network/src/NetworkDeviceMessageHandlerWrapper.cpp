
#include "NetworkDeviceMessageHandlerWrapper.h"
#include "LocalDeviceMessageHandler.h"
#include "ORBManager.h"

#include <memory>


using STI::Network::NetworkDeviceMessageHandlerWrapper;
using STI::Device::LocalDeviceMessageHandler;
using STI::Device::DeviceMessageType;
using STI::Device::DeviceMessage;


NetworkDeviceMessageHandlerWrapper::NetworkDeviceMessageHandlerWrapper(const std::shared_ptr<LocalDeviceMessageHandler>& localHandler)
: localMessageHandler(localHandler), 
messageHandlerServantHolder(new STI::TNetwork::TDeviceMessageHandler_i(localHandler))
{
	// STI::Network::ORBManager::ORBManager::activateServant(messageHandlerServant);
}

NetworkDeviceMessageHandlerWrapper::~NetworkDeviceMessageHandlerWrapper()
{
}

void NetworkDeviceMessageHandlerWrapper::disable()
{
	auto servant = messageHandlerServantHolder.get();
	if (servant == nullptr) return;

	servant->disableRefreshIndicator();
	// messageHandlerServantHolder.get()->disableRefreshIndicator();
	// messageHandlerServant.disableRefreshIndicator();
}

void NetworkDeviceMessageHandlerWrapper::addListenerGroup(const DeviceMessageType& type, 
	std::shared_ptr<STI::Device::AbstractMessageListenerGroup>& listenerGroup)
{
	if (localMessageHandler != 0) {
		localMessageHandler->addListenerGroup(type, listenerGroup);
		
		auto servant = messageHandlerServantHolder.get();
		if (servant == nullptr) return;
		
		servant->refresh();		//Raises flag on RemoteDeviceMessageHandler, 
												//indicating that this Handler has changed and need to be refreshed.
	}
}

void NetworkDeviceMessageHandlerWrapper::removeListenerGroup(const DeviceMessageType& type)
{
	if (localMessageHandler != 0) {
		localMessageHandler->removeListenerGroup(type);

		auto servant = messageHandlerServantHolder.get();
		if (servant == nullptr) return;
		
		servant->refresh();		//Raises flag on RemoteDeviceMessageHandler
	}
}


void NetworkDeviceMessageHandlerWrapper::addMessage(const std::shared_ptr<DeviceMessage>& mess)
{
	if (localMessageHandler != 0) {
		localMessageHandler->addMessage(mess);
	}
}

void NetworkDeviceMessageHandlerWrapper::clearMessages()
{
	if (localMessageHandler != 0) {
		localMessageHandler->clearMessages();
	}
}


bool NetworkDeviceMessageHandlerWrapper::hasListeners(const std::shared_ptr<DeviceMessage>& mess)
{
	if (localMessageHandler != 0) {
		return localMessageHandler->hasListeners(mess);
	}
	return false;
}

