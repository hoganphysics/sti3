#ifndef STI_NETWORK_DEVICEHUB_H
#define STI_NETWORK_DEVICEHUB_H

#include "Hub.h"
#include "Device.h"
#include "DeviceID.h"

namespace STI
{
namespace Network
{

typedef Hub<STI::Device::DeviceID, STI::Device::Device> DeviceHub;

} //Network
} //STI

#endif
