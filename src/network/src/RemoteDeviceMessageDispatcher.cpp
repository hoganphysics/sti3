

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
	: tMessageDispatcher(STI::TNetwork::TDeviceMessageDispatcher::_duplicate(messageDispatcher))
{
}

RemoteDeviceMessageDispatcher::~RemoteDeviceMessageDispatcher()
{
}


//need to be able to cast DeviceEventHandler to something that can produce a TDeviceEventHandler_ptr.
//Need the EventReceiver to own wrapped handlers, which include handler servants.
void RemoteDeviceMessageDispatcher::addMessageHandler(const STI::Device::DeviceID& targetID, const std::shared_ptr<STI::Device::DeviceMessageHandler>& handler)
{
	//need to upcast to NetworkDeviceMessageHandlerWrapper and extract TDeviceMessageHandler_ptr reference from servant
	STI::TNetwork::TDeviceMessageHandler_ptr tMessageHandler;
	//STI::TNetwork::TDeviceEventHandler_var tEventHandlervar;

	if (!NetworkDeviceMessageHandlerWrapper::getTDeviceMessageHandlerReference(handler, tMessageHandler)) {
		return;
	}


//	STI::TNetwork::TDeviceEventHandler_var tEventHandlervar = tEventHandler;

	//tEventHandlervar.inout()

	try {
//		tEventDispatcher->removeEventHandler(convert<DeviceID, TDeviceID>(targetID));	//remote call

//		tEventDispatcher->addEventHandler(convert<DeviceID, TDeviceID>(targetID), tEventHandlervar);	//remote call
		tMessageDispatcher->addMessageHandler(convert<DeviceID, TDeviceID>(targetID), tMessageHandler);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

//	CORBA::release(tEventHandler);

}

void RemoteDeviceMessageDispatcher::removeMessageHandler(const STI::Device::DeviceID& targetID)
{
	try {
		tMessageDispatcher->removeMessageHandler(convert<DeviceID, TDeviceID>(targetID));	//remote call
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

