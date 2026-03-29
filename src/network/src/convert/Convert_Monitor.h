#ifndef STI_NETWORK_CONVERT_MONITOR_H
#define STI_NETWORK_CONVERT_MONITOR_H

#include "NetworkConvert.h"
#include "generated/deviceNet.h"
#include "generated/orbTypes.h"


#include <sti/device/Monitor.h>

#include <memory>
#include <vector>


namespace STI
{

namespace Device
{


} //Device

namespace Network
{

class RemoteMonitor;

} //Network



template<>
std::shared_ptr<Device::Monitor> Network::convert<TNetwork::TMonitor, std::shared_ptr<Device::Monitor>>(const TNetwork::TMonitor& tMonitor);
template<>
TNetwork::TMonitor Network::convert<std::shared_ptr<Device::Monitor>, TNetwork::TMonitor>(const std::shared_ptr<Device::Monitor>& monitor);


template<>
bool Network::convert<TNetwork::TMonitor, std::shared_ptr<Device::Monitor>>(const TNetwork::TMonitor& tMonitor, std::shared_ptr<Device::Monitor>& monitor);
template<>
bool Network::convert<std::shared_ptr<Device::Monitor>, TNetwork::TMonitor>(const std::shared_ptr<Device::Monitor>& monitor, TNetwork::TMonitor& tMonitor);

template<>
std::shared_ptr<Network::RemoteMonitor> Network::convert<TNetwork::TMonitor, std::shared_ptr<Network::RemoteMonitor>>(const TNetwork::TMonitor& tMonitor);
template<>
bool Network::convert<TNetwork::TMonitor, std::shared_ptr<Network::RemoteMonitor>>(const TNetwork::TMonitor& tMonitor, std::shared_ptr<Network::RemoteMonitor>& monitor);


//MonitorStatus
template<>
Device::MonitorStatus Network::convert<TNetwork::TMonitorStatus, Device::MonitorStatus>(const TNetwork::TMonitorStatus& tStatus);
template<>
TNetwork::TMonitorStatus Network::convert<Device::MonitorStatus, TNetwork::TMonitorStatus>(const Device::MonitorStatus& status);


} //STI

#endif
