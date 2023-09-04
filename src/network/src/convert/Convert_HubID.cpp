#include "NetworkConvert.h"

#include <sti/network/HubID.h>

#include "orbTypes.h"

using STI::Network::convert;
using STI::Network::HubID;
using STI::TNetwork::TDeviceHubID;


template<>
HubID STI::Network::convert<TDeviceHubID, HubID>(const TDeviceHubID& tHubID)
{
	return HubID(
		convert<CORBA::String_member, std::string>(tHubID.name),
		convert<CORBA::String_member, std::string>(tHubID.address),
		static_cast<unsigned short>(tHubID.moduleNum)
	);
}

template<>
TDeviceHubID STI::Network::convert<HubID, TDeviceHubID>(const HubID& hubID)
{
	TDeviceHubID tDeviceHubID;

	convert<HubID, TDeviceHubID>(hubID, tDeviceHubID);

	return tDeviceHubID;
}

template<>
bool STI::Network::convert<HubID, TDeviceHubID>(const HubID& hubID, TDeviceHubID& tHubID)
{
	using std::string;
	using ::CORBA::String_member;

	tHubID.name = convert<string, String_member>(hubID.name);
	tHubID.address = convert<string, String_member>(hubID.address);
	tHubID.moduleNum = static_cast<CORBA::UShort>(hubID.module);

	return true;
}

template<>
bool STI::Network::convert<TDeviceHubID, HubID>(const TDeviceHubID& tHubID, HubID& hubID)
{
	hubID = convert<TDeviceHubID, HubID>(tHubID);
	return true;
}
