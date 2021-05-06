#ifndef STI_DEVICE_DEVICEMESSAGELISTENERFORWARDER_H
#define STI_DEVICE_DEVICEMESSAGELISTENERFORWARDER_H

#include "LocalDevice.h"
#include "DeviceID.h"
#include "DeviceMessageListener.h"
#include "DeviceMessageReceiver.h"

#include <memory>

namespace STI
{
namespace Device
{

class LocalDevice;

class DeviceMessageListenerForwarder
{

public:
	
	DeviceMessageListenerForwarder(LocalDevice* localDevice) : localDevice(localDevice) {}

	template<typename T>
	void addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
		const std::shared_ptr<DeviceMessageListener<T>>& listener)
	{
		if (localDevice != 0 && localDevice->deviceMessageReceiver != 0) {
			localDevice->deviceMessageReceiver->addListener(sourceDeviceID, listenerID, listener);
		}
	}
	
	void removeListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID)
	{
		if (localDevice != 0 && localDevice->deviceMessageReceiver != 0) {
			localDevice->deviceMessageReceiver->removeListener(sourceDeviceID, listenerID);
		}
	}


private:

	LocalDevice* localDevice;
};



} //Device
} //STI


#endif
