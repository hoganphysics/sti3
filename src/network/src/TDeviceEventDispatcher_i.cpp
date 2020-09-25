
#include "TDeviceEventDispatcher_i.h"
#include "NetworkConvert.h"
#include "RemoteDeviceEventHandler.h"
#include "Device.h"
#include "ORBManager.h"

using STI::TNetwork::TDeviceEventDispatcher_i;
using ::STI::TNetwork::TDeviceID;
using ::STI::TNetwork::TDeviceEventHandler_ptr;
using STI::Network::RemoteDeviceEventHandler;
using ::STI::TNetwork::TDeviceID;
using STI::Device::DeviceID;
using STI::Network::convert;


TDeviceEventDispatcher_i::TDeviceEventDispatcher_i(const std::shared_ptr<STI::Device::Device>& device)
{
	std::shared_ptr<STI::Device::DeviceEventDispatcher> dispatcher;
	device->getEventDispatcher(dispatcher);

	eventDispatcher = dispatcher;
}

TDeviceEventDispatcher_i::~TDeviceEventDispatcher_i()
{
	STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TDeviceEventDispatcher_i::addEventHandler(const TDeviceID& targetID, TDeviceEventHandler_ptr handler)
{
	if (eventDispatcher != 0 && !CORBA::is_nil(handler)) {

		//wrap the received TDevice reference in RemoteDevice
		std::shared_ptr<RemoteDeviceEventHandler> remoteHandler = std::make_shared<RemoteDeviceEventHandler>(handler);

		if (remoteHandler != 0) {
			eventDispatcher->addEventHandler(convert<TDeviceID, DeviceID>(targetID), remoteHandler);
		}
	}
}

void TDeviceEventDispatcher_i::removeEventHandler(const TDeviceID& targetID)
{
	if (eventDispatcher != 0) {

		eventDispatcher->removeEventHandler(convert<TDeviceID, DeviceID>(targetID));
	}
}

