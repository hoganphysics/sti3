#ifndef STI_NETWORK_CONVERT_DEVICEMESSAGE_H
#define STI_NETWORK_CONVERT_DEVICEMESSAGE_H

#include "NetworkConvert.h"
#include "DeviceMessage.h"

#include "deviceNet.h"

#include <memory>

namespace STI
{

namespace Device
{

class DeviceTrace;

class DeviceMessage;
class RefreshDeviceMessage;
enum class DeviceMessageType;

} //Device



//DeviceMessage
template<>
bool Network::convert<std::shared_ptr<STI::Device::DeviceMessage>, TNetwork::TDeviceMessage>(const std::shared_ptr<STI::Device::DeviceMessage>& deviceMessage, TNetwork::TDeviceMessage& tMessage);
template<>
bool Network::convert<TNetwork::TDeviceMessage, std::shared_ptr<STI::Device::DeviceMessage>>(const TNetwork::TDeviceMessage& tMessage, std::shared_ptr<STI::Device::DeviceMessage>& deviceMessage);

template<>
bool Network::convert<std::shared_ptr<STI::Device::DeviceMessage>, TNetwork::TAnyMessage>(const std::shared_ptr<STI::Device::DeviceMessage>& deviceMessage, TNetwork::TAnyMessage& tAnyMessage);
template<>
bool Network::convert<TNetwork::TAnyMessage, std::shared_ptr<STI::Device::DeviceMessage>>(const TNetwork::TAnyMessage& tAnyMessage, std::shared_ptr<STI::Device::DeviceMessage>& deviceMessage);

template<>
TNetwork::TDeviceMessageType Network::convert<Device::DeviceMessageType, TNetwork::TDeviceMessageType>(const Device::DeviceMessageType& type);
template<>
enum Device::DeviceMessageType Network::convert<TNetwork::TDeviceMessageType, Device::DeviceMessageType>(const TNetwork::TDeviceMessageType& tType);
template<>
bool Network::convert<Device::DeviceMessageType, TNetwork::TDeviceMessageType>(const Device::DeviceMessageType& type, TNetwork::TDeviceMessageType& tType);
template<>
bool Network::convert<TNetwork::TDeviceMessageType, Device::DeviceMessageType>(const TNetwork::TDeviceMessageType& tType, Device::DeviceMessageType& type);


//RefreshDeviceMessage
template<>
bool Network::convert<TNetwork::TRefreshDeviceMessage, std::shared_ptr<Device::RefreshDeviceMessage>>(
	const TNetwork::TRefreshDeviceMessage& tMessage, std::shared_ptr<Device::RefreshDeviceMessage>& deviceMessage);
template<>
bool Network::convert<std::shared_ptr<Device::RefreshDeviceMessage>, TNetwork::TRefreshDeviceMessage>(
	const std::shared_ptr<Device::RefreshDeviceMessage>& deviceMessage, TNetwork::TRefreshDeviceMessage& tMessage);


//CollectionUpdateMessage
template<>
bool Network::convert<TNetwork::TCollectionUpdateMessage, std::shared_ptr<Device::CollectionUpdateMessage>>(
	const TNetwork::TCollectionUpdateMessage& tMessage, std::shared_ptr<Device::CollectionUpdateMessage>& deviceMessage);
template<>
bool Network::convert<std::shared_ptr<Device::CollectionUpdateMessage>, TNetwork::TCollectionUpdateMessage>(
	const std::shared_ptr<Device::CollectionUpdateMessage>& deviceMessage, TNetwork::TCollectionUpdateMessage& tMessage);


//CollectionMessageType
template<>
TNetwork::TCollectionMessageType Network::convert<Device::CollectionUpdateMessage::CollectionMessageType, TNetwork::TCollectionMessageType>(
	const Device::CollectionUpdateMessage::CollectionMessageType& type);
template<>
Device::CollectionUpdateMessage::CollectionMessageType Network::convert<TNetwork::TCollectionMessageType, Device::CollectionUpdateMessage::CollectionMessageType>(
	const TNetwork::TCollectionMessageType& tType);



//EngineSchedulerMessage
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



//EngineParserDeviceMessage
template<>
bool Network::convert<TNetwork::TEngineParserDeviceMessage, std::shared_ptr<Device::EngineParserDeviceMessage>>(
	const TNetwork::TEngineParserDeviceMessage& tMessage, std::shared_ptr<Device::EngineParserDeviceMessage>& deviceMessage);
template<>
bool Network::convert<std::shared_ptr<Device::EngineParserDeviceMessage>, TNetwork::TEngineParserDeviceMessage>(
	const std::shared_ptr<Device::EngineParserDeviceMessage>& deviceMessage, TNetwork::TEngineParserDeviceMessage& tMessage);



//ChannelUpdateMessage
template<>
bool Network::convert<TNetwork::TChannelUpdateMessage, std::shared_ptr<Device::ChannelUpdateMessage>>(
	const TNetwork::TChannelUpdateMessage& tMessage, std::shared_ptr<Device::ChannelUpdateMessage>& deviceMessage);
template<>
bool Network::convert<std::shared_ptr<Device::ChannelUpdateMessage>, TNetwork::TChannelUpdateMessage>(
	const std::shared_ptr<Device::ChannelUpdateMessage>& deviceMessage, TNetwork::TChannelUpdateMessage& tMessage);

//ChannelUpdateMessageType
template<>
TNetwork::TChannelUpdateMessageType Network::convert<Device::ChannelUpdateMessage::ChannelUpdateMessageType, TNetwork::TChannelUpdateMessageType>(const Device::ChannelUpdateMessage::ChannelUpdateMessageType& type);
template<>
Device::ChannelUpdateMessage::ChannelUpdateMessageType Network::convert<TNetwork::TChannelUpdateMessageType, Device::ChannelUpdateMessage::ChannelUpdateMessageType>(const TNetwork::TChannelUpdateMessageType& tType);



//AttributeUpdateMessage
template<>
bool Network::convert<TNetwork::TAttributeUpdateMessage, std::shared_ptr<Device::AttributeUpdateMessage>>(
	const TNetwork::TAttributeUpdateMessage& tMessage, std::shared_ptr<Device::AttributeUpdateMessage>& deviceMessage);
template<>
bool Network::convert<std::shared_ptr<Device::AttributeUpdateMessage>, TNetwork::TAttributeUpdateMessage>(
	const std::shared_ptr<Device::AttributeUpdateMessage>& deviceMessage, TNetwork::TAttributeUpdateMessage& tMessage);



} //STI

#endif

