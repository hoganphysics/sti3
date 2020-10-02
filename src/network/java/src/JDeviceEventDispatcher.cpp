
#include "JDeviceEventDispatcher.h"
#include "DeviceEventDispatcher.h"
#include "DeviceEvent.h"

#include <memory>

using STI::Device::JDeviceEventDispatcher;


JDeviceEventDispatcher::JDeviceEventDispatcher(const std::shared_ptr<STI::Device::DeviceEventDispatcher>& dispatcher)
: localDispatcher(dispatcher)
{
}

JDeviceEventDispatcher::~JDeviceEventDispatcher()
{
}


void JDeviceEventDispatcher::addEvent(const std::shared_ptr<STI::Device::DeviceEvent>& evt)
{
    if(localDispatcher != 0) {
        localDispatcher->addEvent(evt);
    }
}

void JDeviceEventDispatcher::clearEvents()
{
    if(localDispatcher != 0) {
        localDispatcher->clearEvents();
    }
}
