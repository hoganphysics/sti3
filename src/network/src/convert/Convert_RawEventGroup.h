
#ifndef STI_NETWORK_CONVERT_RAWEVENTGROUP_H
#define STI_NETWORK_CONVERT_RAWEVENTGROUP_H

#include "NetworkConvert.h"
#include "deviceNet.h"
#include "orbTypes.h"

#include <memory>
#include <vector>


namespace STI
{

namespace Engine
{

class RawEventGroup;
class StackTraceData;

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




} //STI

#endif

