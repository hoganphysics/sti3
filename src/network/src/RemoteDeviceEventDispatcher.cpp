

#include "RemoteDeviceEventDispatcher.h"
#include "LocalDeviceEventHandler.h"
#include "NetworkDeviceEventHandlerWrapper.h"

#include "NetworkConvert.h"
#include "orbTypes.h"

using STI::Network::RemoteDeviceEventDispatcher;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;
using STI::Network::convert;


RemoteDeviceEventDispatcher::RemoteDeviceEventDispatcher(::STI::TNetwork::TDeviceEventDispatcher_ptr eventDispatcher)
	: tEventDispatcher(STI::TNetwork::TDeviceEventDispatcher::_duplicate(eventDispatcher))
{
}

RemoteDeviceEventDispatcher::~RemoteDeviceEventDispatcher()
{
}


//need to be able to cast DeviceEventHandler to something that can produce a TDeviceEventHandler_ptr.
//Need the EventReceiver to own wrapped handlers, which include handler servants.
void RemoteDeviceEventDispatcher::addEventHandler(const STI::Device::DeviceID& targetID, const std::shared_ptr<STI::Device::DeviceEventHandler>& handler)
{
	//need to upcast to NetworkDeviceEventHandlerWrapper and extract TDeviceEventHandler_ptr reference from servant
	STI::TNetwork::TDeviceEventHandler_ptr tEventHandler;
	//STI::TNetwork::TDeviceEventHandler_var tEventHandlervar;

	if (!NetworkDeviceEventHandlerWrapper::getTDeviceEventHandlerReference(handler, tEventHandler)) {
		return;
	}


//	STI::TNetwork::TDeviceEventHandler_var tEventHandlervar = tEventHandler;

	//tEventHandlervar.inout()

	try {
//		tEventDispatcher->removeEventHandler(convert<DeviceID, TDeviceID>(targetID));	//remote call

//		tEventDispatcher->addEventHandler(convert<DeviceID, TDeviceID>(targetID), tEventHandlervar);	//remote call
		tEventDispatcher->addEventHandler(convert<DeviceID, TDeviceID>(targetID), tEventHandler);	//remote call
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

void RemoteDeviceEventDispatcher::removeEventHandler(const STI::Device::DeviceID& targetID)
{
	try {
		tEventDispatcher->removeEventHandler(convert<DeviceID, TDeviceID>(targetID));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

bool RemoteDeviceEventDispatcher::makeEventHandler(std::shared_ptr<STI::Device::DeviceEventHandler>& handler)
{
	auto localHandler = std::make_shared<STI::Device::LocalDeviceEventHandler>();
	handler = std::make_shared<NetworkDeviceEventHandlerWrapper>(localHandler);

	return (handler != 0);
}

void RemoteDeviceEventDispatcher::addEvent(const std::shared_ptr<STI::Device::DeviceEvent>& evt)
{
}

void RemoteDeviceEventDispatcher::clearEvents()
{
}

