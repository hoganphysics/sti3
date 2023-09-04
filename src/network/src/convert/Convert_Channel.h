#ifndef STI_NETWORK_CONVERT_CHANNEL_H
#define STI_NETWORK_CONVERT_CHANNEL_H

#include "NetworkConvert.h"
#include "generated/deviceNet.h"
#include <sti/device/Channel.h>

#include <memory>


namespace STI
{

namespace Device
{
class Channel;
} //Device

namespace Network
{
class RemoteChannel;
} //Network


//Channel
template<>
bool Network::convert<std::shared_ptr<Device::Channel>, TNetwork::TChannel>(const std::shared_ptr<Device::Channel>& channel, TNetwork::TChannel& tChannel);
template<>
bool Network::convert<TNetwork::TChannel, std::shared_ptr<Network::RemoteChannel>>(const TNetwork::TChannel& tChannel, std::shared_ptr<Network::RemoteChannel>& channel);

template<>
std::shared_ptr<Network::RemoteChannel> Network::convert<TNetwork::TChannel, std::shared_ptr<Network::RemoteChannel>>(const TNetwork::TChannel& tChannel);


//ChannelType
template<>
TNetwork::TChannelType Network::convert<Device::ChannelType, TNetwork::TChannelType>(const Device::ChannelType& type);
template<>
Device::ChannelType Network::convert<TNetwork::TChannelType, Device::ChannelType>(const TNetwork::TChannelType& tType);



} //STI

#endif

