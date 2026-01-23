#include "TTriggerCallback_i.h"

#include "NetworkConvert.h"
#include <sti/device/DeviceID.h>
#include "TriggerCallback.h"

using STI::TNetwork::TTriggerCallback_i;
using ::STI::TNetwork::TDeviceID;
using STI::Device::DeviceID;
using STI::Network::convert;


TTriggerCallback_i::TTriggerCallback_i(const std::shared_ptr<STI::Engine::TriggerCallback>& triggerCB)
: _triggerCB(triggerCB)
{
}

TTriggerCallback_i::~TTriggerCallback_i()
{
}

void TTriggerCallback_i::ready(const TDeviceID& id)
{
    if (_triggerCB != 0) {
        _triggerCB->ready(convert<TDeviceID, DeviceID>(id));
    }
}

void TTriggerCallback_i::triggerFired(const TDeviceID& id)
{
    if (_triggerCB != 0) {
        _triggerCB->triggerFired(convert<TDeviceID, DeviceID>(id));
    }
}
