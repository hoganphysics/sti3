

#include "JDeviceMessageReceiver.h"
#include "DeviceMessageReceiver.h"
#include "DeviceMessageListener.h"
#include "DeviceID.h"

#include <memory>

using STI::Device::JDeviceMessageReceiver;
using STI::Device::DeviceID;
using STI::Device::DeviceMessageListenerID;
using STI::Device::DeviceMessageListener;


JDeviceMessageReceiver::JDeviceMessageReceiver(std::shared_ptr<STI::Device::DeviceMessageReceiver>& receiver)
: deviceMessageReceiver(receiver)
{
}

JDeviceMessageReceiver::~JDeviceMessageReceiver()
{
}

void JDeviceMessageReceiver::addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
		const std::shared_ptr<DeviceMessageListener<RefreshDeviceMessage>>& listener)
{
    if(deviceMessageReceiver != 0) {
        deviceMessageReceiver->addListener<STI::Device::RefreshDeviceMessage>(sourceDeviceID, listenerID, listener);
    }
}

void JDeviceMessageReceiver::addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
        const std::shared_ptr<DeviceMessageListener<ChannelUpdateMessage>>& listener)
{
    if(deviceMessageReceiver != 0) {
        deviceMessageReceiver->addListener<STI::Device::ChannelUpdateMessage>(sourceDeviceID, listenerID, listener);
    }
}

void JDeviceMessageReceiver::addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
                        const std::shared_ptr<DeviceMessageListener<STI::Device::AttributeUpdateMessage>>& listener)
{
    if(deviceMessageReceiver != 0) {
        deviceMessageReceiver->addListener<STI::Device::AttributeUpdateMessage>(sourceDeviceID, listenerID, listener);
    }
}

void JDeviceMessageReceiver::addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
                        const std::shared_ptr<DeviceMessageListener<STI::Device::EngineSchedulerMessage>>& listener)
{
    if(deviceMessageReceiver != 0) {
        deviceMessageReceiver->addListener<STI::Device::EngineSchedulerMessage>(sourceDeviceID, listenerID, listener);
    }
}

void JDeviceMessageReceiver::removeListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID)
{
    if(deviceMessageReceiver != 0) {
        deviceMessageReceiver->removeListener(sourceDeviceID, listenerID);
    }
}
