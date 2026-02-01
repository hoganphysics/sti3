#include "RemoteDeviceMessageHandler.h"
#include "NetworkConvert.h"
#include <sti/device/DeviceMessage.h>
#include "ORBManager.h"

#include "convert/Convert_DeviceMessage.h"

#include "generated/deviceNet.h"
#include "generated/orbTypes.h"

#include <set>

using STI::Network::RemoteDeviceMessageHandler;
using STI::Network::convert;
using STI::TNetwork::TReferenceHolder;
using STI::TNetwork::TDeviceMessageHandler;


RemoteDeviceMessageHandler::RemoteDeviceMessageHandler(::STI::TNetwork::TDeviceMessageHandler_var deviceHandler)
: TReferenceHolder<TDeviceMessageHandler>(deviceHandler), 
refreshIndicatorHolder(new STI::TNetwork::TRefreshIndicator_i())
{
	std::unique_lock<std::mutex> handlerLock(handlerMutex);

	//install refresh indicator on the remote resource this object is wrapping
	try {
		if (!isDisabled()) {
			getTRef()->setRefreshIndicator(refreshIndicatorHolder.getRefPtr());	//remote call
		}
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
	if (refreshIndicatorHolder.get() != nullptr 
		&& refreshIndicatorHolder.get()->checkThenReset()) {
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
			if (refreshIndicatorHolder.get() != nullptr) {
				refreshIndicatorHolder.get()->refresh();	//undo reset done by checkThenReset()
			}
		}
	}

	//Event filter based on whether listeners of a given type are present on the remote device
	return success && listenersTypes.count(mess->getType()) > 0;
}

