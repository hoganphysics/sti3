#include "TDeviceMessageHandler_i.h"

#include "ORBManager.h"
#include "NetworkConvert.h"
#include "Convert_DeviceMessage.h"

#include <set>

using STI::TNetwork::TDeviceMessageHandler_i;
using STI::TNetwork::TDeviceMessageTypeSeq;
using STI::Device::DeviceMessage;
using STI::Network::convert;
using STI::TNetwork::TReferenceHolder;
using STI::TNetwork::TRefreshIndicator;


TDeviceMessageHandler_i::TDeviceMessageHandler_i(const std::shared_ptr<STI::Device::LocalDeviceMessageHandler>& handler)
	: messageHandler(handler), tRefreshIndicatorInstalled(false)
{
}

TDeviceMessageHandler_i::~TDeviceMessageHandler_i()
{
	disableRefreshIndicator();

	STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TDeviceMessageHandler_i::disableRefreshIndicator()
{
	if (tRefreshIndicatorHolder != 0) {
		tRefreshIndicatorHolder->disable();
	}
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
	std::unique_lock<std::mutex> handlerLock(refreshMutex);

	tRefreshIndicatorHolder = std::make_unique<TReferenceHolder<TRefreshIndicator>>(refresher, refreshMutex);

	tRefreshIndicatorInstalled = tRefreshIndicatorHolder !=0 && !tRefreshIndicatorHolder->isDisabled();
}

void TDeviceMessageHandler_i::refresh()
{
	std::unique_lock<std::mutex> handlerLock(refreshMutex);

	try {
		if(tRefreshIndicatorInstalled && tRefreshIndicatorHolder !=0 && !tRefreshIndicatorHolder->isDisabled()) {
			tRefreshIndicatorHolder->getTRef()->refresh();	//remote call
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
