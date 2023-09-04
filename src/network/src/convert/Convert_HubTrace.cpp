
#include "NetworkConvert.h"

#include <sti/network/HubTrace.h>
#include <sti/network/HubID.h>

#include "generated/orbTypes.h"

using STI::Network::convert;
using STI::Network::HubTrace;
using STI::TNetwork::TDeviceHubTrace;


template<>
bool STI::Network::convert<HubTrace, TDeviceHubTrace>(const HubTrace& hubTrace, TDeviceHubTrace& tHubTrace)
{
	using STI::TNetwork::TDeviceHubID;

	return convert(hubTrace.getIDs(), (_CORBA_Unbounded_Sequence<TDeviceHubID>&) tHubTrace.ids);
}

template<>
bool STI::Network::convert<TDeviceHubTrace, HubTrace>(const TDeviceHubTrace& tHubTrace, HubTrace& hubTrace)
{
	using STI::TNetwork::TDeviceHubID;

	bool success = false;
	std::vector<STI::Network::HubID> ids;

	if (convert((_CORBA_Unbounded_Sequence<TDeviceHubID>&) tHubTrace, ids)) {
		for (auto& id : ids) {
			hubTrace.addHubID(id);
		}
		success = hubTrace.size() == tHubTrace.ids.length();
	}

	return success;
}

template<>
TDeviceHubTrace STI::Network::convert<HubTrace, TDeviceHubTrace>(const HubTrace& hubTrace)
{
	TDeviceHubTrace tDeviceHubTrace;

	convert<HubTrace, TDeviceHubTrace>(hubTrace, tDeviceHubTrace);

	return tDeviceHubTrace;
}

template<>
HubTrace STI::Network::convert<TDeviceHubTrace, HubTrace>(const TDeviceHubTrace& tDeviceHubTrace)
{
	HubTrace hubTrace;

	convert<TDeviceHubTrace, HubTrace>(tDeviceHubTrace, hubTrace);

	return hubTrace;
}

