
#ifndef STI_NETWORK_CONVERT_SHOTRESULT_H
#define STI_NETWORK_CONVERT_SHOTRESULT_H

#include "NetworkConvert.h"
#include "deviceNet.h"
#include "orbTypes.h"

#include <memory>
#include <vector>


namespace STI
{

namespace Engine
{

class ShotResult;

} //Engine



//ShotResult
template<>
bool Network::convert<TNetwork::TShotResult, std::shared_ptr<Engine::ShotResult>>(
        const TNetwork::TShotResult& tShotResult, std::shared_ptr<Engine::ShotResult>& shotResult);
template<>
bool Network::convert<std::shared_ptr<Engine::ShotResult>, TNetwork::TShotResult>(
        const std::shared_ptr<Engine::ShotResult>& shotResult, TNetwork::TShotResult& tShotResult);




} //STI

#endif

