#ifndef STI_NETWORK_CONVERT_ATTRIBUTE_H
#define STI_NETWORK_CONVERT_ATTRIBUTE_H

#include "NetworkConvert.h"
#include "deviceNet.h"
// #include "Attribute.h"

#include <memory>

namespace STI
{

namespace Device
{
class Attribute;
} //Device

namespace Network
{
class RemoteAttribute;
} //Network


//Attribute
template<>
bool Network::convert<std::shared_ptr<Device::Attribute>, TNetwork::TAttribute>(const std::shared_ptr<Device::Attribute>& attribute, TNetwork::TAttribute& tAttribute);
template<>
bool Network::convert<TNetwork::TAttribute, std::shared_ptr<Device::Attribute>>(const TNetwork::TAttribute& tAttribute, std::shared_ptr<Device::Attribute>& attribute);

template<>
std::shared_ptr<Device::Attribute> Network::convert<TNetwork::TAttribute, std::shared_ptr<Device::Attribute>>(const TNetwork::TAttribute& tAttribute);


template<>
bool Network::convert<TNetwork::TAttribute, std::shared_ptr<Network::RemoteAttribute>>(const TNetwork::TAttribute& tAttribute, std::shared_ptr<Network::RemoteAttribute>& attribute);
template<>
std::shared_ptr<Network::RemoteAttribute> Network::convert<TNetwork::TAttribute, std::shared_ptr<Network::RemoteAttribute>>(const TNetwork::TAttribute& tAttribute);





} //STI

#endif

