#ifndef STI_DEVICE_JDEVICEEVENTRECEIVER_H
#define STI_DEVICE_JDEVICEEVENTRECEIVER_H

#include "DeviceEvent.h"
#include "DeviceEventListener.h"

#include <memory>


namespace STI
{
namespace Device
{

class DeviceEventReceiver;
class DeviceID;
class DeviceEventListenerID;

//Listeners

// typedef DeviceEventListener<RefreshDeviceEvent> RefreshDeviceEventListener;
// typedef DeviceEventListener<ChannelUpdateDeviceEvent> ChannelUpdateDeviceEventListener;

//class RefreshDeviceEventListener : public DeviceEventListener<RefreshDeviceEvent> {};
//class ChannelUpdateDeviceEventListener : public DeviceEventListener<ChannelUpdateDeviceEvent> {};
//...


//Java DeviceEventReceiver wrapper
class JDeviceEventReceiver
{
public:
	
	JDeviceEventReceiver(std::shared_ptr<STI::Device::DeviceEventReceiver>& receiver);
	~JDeviceEventReceiver();

    // void addListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID, 
	// 	const std::shared_ptr<RefreshDeviceEventListener>& listener);
    // void addListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID, 
	// 	const std::shared_ptr<ChannelUpdateDeviceEventListener>& listener);

    //Need to write JRefreshDeviceEventListener which wraps a new shared_ptr that gets made in the constructor. 
    void addListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID, 
		                 const std::shared_ptr<DeviceEventListener<RefreshDeviceEvent>>& listener);
    void addListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID, 
		                 const std::shared_ptr<DeviceEventListener<ChannelUpdateDeviceEvent>>& listener);
    //...

    void removeListener(const DeviceID& sourceDeviceID, const DeviceEventListenerID& listenerID);

private:

    std::shared_ptr<DeviceEventReceiver> deviceEventReceiver;

    std::shared_ptr<STI::Device::DeviceEventListener<RefreshDeviceEvent>> temp;
};

} //Device
} //STI

#endif
