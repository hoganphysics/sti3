

#include "GroupingDeviceMessageDispatcher.h"
#include "LocalDeviceMessageDispatcher.h"
#include "MessageGrouper.h"

using STI::Device::GroupingDeviceMessageDispatcher;
using STI::Device::LocalDeviceMessageDispatcher;
using STI::Device::DeviceMessage;
using STI::Device::DeviceID;
using STI::Device::DeviceMessageHandler;
using STI::Device::MessageGrouper;


GroupingDeviceMessageDispatcher::GroupingDeviceMessageDispatcher()
{
    defaultDispatcher = std::make_shared<LocalDeviceMessageDispatcher>();
}

GroupingDeviceMessageDispatcher::~GroupingDeviceMessageDispatcher()
{
}

void GroupingDeviceMessageDispatcher::addMessageHandler(const DeviceID& targetID, const std::shared_ptr<DeviceMessageHandler>& handler)
{
    if (defaultDispatcher != 0) {
        defaultDispatcher->addMessageHandler(targetID, handler);
    }
}

void GroupingDeviceMessageDispatcher::removeMessageHandler(const DeviceID& targetID)
{
    if (defaultDispatcher != 0) {
        defaultDispatcher->removeMessageHandler(targetID);
    }
}

bool GroupingDeviceMessageDispatcher::makeMessageHandler(std::shared_ptr<DeviceMessageHandler>& handler)
{
    if (defaultDispatcher != 0) {
        defaultDispatcher->makeMessageHandler(handler);
    }
}

void GroupingDeviceMessageDispatcher::addMessage(const std::shared_ptr<DeviceMessage>& mess)
{
    if (defaultDispatcher != 0) {
        defaultDispatcher->addMessage(mess);
    }
}

void GroupingDeviceMessageDispatcher::clearMessages()
{
    if (defaultDispatcher != 0) {
        defaultDispatcher->clearMessages();
    }
}

// void GroupingDeviceMessageDispatcher::enableGrouping(STI::Device::DeviceMessageType messageType, int warmupTime_ms, int cooldownTime_ms)
// {
//     MessageGrouper<> grouper(defaultDispatcher);
// //    messageGroupers[messageType] = 
// }

void GroupingDeviceMessageDispatcher::disableGrouping(STI::Device::DeviceMessageType messageType)
{

}
