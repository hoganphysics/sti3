#ifndef STI_NETWORK_CONVERT_DEVICEMESSAGE_H
#define STI_NETWORK_CONVERT_DEVICEMESSAGE_H

#include "NetworkConvert.h"
#include "DeviceEvent.h"

#include "deviceNet.h"

#include <memory>

namespace STI
{

namespace Device
{

class DeviceTrace;

} //Device



template<>
bool Network::convert<TNetwork::TEngineSchedulerMessage, std::shared_ptr<Device::EngineSchedulerMessage>>(
	const TNetwork::TEngineSchedulerMessage& tMessage, std::shared_ptr<Device::EngineSchedulerMessage>& deviceMessage);
template<>
bool Network::convert<std::shared_ptr<Device::EngineSchedulerMessage>, TNetwork::TEngineSchedulerMessage>(
	const std::shared_ptr<Device::EngineSchedulerMessage>& deviceMessage, TNetwork::TEngineSchedulerMessage& tMessage);



  //SchedulerMessageType
template<>
TNetwork::TSchedulerMessageType Network::convert<Device::EngineSchedulerMessage::SchedulerMessageType, TNetwork::TSchedulerMessageType>(const Device::EngineSchedulerMessage::SchedulerMessageType& type);
template<>
Device::EngineSchedulerMessage::SchedulerMessageType Network::convert<TNetwork::TSchedulerMessageType, Device::EngineSchedulerMessage::SchedulerMessageType>(const TNetwork::TSchedulerMessageType& tType);




} //STI

#endif

