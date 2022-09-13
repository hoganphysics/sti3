#ifndef STI_DEVICE_DEVICECOLLECTION_H
#define STI_DEVICE_DEVICECOLLECTION_H

#include <sti/utils/Collection.h>
#include <sti/utils/Collector.h>
//#include <sti/device/Device.h>
#include <sti/device/DeviceID.h>

namespace STI
{
namespace Device
{

class Device;

//A Collector has a Collection
typedef STI::Utils::Collector<STI::Device::DeviceID, STI::Device::Device> DeviceCollector;
typedef STI::Utils::Collection<STI::Device::DeviceID, STI::Device::Device> DeviceCollection;

} //Device
} //STI

#endif
