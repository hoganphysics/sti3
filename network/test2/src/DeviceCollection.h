#ifndef STI_DEVICE_DEVICECOLLECTION_H
#define STI_DEVICE_DEVICECOLLECTION_H

#include "Collection.h"
#include "Device.h"
#include "DeviceID.h"

namespace STI
{
namespace Device
{

//A Collector has a Collection
typedef STI::Utils::Collector<STI::Device::DeviceID, STI::Device::Device> DeviceCollector;
typedef STI::Utils::Collection<STI::Device::DeviceID, STI::Device::Device> DeviceCollection;

} //Device
} //STI

#endif
