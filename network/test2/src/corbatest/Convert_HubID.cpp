
#include "NetworkConvert.h"

#include "HubID.h"

#include "orbTypes.h"

using STI::Network::convert;
using STI::Network::HubID;
using STI::TNetwork::TDeviceHubID;


template<>
HubID convert<TDeviceHubID, HubID>(const TDeviceHubID& tHubID)
{
	return HubID(
		convert<CORBA::String_member, std::string>(tHubID.name),
		convert<CORBA::String_member, std::string>(tHubID.address)
	);
}

template<>
TDeviceHubID convert<HubID, TDeviceHubID>(const HubID& hubID)
{
	TDeviceHubID tDeviceHubID;

	convert<HubID, TDeviceHubID>(hubID, tDeviceHubID);

	return tDeviceHubID;
}

template<>
bool convert<HubID, TDeviceHubID>(const HubID& hubID, TDeviceHubID& tHubID)
{
	using std::string;
	using ::CORBA::String_member;

	tHubID.name = convert<string, String_member>(hubID.name);
	tHubID.address = convert<string, String_member>(hubID.address);

	return true;
}

