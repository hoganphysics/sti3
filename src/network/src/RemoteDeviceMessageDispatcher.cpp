

#include "RemoteDeviceMessageDispatcher.h"
#include "LocalDeviceMessageHandler.h"
#include "NetworkDeviceMessageHandlerWrapper.h"

#include "NetworkConvert.h"
#include "orbTypes.h"

using STI::Network::RemoteDeviceMessageDispatcher;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;
using STI::Network::convert;


RemoteDeviceMessageDispatcher::RemoteDeviceMessageDispatcher(::STI::TNetwork::TDeviceMessageDispatcher_ptr messageDispatcher)
: STI::TNetwork::TReferenceHolder<STI::TNetwork::TDeviceMessageDispatcher>(messageDispatcher, dispatcherMutex)
{
}

RemoteDeviceMessageDispatcher::~RemoteDeviceMessageDispatcher()
{
}


//need to be able to cast DeviceEventHandler to something that can produce a TDeviceEventHandler_ptr.
//Need the EventReceiver to own wrapped handlers, which include handler servants.
void RemoteDeviceMessageDispatcher::addMessageHandler(const STI::Device::DeviceID& targetID, const std::shared_ptr<STI::Device::DeviceMessageHandler>& handler)
{
	std::unique_lock<std::mutex> dispatcherLock(dispatcherMutex);

	if (isDisabled()) return;

	//need to upcast to NetworkDeviceMessageHandlerWrapper and extract TDeviceMessageHandler_ptr reference from servant
	STI::TNetwork::TDeviceMessageHandler_ptr tMessageHandler;

	if (!NetworkDeviceMessageHandlerWrapper::getTDeviceMessageHandlerReference(handler, tMessageHandler)) {
		return;
	}

	try {
		getTRef()->addMessageHandler(convert<DeviceID, TDeviceID>(targetID), tMessageHandler);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

}

void RemoteDeviceMessageDispatcher::removeMessageHandler(const STI::Device::DeviceID& targetID)
{
	std::unique_lock<std::mutex> dispatcherLock(dispatcherMutex);

	if (isDisabled()) return;

	try {
		getTRef()->removeMessageHandler(convert<DeviceID, TDeviceID>(targetID));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

bool RemoteDeviceMessageDispatcher::makeMessageHandler(std::shared_ptr<STI::Device::DeviceMessageHandler>& handler)
{
	auto localHandler = std::make_shared<STI::Device::LocalDeviceMessageHandler>();
	handler = std::make_shared<NetworkDeviceMessageHandlerWrapper>(localHandler);

	return (handler != 0);
}

void RemoteDeviceMessageDispatcher::addMessage(const std::shared_ptr<STI::Device::DeviceMessage>& mess)
{
}

void RemoteDeviceMessageDispatcher::clearMessages()
{
}

bool RemoteDeviceMessageDispatcher::ping() const
{
	std::unique_lock<std::mutex> dispatcherLock(dispatcherMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->ping();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}

