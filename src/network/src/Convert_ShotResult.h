
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
class ParseResult;
class EngineParseResult;
class RawEventGroup;
class ParsedVar;
class ParsedTag;

} //Engine



//ShotResult
template<>
bool Network::convert<TNetwork::TShotResult, std::shared_ptr<Engine::ShotResult>>(
        const TNetwork::TShotResult& tShotResult, std::shared_ptr<Engine::ShotResult>& shotResult);
template<>
bool Network::convert<std::shared_ptr<Engine::ShotResult>, TNetwork::TShotResult>(
        const std::shared_ptr<Engine::ShotResult>& shotResult, TNetwork::TShotResult& tShotResult);


//ParseResult
template<>
bool Network::convert<TNetwork::TParseResult, Engine::ParseResult>(
        const TNetwork::TParseResult& tParseResult, Engine::ParseResult& parseResult);
template<>
bool Network::convert<Engine::ParseResult, TNetwork::TParseResult>(
        const Engine::ParseResult& parseResult, TNetwork::TParseResult& tParseResult);


//EngineParseResult
template<>
bool Network::convert<TNetwork::TEngineParseResult, Engine::EngineParseResult>(
        const TNetwork::TEngineParseResult& tEngineParseResult, Engine::EngineParseResult& engineParseResult);
template<>
bool Network::convert<Engine::EngineParseResult, TNetwork::TEngineParseResult>(
        const Engine::EngineParseResult& engineParseResult, TNetwork::TEngineParseResult& tEngineParseResult);


//RawEventGroup
template<>
bool Network::convert<TNetwork::TRawEventGroup, Engine::RawEventGroup>(
        const TNetwork::TRawEventGroup& tRawEventGroup, Engine::RawEventGroup& rawEventGroup);
template<>
bool Network::convert<Engine::RawEventGroup, TNetwork::TRawEventGroup>(
        const Engine::RawEventGroup& rawEventGroup, TNetwork::TRawEventGroup& tRawEventGroup);
template<>
Engine::RawEventGroup Network::convert<TNetwork::TRawEventGroup, Engine::RawEventGroup>(
        const TNetwork::TRawEventGroup& tRawEventGroup);
template<>
TNetwork::TRawEventGroup Network::convert<Engine::RawEventGroup, TNetwork::TRawEventGroup>(
        const Engine::RawEventGroup& rawEventGroup);
template<>
std::shared_ptr<Engine::RawEventGroup> Network::convert<TNetwork::TRawEventGroup, std::shared_ptr<Engine::RawEventGroup>>(
        const TNetwork::TRawEventGroup& tRawEventGroup);
template<>
TNetwork::TRawEventGroup Network::convert<std::shared_ptr<Engine::RawEventGroup>, TNetwork::TRawEventGroup>(
        const std::shared_ptr<Engine::RawEventGroup>& rawEventGroup);
template<>
bool Network::convert<std::shared_ptr<Engine::RawEventGroup>, TNetwork::TRawEventGroup>(
        const std::shared_ptr<Engine::RawEventGroup>& rawEventGroup, TNetwork::TRawEventGroup& tRawEventGroup);


//ParsedVar
template<>
bool Network::convert<TNetwork::TParsedVar, Engine::ParsedVar>(
        const TNetwork::TParsedVar& tParsedVar, Engine::ParsedVar& parsedVar);
template<>
bool Network::convert<Engine::ParsedVar, TNetwork::TParsedVar>(
        const Engine::ParsedVar& parsedVar, TNetwork::TParsedVar& tParsedVar);
template<>
Engine::ParsedVar Network::convert<TNetwork::TParsedVar, Engine::ParsedVar>(
        const TNetwork::TParsedVar& tParsedVar);
template<>
TNetwork::TParsedVar Network::convert<Engine::ParsedVar, TNetwork::TParsedVar>(
        const Engine::ParsedVar& parsedVar);


//ParsedTag
template<>
bool Network::convert<TNetwork::TParsedTag, Engine::ParsedTag>(
        const TNetwork::TParsedTag& tParsedTag, Engine::ParsedTag& parsedTag);
template<>
bool Network::convert<Engine::ParsedTag, TNetwork::TParsedTag>(
        const Engine::ParsedTag& parsedTag, TNetwork::TParsedTag& tParsedTag);
template<>
Engine::ParsedTag Network::convert<TNetwork::TParsedTag, Engine::ParsedTag>(
        const TNetwork::TParsedTag& tParsedTag);
template<>
TNetwork::TParsedTag Network::convert<Engine::ParsedTag, TNetwork::TParsedTag>(
        const Engine::ParsedTag& parsedTag);


} //STI

#endif

