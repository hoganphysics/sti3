
#ifndef STI_NETWORK_CONVERT_RAWEVENTGROUP_H
#define STI_NETWORK_CONVERT_RAWEVENTGROUP_H

#include "NetworkConvert.h"
#include "generated/deviceNet.h"
#include "generated/orbTypes.h"

#include <memory>
#include <vector>


namespace STI
{

namespace Engine
{

class RawEventGroup;
class StackTraceData;
class PostProcessRequest;

} //Engine


namespace Network
{

bool convertGroup(const std::shared_ptr<Engine::RawEventGroup>& rawEventGroup, TNetwork::TRawEventGroup& tRawEventGroup);
bool convertGroup(const TNetwork::TRawEventGroup& tRawEventGroup, std::shared_ptr<Engine::RawEventGroup>& rawEventGroup);

} //Network




//RawEventGroup
template<>
bool Network::convert<std::shared_ptr<Engine::RawEventGroup>, TNetwork::TRawEventGroup>(
        const std::shared_ptr<Engine::RawEventGroup>& rawEventGroup, TNetwork::TRawEventGroup& tRawEventGroup);
template<>
bool Network::convert<TNetwork::TRawEventGroup, std::shared_ptr<Engine::RawEventGroup>>(
        const TNetwork::TRawEventGroup& tRawEventGroup, std::shared_ptr<Engine::RawEventGroup>& rawEventGroup);


//PostProcessRequest (single element; the seq<->vector forms are handled by the
//generic list converters in NetworkConvert.h once these are specialized).
template<>
bool Network::convert<Engine::PostProcessRequest, TNetwork::TPostProcessRequest>(
        const Engine::PostProcessRequest& request, TNetwork::TPostProcessRequest& tRequest);
template<>
bool Network::convert<TNetwork::TPostProcessRequest, Engine::PostProcessRequest>(
        const TNetwork::TPostProcessRequest& tRequest, Engine::PostProcessRequest& request);

//Value-returning form (used by ConvertList for the seq->vector direction).
template<>
Engine::PostProcessRequest Network::convert<TNetwork::TPostProcessRequest, Engine::PostProcessRequest>(
        const TNetwork::TPostProcessRequest& tRequest);




} //STI

#endif

