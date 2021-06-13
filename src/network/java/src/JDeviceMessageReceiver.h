#ifndef STI_DEVICE_JDEVICEMESSAGERECEIVER_H
#define STI_DEVICE_JDEVICEMESSAGERECEIVER_H

#include "DeviceMessage.h"
#include "DeviceMessageListener.h"

#include <memory>


namespace STI
{
namespace Device
{

class DeviceMessageReceiver;
class DeviceID;
class DeviceMessageListenerID;

//Listeners

// typedef DeviceMessageListener<RefreshDeviceMessage> RefreshDeviceMessageListener;
// typedef DeviceMessageListener<ChannelUpdateDeviceMessage> ChannelUpdateDeviceMessageListener;

//class RefreshDeviceMessageListener : public DeviceMessageListener<RefreshDeviceMessage> {};
//class ChannelUpdateDeviceMessageListener : public DeviceMessageListener<ChannelUpdateDeviceMessage> {};
//...


//Java DeviceMessageReceiver wrapper
class JDeviceMessageReceiver
{
public:
	
	JDeviceMessageReceiver(std::shared_ptr<STI::Device::DeviceMessageReceiver>& receiver);
	~JDeviceMessageReceiver();

    // void addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
	// 	const std::shared_ptr<RefreshDeviceMessageListener>& listener);
    // void addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
	// 	const std::shared_ptr<ChannelUpdateDeviceMessageListener>& listener);

    //Need to write JRefreshDeviceMessageListener which wraps a new shared_ptr that gets made in the constructor. 
    void addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
		                 const std::shared_ptr<DeviceMessageListener<RefreshDeviceMessage>>& listener);
    void addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
		                 const std::shared_ptr<DeviceMessageListener<ChannelUpdateMessage>>& listener);
    void addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
		                 const std::shared_ptr<DeviceMessageListener<AttributeUpdateMessage>>& listener);
    void addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
		                 const std::shared_ptr<DeviceMessageListener<EngineSchedulerMessage>>& listener);
    void addListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID, 
		                 const std::shared_ptr<DeviceMessageListener<CollectionUpdateMessage>>& listener);
    //...

    void removeListener(const DeviceID& sourceDeviceID, const DeviceMessageListenerID& listenerID);

private:

    std::shared_ptr<DeviceMessageReceiver> deviceMessageReceiver;

    std::shared_ptr<STI::Device::DeviceMessageListener<RefreshDeviceMessage>> temp;
};

} //Device
} //STI

#endif
