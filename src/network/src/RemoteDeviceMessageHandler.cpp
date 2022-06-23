
#include "RemoteDeviceMessageHandler.h"
#include "NetworkConvert.h"
#include <sti/device/DeviceMessage.h>

#include "Convert_DeviceMessage.h"

#include "deviceNet.h"
#include "orbTypes.h"

#include <set>

using STI::Network::RemoteDeviceMessageHandler;
using STI::Network::convert;
using STI::TNetwork::TReferenceHolder;
using STI::TNetwork::TDeviceMessageHandler;

RemoteDeviceMessageHandler::RemoteDeviceMessageHandler(::STI::TNetwork::TDeviceMessageHandler_ptr deviceHandler)
: TReferenceHolder<TDeviceMessageHandler>(deviceHandler, handlerMutex)
//	: tDeviceHandler(STI::TNetwork::TDeviceMessageHandler::_duplicate(deviceHandler))
{
	std::unique_lock<std::mutex> handlerLock(handlerMutex);

//	STI::TNetwork::TDeviceEventHandler
//	CORBA::remove_ref(deviceHandler);
	//install refresh indicator on the remote resource this object is wrapping
	try {

		//STI::TNetwork::TRefreshIndicator_var refreshIndicatorVar = refreshIndicator._this();
		//getTRef()->setRefreshIndicator(refreshIndicatorVar);	//remote call
		
		getTRef()->setRefreshIndicator(refreshIndicator._this());	//remote call

	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

RemoteDeviceMessageHandler::~RemoteDeviceMessageHandler()
{
	TReferenceHolder<TDeviceMessageHandler>::disable();
}

void RemoteDeviceMessageHandler::disable()
{
	TReferenceHolder<TDeviceMessageHandler>::disable();
}

void RemoteDeviceMessageHandler::addListenerGroup(const STI::Device::DeviceMessageType& type, 
								std::shared_ptr<STI::Device::AbstractMessageListenerGroup>& listenerGroup)
{
	//not allowed; listeners can only be added locally
}

void RemoteDeviceMessageHandler::removeListenerGroup(const STI::Device::DeviceMessageType& type)
{
	//not allowed; listeners can only be added locally
}

void RemoteDeviceMessageHandler::addMessage(const std::shared_ptr<STI::Device::DeviceMessage>& mess)
{
	if (!hasListeners(mess)) {
		return;
	}

	std::unique_lock<std::mutex> handlerLock(handlerMutex);

	if (isDisabled()) return;

	STI::TNetwork::TAnyMessage tAnyMessage;

	if (!convert<std::shared_ptr<STI::Device::DeviceMessage>, STI::TNetwork::TAnyMessage>(mess, tAnyMessage)) {
		return;
	}

	try {
		getTRef()->addMessage(tAnyMessage);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteDeviceMessageHandler::clearMessages()
{
	std::unique_lock<std::mutex> handlerLock(handlerMutex);

	if (isDisabled()) return;

	try {
		getTRef()->clearMessages();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

bool RemoteDeviceMessageHandler::hasListeners(const std::shared_ptr<STI::Device::DeviceMessage>& mess)
{
	using ::STI::TNetwork::TDeviceMessageTypeSeq_var;
	using ::STI::TNetwork::TDeviceMessageType;
	using STI::Device::DeviceMessageType;

	std::unique_lock<std::mutex> handlerLock(handlerMutex);		//avoids reentrant calls

	bool success = true;

	//A remote call is only made to refresh the event filter list if the remote resource refreshed.
	if (refreshIndicator.checkThenReset()) {
		//a refresh occurred on the remote resource; we need to refresh
		
		success = false;
		TDeviceMessageTypeSeq_var tListenersTypes;

		try {
			if (!isDisabled()) {
				tListenersTypes = getTRef()->listenersTypes();	//remote call
				success = true;
			}
		}
		catch (CORBA::TRANSIENT&) {
		}
		catch (CORBA::SystemException&) {
		}
		catch (CORBA::Exception&)
		{
		}

		if (success) {
			
			listenersTypes.clear();

			convert<TDeviceMessageType, DeviceMessageType>(
				(const _CORBA_Unbounded_Sequence<TDeviceMessageType>&) tListenersTypes, listenersTypes);
		}
		else {
			//A refresh occured, but remote call failed, so update was not completed.
			refreshIndicator.refresh();	//undo reset done by checkThenReset()
		}
	}

	//Event filter based on whether listeners of a given type are present on the remote device
	return success && listenersTypes.count(mess->getType()) > 0;
	
}


