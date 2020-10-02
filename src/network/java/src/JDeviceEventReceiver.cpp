

#include "JDeviceEventReceiver.h"
#include "DeviceEventReceiver.h"
#include "DeviceEventListener.h"
#include "DeviceID.h"

#include <memory>


using STI::Device::JDeviceEventReceiver;
using STI::Device::DeviceID;
using STI::Device::DeviceEventListenerID;


JDeviceEventReceiver::JDeviceEventReceiver(const std::shared_ptr<DeviceEventReceiver>& receiver)
: deviceEventReceiver(receiver)
{
}

JDeviceEventReceiver::~JDeviceEventReceiver()
{
}

void JDeviceEventReceiver::addListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID, 
		const std::shared_ptr<STI::Device::DeviceEventListener<RefreshDeviceEvent>>& listener)
{
    if(deviceEventReceiver != 0) {
        deviceEventReceiver->addListener<STI::Device::RefreshDeviceEvent>(sourceDeviceID, listenerID, listener);
    }
}

void JDeviceEventReceiver::addListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID, 
        const std::shared_ptr<STI::Device::DeviceEventListener<ChannelUpdateDeviceEvent>>& listener)
{
    if(deviceEventReceiver != 0) {
        deviceEventReceiver->addListener<STI::Device::ChannelUpdateDeviceEvent>(sourceDeviceID, listenerID, listener);
    }
}

void JDeviceEventReceiver::removeListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID)
{
    if(deviceEventReceiver != 0) {
        deviceEventReceiver->removeListener(sourceDeviceID, listenerID);
    }
}
