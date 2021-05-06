
#include "TDeviceMessageDispatcher_i.h"
#include "NetworkConvert.h"
#include "RemoteDeviceMessageHandler.h"
#include "Device.h"
#include "ORBManager.h"

using STI::TNetwork::TDeviceMessageDispatcher_i;
using ::STI::TNetwork::TDeviceID;
using ::STI::TNetwork::TDeviceMessageHandler_ptr;
using STI::Network::RemoteDeviceMessageHandler;
using ::STI::TNetwork::TDeviceID;
using STI::Device::DeviceID;
using STI::Network::convert;


TDeviceMessageDispatcher_i::TDeviceMessageDispatcher_i(const std::shared_ptr<STI::Device::Device>& device)
{
	std::shared_ptr<STI::Device::DeviceMessageDispatcher> dispatcher;
	device->getMessageDispatcher(dispatcher);

	messageDispatcher = dispatcher;
}

TDeviceMessageDispatcher_i::~TDeviceMessageDispatcher_i()
{
	STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TDeviceMessageDispatcher_i::addMessageHandler(const TDeviceID& targetID, TDeviceMessageHandler_ptr handler)
{
	if (messageDispatcher != 0 && !CORBA::is_nil(handler)) {

		//wrap the received TDevice reference in RemoteDevice
		std::shared_ptr<RemoteDeviceMessageHandler> remoteHandler = std::make_shared<RemoteDeviceMessageHandler>(handler);

		if (remoteHandler != 0) {
			messageDispatcher->addMessageHandler(convert<TDeviceID, DeviceID>(targetID), remoteHandler);
		}
	}
}

void TDeviceMessageDispatcher_i::removeMessageHandler(const TDeviceID& targetID)
{
	if (messageDispatcher != 0) {

		messageDispatcher->removeMessageHandler(convert<TDeviceID, DeviceID>(targetID));
	}
}

::CORBA::Boolean TDeviceMessageDispatcher_i::ping()
{
	return true;
}

