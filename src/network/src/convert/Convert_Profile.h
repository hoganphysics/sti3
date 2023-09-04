
#ifndef STI_NETWORK_CONVERT_PROFILE_H
#define STI_NETWORK_CONVERT_PROFILE_H

#include "NetworkConvert.h"
#include "generated/deviceNet.h"
#include "generated/orbTypes.h"

#include <sti/device/Profile.h>

#include <memory>
#include <vector>


namespace STI
{

namespace Device
{

} //Device



//ProfileType
template<>
Device::ProfileType Network::convert<TNetwork::TProfileType, Device::ProfileType>(const TNetwork::TProfileType& tType);

template<>
TNetwork::TProfileType Network::convert<Device::ProfileType, TNetwork::TProfileType>(const Device::ProfileType& type);



//Profile
template<>
bool Network::convert<TNetwork::TProfile, Device::Profile>(const TNetwork::TProfile& tProfile, Device::Profile& profile);

template<>
bool Network::convert<Device::Profile, TNetwork::TProfile>(const Device::Profile& profile, TNetwork::TProfile& tProfile);



} //STI

#endif

