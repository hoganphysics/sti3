#ifndef STI_NETWORK_CONVERT_DEVICETRACE_H
#define STI_NETWORK_CONVERT_DEVICETRACE_H

#include "NetworkConvert.h"
#include "deviceNet.h"



namespace STI
{

namespace Device
{

class DeviceTrace;

} //Device


//DeviceTrace
template<> 
Device::DeviceTrace Network::convert<TNetwork::TDeviceTrace, Device::DeviceTrace>(const TNetwork::TDeviceTrace& tDeviceTrace);
template<>
TNetwork::TDeviceTrace Network::convert<Device::DeviceTrace, TNetwork::TDeviceTrace>(const Device::DeviceTrace& deviceTrace);



} //STI

#endif

