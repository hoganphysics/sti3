
#include "Convert_Monitor.h"
#include "RemoteMonitor.h"


using STI::Network::convert;
using STI::Device::Monitor;
using STI::Network::RemoteMonitor;
using STI::TNetwork::TMonitor;
using STI::Device::MonitorStatus;
using STI::TNetwork::TMonitorStatus;

using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValue;


template<>
std::shared_ptr<Monitor> STI::Network::convert<TMonitor, std::shared_ptr<Monitor>>(const TMonitor& tMonitor)
{
    std::shared_ptr<Monitor> monitor;
    convert<TMonitor, std::shared_ptr<Monitor>>(tMonitor, monitor);
    return monitor;
}

template<>
TMonitor STI::Network::convert<std::shared_ptr<Monitor>, TMonitor>(const std::shared_ptr<Monitor>& monitor)
{
    TMonitor tMonitor;
    convert<std::shared_ptr<Monitor>, TMonitor>(monitor, tMonitor);
    return tMonitor;
}


template<>
bool STI::Network::convert<TMonitor, std::shared_ptr<Monitor>>(const TMonitor& tMonitor, std::shared_ptr<Monitor>& monitor)
{
    auto remoteMonitor = convert<TMonitor, std::shared_ptr<RemoteMonitor>>(tMonitor);
    monitor = std::static_pointer_cast<Monitor>(remoteMonitor);
    return monitor != 0;
}

template<>
bool STI::Network::convert<std::shared_ptr<Monitor>, TMonitor>(const std::shared_ptr<Monitor>& monitor, TMonitor& tMonitor)
{
    if (monitor == 0) return false;

    tMonitor.id = convert<std::string, ::CORBA::String_member>(monitor->getID());
    tMonitor.group = convert<std::string, ::CORBA::String_member>(monitor->getGroup());
    tMonitor.status = convert<MonitorStatus, TMonitorStatus>(monitor->getStatus());
    tMonitor.value = convert<MixedValue, TMixedValue>(monitor->getValue());
    tMonitor.metaData = convert<MixedValue, TMixedValue>(monitor->getMetaData());

    return true;
}

template<>
std::shared_ptr<RemoteMonitor> STI::Network::convert<TMonitor, std::shared_ptr<RemoteMonitor>>(const TMonitor& tMonitor)
{
    return std::make_shared<RemoteMonitor>(
        convert<::CORBA::String_member, std::string>(tMonitor.id),
        convert<::CORBA::String_member, std::string>(tMonitor.group),
        convert<TMonitorStatus, MonitorStatus>(tMonitor.status),
        convert<TMixedValue, MixedValue>(tMonitor.value),
        convert<TMixedValue, MixedValue>(tMonitor.metaData)
    );
}

template<>
bool STI::Network::convert<TMonitor, std::shared_ptr<RemoteMonitor>>(const TMonitor& tMonitor, std::shared_ptr<RemoteMonitor>& monitor)
{
    monitor = convert<TMonitor, std::shared_ptr<RemoteMonitor>>(tMonitor);
    return monitor != 0;
}


//MonitorStatus
template<>
MonitorStatus STI::Network::convert<TMonitorStatus, MonitorStatus>(const TMonitorStatus& tStatus)
{
    //enum class MonitorStatus { Active, Inactive, Missing };
    //TMonitorStatus { MonitorActive, MonitorInactive, MonitorMissing };

    MonitorStatus status;

    switch (tStatus) 
    {
    case TMonitorStatus::MonitorActive:
        status = MonitorStatus::Active;
        break;
    case TMonitorStatus::MonitorInactive:
        status = MonitorStatus::Inactive;
        break;
    case TMonitorStatus::MonitorMissing:
        status = MonitorStatus::Missing;
        break;
    default:
        status = MonitorStatus::Missing;
        break;
    }
    return status;
}

template<>
TMonitorStatus STI::Network::convert<MonitorStatus, TMonitorStatus>(const MonitorStatus& status)
{
    TMonitorStatus tStatus;

    switch (status) 
    {
    case MonitorStatus::Active:
        tStatus = TMonitorStatus::MonitorActive;
        break;
    case MonitorStatus::Inactive:
        tStatus = TMonitorStatus::MonitorInactive;
        break;
    case MonitorStatus::Missing:
        tStatus = TMonitorStatus::MonitorMissing;
        break;
    default:
        tStatus = TMonitorStatus::MonitorMissing;
        break;
    }

    return tStatus;
}
