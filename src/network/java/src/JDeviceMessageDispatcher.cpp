
#include "JDeviceMessageDispatcher.h"
#include "DeviceMessageDispatcher.h"
#include "DeviceMessage.h"

#include <memory>

using STI::Device::JDeviceMessageDispatcher;


JDeviceMessageDispatcher::JDeviceMessageDispatcher(std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher)
: localDispatcher(dispatcher)
{
}

JDeviceMessageDispatcher::~JDeviceMessageDispatcher()
{
}


void JDeviceMessageDispatcher::addMessage(const std::shared_ptr<STI::Device::DeviceMessage>& mess)
{
    if(localDispatcher != 0) {
        localDispatcher->addMessage(mess);
    }
}

void JDeviceMessageDispatcher::clearMessages()
{
    if(localDispatcher != 0) {
        localDispatcher->clearMessages();
    }
}
