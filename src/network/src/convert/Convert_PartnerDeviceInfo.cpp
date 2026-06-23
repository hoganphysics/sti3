#include "NetworkConvert.h"

#include <sti/device/PartnerDeviceInfo.h>

using STI::Device::PartnerDeviceInfo;
using STI::Network::convert;
using STI::TNetwork::TPartnerDeviceInfo;

template<>
TPartnerDeviceInfo STI::Network::convert<PartnerDeviceInfo, TPartnerDeviceInfo>(const PartnerDeviceInfo& partner)
{
	TPartnerDeviceInfo tPartner;
	convert<PartnerDeviceInfo, TPartnerDeviceInfo>(partner, tPartner);
	return tPartner;
}

template<>
PartnerDeviceInfo STI::Network::convert<TPartnerDeviceInfo, PartnerDeviceInfo>(const TPartnerDeviceInfo& tPartner)
{
	PartnerDeviceInfo partner;
	convert<TPartnerDeviceInfo, PartnerDeviceInfo>(tPartner, partner);
	return partner;
}

template<>
bool STI::Network::convert<PartnerDeviceInfo, TPartnerDeviceInfo>(const PartnerDeviceInfo& partner, TPartnerDeviceInfo& tPartner)
{
	convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(partner.deviceID, tPartner.deviceID);
	convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(partner.aliases, tPartner.aliases);
	tPartner.eventTarget = static_cast<CORBA::Boolean>(partner.eventTarget);
	return true;
}

template<>
bool STI::Network::convert<TPartnerDeviceInfo, PartnerDeviceInfo>(const TPartnerDeviceInfo& tPartner, PartnerDeviceInfo& partner)
{
	partner.deviceID = convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tPartner.deviceID);
	convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tPartner.aliases, partner.aliases);
	partner.eventTarget = static_cast<bool>(tPartner.eventTarget);
	return true;
}
