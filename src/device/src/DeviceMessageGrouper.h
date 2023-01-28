#ifndef STI_DEVICE_DEVICEMESSAGEGROUPER_H
#define STI_DEVICE_DEVICEMESSAGEGROUPER_H


#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/device/MessageGrouper.h>


namespace STI
{
namespace Device
{


template<typename Message>
class DeviceMessageGrouper : public MessageGrouper<Message>
{
public:

    DeviceMessageGrouper(const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher) 
    : MessageGrouper<Message>(), messageDispatcher(dispatcher)
    {
    }

private:
    
    void dispatchMessage(const std::shared_ptr<Message>& mess);

    std::shared_ptr<STI::Device::DeviceMessageDispatcher> messageDispatcher;
};


} // Device
} // STI


template<class Message>
void STI::Device::DeviceMessageGrouper<Message>::dispatchMessage(const std::shared_ptr<Message>& mess)
{
    if (messageDispatcher != 0) {
        messageDispatcher->addMessage(mess);            
    }
}


#endif

