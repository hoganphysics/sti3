#include "TDeviceMessageDispatcher_i.h"

#include <sti/device/Device.h>

#include "NetworkConvert.h"
#include "RemoteDeviceMessageHandler.h"
// #include "ORBManager.h"

using STI::TNetwork::TDeviceMessageDispatcher_i;
using ::STI::TNetwork::TDeviceID;
using ::STI::TNetwork::TDeviceMessageHandler_ptr;
using STI::Network::RemoteDeviceMessageHandler;
using ::STI::TNetwork::TDeviceID;
using STI::Device::DeviceID;
using STI::Network::convert;


TDeviceMessageDispatcher_i::TDeviceMessageDispatcher_i(const std::shared_ptr<STI::Device::Device>& device)
{
	if (device != 0) {
        device->getMessageDispatcher(messageDispatcher);        
    }
}

TDeviceMessageDispatcher_i::~TDeviceMessageDispatcher_i()
{
	// STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TDeviceMessageDispatcher_i::addMessageHandler(const TDeviceID& targetID, TDeviceMessageHandler_ptr handler)
{
	if (messageDispatcher != 0 && !CORBA::is_nil(handler)) {

		//wrap the received TDevice reference in RemoteDevice
		STI::TNetwork::TDeviceMessageHandler_var handler_var = STI::TNetwork::TDeviceMessageHandler::_duplicate(handler);
		std::shared_ptr<RemoteDeviceMessageHandler> remoteHandler = std::make_shared<RemoteDeviceMessageHandler>(handler_var);

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

