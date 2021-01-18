
#include "TDeviceMessageHandler_i.h"

#include "ORBManager.h"
#include "NetworkConvert.h"

#include <set>

using STI::TNetwork::TDeviceMessageHandler_i;
using STI::TNetwork::TDeviceMessageTypeSeq;
using STI::Device::DeviceMessage;
using STI::Network::convert;


TDeviceMessageHandler_i::TDeviceMessageHandler_i(const std::shared_ptr<STI::Device::LocalDeviceMessageHandler>& handler)
	: messageHandler(handler), tRefreshIndicatorInstalled(false)
{
}

TDeviceMessageHandler_i::~TDeviceMessageHandler_i()
{
	STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TDeviceMessageHandler_i::addMessage(const ::STI::TNetwork::TAnyMessage& mess)
{
	std::shared_ptr<DeviceMessage> deviceMessage;

	if (messageHandler != 0 &&
		convert<::STI::TNetwork::TAnyMessage, std::shared_ptr<DeviceMessage>>(mess, deviceMessage)
		) 
	{
		messageHandler->addMessage(deviceMessage);
	}
}

void TDeviceMessageHandler_i::clearMessages()
{
	if (messageHandler != 0) {
		messageHandler->clearMessages();
	}
}

TDeviceMessageTypeSeq* TDeviceMessageHandler_i::listenersTypes()
{
	std::set<STI::Device::DeviceMessageType> types;

	if (messageHandler != 0) {
		messageHandler->getListenerTypes(types);
	}

	STI::TNetwork::TDeviceMessageTypeSeq_var tMessageTypes(new STI::TNetwork::TDeviceMessageTypeSeq);
	convert<STI::Device::DeviceMessageType, STI::TNetwork::TDeviceMessageType>(types, 
					(_CORBA_Unbounded_Sequence<STI::TNetwork::TDeviceMessageType>&) tMessageTypes);

	return tMessageTypes._retn();
}

void TDeviceMessageHandler_i::setRefreshIndicator(::STI::TNetwork::TRefreshIndicator_ptr refresher)
{
	tRefreshIndicator = STI::TNetwork::TRefreshIndicator::_duplicate(refresher);

	tRefreshIndicatorInstalled = !CORBA::is_nil(tRefreshIndicator);
}

void TDeviceMessageHandler_i::refresh()
{
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
