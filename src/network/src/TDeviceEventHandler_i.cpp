
#include "TDeviceEventHandler_i.h"

#include "ORBManager.h"
#include "NetworkConvert.h"

#include <set>

#include <iostream>

using STI::TNetwork::TDeviceEventHandler_i;
using STI::TNetwork::TDeviceEventTypeSeq;
using STI::Device::DeviceEvent;
using STI::Network::convert;



TDeviceEventHandler_i::TDeviceEventHandler_i(const std::shared_ptr<STI::Device::LocalDeviceEventHandler>& handler)
	: eventHandler(handler), tRefreshIndicatorInstalled(false)
{
}

TDeviceEventHandler_i::~TDeviceEventHandler_i()
{
	STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TDeviceEventHandler_i::addEvent(const ::STI::TNetwork::TAnyEvent& evt)
{
	std::shared_ptr<DeviceEvent> deviceEvent;

	if (eventHandler != 0 &&
		convert<::STI::TNetwork::TAnyEvent, std::shared_ptr<DeviceEvent>>(evt, deviceEvent)
		) 
	{
		eventHandler->addEvent(deviceEvent);
	}
}

void TDeviceEventHandler_i::clearEvents()
{
	if (eventHandler != 0) {
		eventHandler->clearEvents();
	}
}

TDeviceEventTypeSeq* TDeviceEventHandler_i::listenersTypes()
{
	std::set<STI::Device::DeviceEventType> types;

	if (eventHandler != 0) {
		eventHandler->getListenerTypes(types);
	}

	STI::TNetwork::TDeviceEventTypeSeq_var tEventTypes(new STI::TNetwork::TDeviceEventTypeSeq);
	convert<STI::Device::DeviceEventType, STI::TNetwork::TDeviceEventType>(types, (_CORBA_Unbounded_Sequence<STI::TNetwork::TDeviceEventType>&) tEventTypes);

	return tEventTypes._retn();
}

void TDeviceEventHandler_i::setRefreshIndicator(::STI::TNetwork::TRefreshIndicator_ptr refresher)
{
	tRefreshIndicator = STI::TNetwork::TRefreshIndicator::_duplicate(refresher);
//	tRefreshIndicator = refresher;

	tRefreshIndicatorInstalled = !CORBA::is_nil(tRefreshIndicator);
}

void TDeviceEventHandler_i::refresh()
{
	std::cout << "TDeviceEventHandler_i::refresh() nil? = " << CORBA::is_nil(tRefreshIndicator) << std::endl;

	try {
		if(tRefreshIndicatorInstalled && !CORBA::is_nil(tRefreshIndicator)) {
			tRefreshIndicator->refresh();	//remote call
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
