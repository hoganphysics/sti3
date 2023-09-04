
#ifndef STI_NETWORK_CONVERT_SEQUENCERESULT_H
#define STI_NETWORK_CONVERT_SEQUENCERESULT_H

#include "NetworkConvert.h"
#include "deviceNet.h"
#include "orbTypes.h"

#include <sti/engine/Sequence.h>

#include <memory>
#include <vector>


namespace STI
{

namespace Engine
{

class SequenceID;
class SequenceResult;
class SequenceEntryID;
class Sequence;
class SequenceIndex;


} //Engine


//SequenceResult
template<>
bool Network::convert<TNetwork::TSequenceResult, std::shared_ptr<Engine::SequenceResult>>(
        const TNetwork::TSequenceResult& tSequenceResult, std::shared_ptr<Engine::SequenceResult>& sequenceResult);
template<>
bool Network::convert<std::shared_ptr<Engine::SequenceResult>, TNetwork::TSequenceResult>(
        const std::shared_ptr<Engine::SequenceResult>& sequenceResult, TNetwork::TSequenceResult& tSequenceResult);


//SequenceID
template<>
Engine::SequenceID Network::convert<TNetwork::TSequenceID, Engine::SequenceID>(const TNetwork::TSequenceID& tSequenceID);
template<>
TNetwork::TSequenceID Network::convert<Engine::SequenceID, TNetwork::TSequenceID>(const Engine::SequenceID& sequenceID);
template<>
bool Network::convert<TNetwork::TSequenceID, Engine::SequenceID>(
        const TNetwork::TSequenceID& tSequenceID, Engine::SequenceID& sequenceID);
template<>
bool Network::convert<Engine::SequenceID, TNetwork::TSequenceID>(
        const Engine::SequenceID& sequenceID, TNetwork::TSequenceID& tSequenceID);


//SequenceEntryID
template<>
Engine::SequenceEntryID Network::convert<TNetwork::TSequenceEntryID, Engine::SequenceEntryID>(const TNetwork::TSequenceEntryID& tSequenceEntryID);
template<>
TNetwork::TSequenceEntryID Network::convert<Engine::SequenceEntryID, TNetwork::TSequenceEntryID>(const Engine::SequenceEntryID& sequenceEntryID);


//SequenceType
template<>
Engine::SequenceType Network::convert<TNetwork::TSequenceType, Engine::SequenceType>(const TNetwork::TSequenceType& tSequenceType);
template<>
TNetwork::TSequenceType Network::convert<Engine::SequenceType, TNetwork::TSequenceType>(const Engine::SequenceType& sequenceType);


//SequenceEntry
template<>
bool Network::convert<TNetwork::TSequenceEntry, Engine::SequenceEntry>(
        const TNetwork::TSequenceEntry& tSequenceEntry, Engine::SequenceEntry& sequenceEntry);
template<>
bool Network::convert<Engine::SequenceEntry, TNetwork::TSequenceEntry>(
        const Engine::SequenceEntry& sequenceEntry, TNetwork::TSequenceEntry& tSequenceEntry);


//Sequence
template<>
bool Network::convert<TNetwork::TSequence, std::shared_ptr<Engine::Sequence>>(
        const TNetwork::TSequence& tSequence, std::shared_ptr<Engine::Sequence>& sequence);
template<>
bool Network::convert<std::shared_ptr<Engine::Sequence>, TNetwork::TSequence>(
        const std::shared_ptr<Engine::Sequence>& sequence, TNetwork::TSequence& tSequence);


//SequenceIndex
template<>
Engine::SequenceIndex Network::convert<TNetwork::TSequenceIndex, Engine::SequenceIndex>(const TNetwork::TSequenceIndex& tSequenceIndex);
template<>
TNetwork::TSequenceIndex Network::convert<Engine::SequenceIndex, TNetwork::TSequenceIndex>(const Engine::SequenceIndex& sequenceIndex);


} //STI

#endif

