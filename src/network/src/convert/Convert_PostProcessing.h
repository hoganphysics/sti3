#ifndef STI_NETWORK_CONVERT_POSTPROCESSING_H
#define STI_NETWORK_CONVERT_POSTPROCESSING_H

#include "NetworkConvert.h"
#include "generated/orbTypes.h"

#include <sti/device/PostProcessingManager.h>


namespace STI
{

//PostProcessingOptionInfo (single element; seq<->vector handled by the generic
//list converters in NetworkConvert.h once these single-element forms exist).
template<>
bool Network::convert<Device::PostProcessingOptionInfo, TNetwork::TPostProcessingOptionInfo>(
        const Device::PostProcessingOptionInfo& option, TNetwork::TPostProcessingOptionInfo& tOption);
template<>
bool Network::convert<TNetwork::TPostProcessingOptionInfo, Device::PostProcessingOptionInfo>(
        const TNetwork::TPostProcessingOptionInfo& tOption, Device::PostProcessingOptionInfo& option);
template<>
Device::PostProcessingOptionInfo Network::convert<TNetwork::TPostProcessingOptionInfo, Device::PostProcessingOptionInfo>(
        const TNetwork::TPostProcessingOptionInfo& tOption);


//PostProcessingTargetInfo
template<>
bool Network::convert<Device::PostProcessingTargetInfo, TNetwork::TPostProcessingTargetInfo>(
        const Device::PostProcessingTargetInfo& info, TNetwork::TPostProcessingTargetInfo& tInfo);
template<>
bool Network::convert<TNetwork::TPostProcessingTargetInfo, Device::PostProcessingTargetInfo>(
        const TNetwork::TPostProcessingTargetInfo& tInfo, Device::PostProcessingTargetInfo& info);
template<>
Device::PostProcessingTargetInfo Network::convert<TNetwork::TPostProcessingTargetInfo, Device::PostProcessingTargetInfo>(
        const TNetwork::TPostProcessingTargetInfo& tInfo);


} //STI

#endif
