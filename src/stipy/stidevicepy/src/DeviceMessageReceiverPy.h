#ifndef STI_PYTHON_DEVICEMESSAGERECEIVERPY_H
#define STI_PYTHON_DEVICEMESSAGERECEIVERPY_H

#include <sti/device/DeviceMessageReceiver.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceMessageListener.h>

#include <memory>
#include <string>
#include <functional>


namespace STI
{
namespace Python
{

class DeviceMessageReceiverPy
{
public:

    DeviceMessageReceiverPy(const std::shared_ptr<STI::Device::DeviceMessageReceiver>& receiver)
    : receiver(receiver) {}
    
    void addRefreshListener(const STI::Device::DeviceID& sourceDeviceID, const std::string& listenerName, 
		const std::function<void (const std::shared_ptr<STI::Device::RefreshDeviceMessage>&)>& handler)
    {
        if (receiver == 0) return;
        receiver->addListener<STI::Device::RefreshDeviceMessage>(sourceDeviceID, listenerName, handler);
    }

    void addCollectionUpdateListener(const STI::Device::DeviceID& sourceDeviceID, const std::string& listenerName, 
		const std::function<void (const std::shared_ptr<STI::Device::CollectionUpdateMessage>&)>& handler)
    {
        if (receiver == 0) return;
        receiver->addListener<STI::Device::CollectionUpdateMessage>(sourceDeviceID, listenerName, handler);
    }
    
    void addChannelUpdateListener(const STI::Device::DeviceID& sourceDeviceID, const std::string& listenerName, 
		const std::function<void (const std::shared_ptr<STI::Device::ChannelUpdateMessage>&)>& handler)
    {
        if (receiver == 0) return;
        receiver->addListener<STI::Device::ChannelUpdateMessage>(sourceDeviceID, listenerName, handler);
    }
	
    void addAttributeUpdateListener(const STI::Device::DeviceID& sourceDeviceID, const std::string& listenerName, 
		const std::function<void (const std::shared_ptr<STI::Device::AttributeUpdateMessage>&)>& handler)
    {
        if (receiver == 0) return;
        receiver->addListener<STI::Device::AttributeUpdateMessage>(sourceDeviceID, listenerName, handler);
    }

    void addEngineJobUpdateListener(const STI::Device::DeviceID& sourceDeviceID, const std::string& listenerName, 
		const std::function<void (const std::shared_ptr<STI::Device::EngineJobUpdateDeviceMessage>&)>& handler)
    {
        if (receiver == 0) return;
        receiver->addListener<STI::Device::EngineJobUpdateDeviceMessage>(sourceDeviceID, listenerName, handler);
    }

    void addEngineSchedulerMessageListener(const STI::Device::DeviceID& sourceDeviceID, const std::string& listenerName, 
		const std::function<void (const std::shared_ptr<STI::Device::EngineSchedulerMessage>&)>& handler)
    {
        if (receiver == 0) return;
        receiver->addListener<STI::Device::EngineSchedulerMessage>(sourceDeviceID, listenerName, handler);
    }

    void addEngineStateMessageListener(const STI::Device::DeviceID& sourceDeviceID, const std::string& listenerName, 
		const std::function<void (const std::shared_ptr<STI::Device::EngineStateMessage>&)>& handler)
    {
        if (receiver == 0) return;
        receiver->addListener<STI::Device::EngineStateMessage>(sourceDeviceID, listenerName, handler);
    }
    
    void removeListener(const STI::Device::DeviceID& sourceDeviceID, const STI::Device::DeviceMessageType& type, const std::string& name)
    {
        if (receiver == 0) return;
        
        STI::Device::DeviceMessageListenerID listenerID(type, name);
        
        receiver->removeListener(sourceDeviceID, listenerID);
    }

	void clearListeners()
    {
        if (receiver == 0) return;
        receiver->clearListeners();
    }

private:

    std::shared_ptr<STI::Device::DeviceMessageReceiver> receiver;

};


} //Python
} //STI

#endif

