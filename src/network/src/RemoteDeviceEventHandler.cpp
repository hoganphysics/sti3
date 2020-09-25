
#include "RemoteDeviceEventHandler.h"
#include "NetworkConvert.h"

#include "deviceNet.h"
#include "orbTypes.h"

#include <set>

using STI::Network::RemoteDeviceEventHandler;
using STI::Network::convert;

RemoteDeviceEventHandler::RemoteDeviceEventHandler(::STI::TNetwork::TDeviceEventHandler_ptr deviceHandler)
	: tDeviceHandler(STI::TNetwork::TDeviceEventHandler::_duplicate(deviceHandler))
//	: tDeviceHandler(deviceHandler)
{
	//install refresh indicator on the remote resource this object is wrapping
	try {

		//STI::TNetwork::TRefreshIndicator_var refreshIndicatorVar = refreshIndicator._this();
		//tDeviceHandler->setRefreshIndicator(refreshIndicatorVar);	//remote call
		
		tDeviceHandler->setRefreshIndicator(refreshIndicator._this());	//remote call

	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}
RemoteDeviceEventHandler::~RemoteDeviceEventHandler()
{
}

void RemoteDeviceEventHandler::addListenerGroup(const STI::Device::DeviceEventType& type, std::shared_ptr<STI::Device::AbstractEventListenerGroup>& listenerGroup)
{
	//not allowed; listeners can only be added locally
}

void RemoteDeviceEventHandler::removeListenerGroup(const STI::Device::DeviceEventType& type)
{
	//not allowed; listeners can only be added locally
}

void RemoteDeviceEventHandler::addEvent(const std::shared_ptr<STI::Device::DeviceEvent>& evt)
{
	if (!hasListeners(evt)) {
		return;
	}

	STI::TNetwork::TAnyEvent tAnyEvent;

	//CORBA::Any anyEvent;

	if (!convert<std::shared_ptr<STI::Device::DeviceEvent>, STI::TNetwork::TAnyEvent>(evt, tAnyEvent)) {
		return;
	}

	try {
		tDeviceHandler->addEvent(tAnyEvent);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteDeviceEventHandler::clearEvents()
{

	try {
		tDeviceHandler->clearEvents();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

bool RemoteDeviceEventHandler::hasListeners(const std::shared_ptr<STI::Device::DeviceEvent>& evt)
{
	using ::STI::TNetwork::TDeviceEventType;
	using STI::Device::DeviceEventType;

	std::unique_lock<std::mutex> writelock(listenersMutex);		//avoids reentrant calls

	//A remote call is only made to refresh the event filter list if the remote resource refreshed.
	if (refreshIndicator.checkThenReset()) {
		//a refresh occurred on the remote resource; we need to refresh

		auto tListenersTypes = tDeviceHandler->listenersTypes();

		if (tListenersTypes != 0) {
			
			listenersTypes.clear();

			convert<TDeviceEventType, DeviceEventType>(
				(const _CORBA_Unbounded_Sequence<TDeviceEventType>&) *tListenersTypes, listenersTypes);
		}
	}

	//should cache this...
	//std::set<STI::Device::DeviceEventType> listenersTypes;
	
	//Event filter based on whether listeners of a given type are present on the remote device
	return listenersTypes.count(evt->getType()) > 0;
	
//	return false;
}


