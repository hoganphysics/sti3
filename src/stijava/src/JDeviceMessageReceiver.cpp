#include "JDeviceMessageReceiver.h"
#include <sti/device/DeviceMessageReceiver.h>
#include <sti/device/DeviceMessageListener.h>
#include <sti/device/DeviceID.h>
#include "JEngineJobUpdateDeviceMessage.h"
#include "JEngineJobUpdateDeviceMessageListener.h"

#include <memory>


using STI::Device::JDeviceMessageReceiver;
using STI::Device::DeviceID;
using STI::Device::DeviceMessageListenerID;
using STI::Device::DeviceMessageListener;
using STI::Device::JEngineJobUpdateDeviceMessageListener;


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

void JDeviceMessageReceiver::addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
		                 const std::shared_ptr<DeviceMessageListener<CollectionUpdateMessage>>& listener)
{
    if(deviceMessageReceiver != 0) {
        deviceMessageReceiver->addListener<STI::Device::CollectionUpdateMessage>(sourceDeviceID, listenerID, listener);

        // auto listenerIDtmp = listenerID;
        // listenerIDtmp.name = listenerID.name + "tmp";

        // deviceMessageReceiver->addListener<STI::Device::CollectionUpdateMessage>(sourceDeviceID, listenerIDtmp,
        // [](auto message) {
        //     std::cout << "*** Java CollectionUpdateMessage ***" << std::endl;
        // });
    }
}

void JDeviceMessageReceiver::addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
                        const std::shared_ptr<DeviceMessageListener<EngineStateMessage>>& listener)
{
    if(deviceMessageReceiver != 0) {
        deviceMessageReceiver->addListener<STI::Device::EngineStateMessage>(sourceDeviceID, listenerID, listener);
    }
}

void JDeviceMessageReceiver::addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
                        const std::shared_ptr<JEngineJobUpdateDeviceMessageListener>& jListener)
{
    auto listener = std::static_pointer_cast<DeviceMessageListener<EngineJobUpdateDeviceMessage>>(jListener);

    if(deviceMessageReceiver != 0) {
       deviceMessageReceiver->addListener<STI::Device::EngineJobUpdateDeviceMessage>(sourceDeviceID, listenerID, listener);

    //    deviceMessageReceiver->addListener<STI::Device::EngineJobUpdateDeviceMessage>(sourceDeviceID, listenerID, 
    //         [jListener](auto message) {
    //             std::cout << "JDeviceMessageReceiver handleMessage" << std::endl;
    //             jListener->handleMessage(message);
    //         }
    //     );
    }
}

void JDeviceMessageReceiver::removeListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID)
{
    if(deviceMessageReceiver != 0) {
        deviceMessageReceiver->removeListener(sourceDeviceID, listenerID);
    }
}
