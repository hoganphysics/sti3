
#include "Convert_SequenceResult.h"

#include "Convert_EventEngine.h"
#include "Convert_ShotResult.h"

#include <sti/engine/ParsedVar.h>
#include <sti/engine/Sequence.h>
#include <sti/engine/SequenceID.h>
#include <sti/engine/SequenceResult.h>
#include <sti/engine/ShotID.h>

#include <memory>

using STI::Engine::SequenceResult;
using STI::TNetwork::TSequenceResult;
using STI::Engine::SequenceID;
using STI::TNetwork::TSequenceID;
using STI::Engine::SequenceEntryID;
using STI::TNetwork::TSequenceEntryID;
using STI::Engine::Sequence;
using STI::TNetwork::TSequence;
using STI::Utils::TimeStamp;
using STI::TNetwork::TTimeStamp;
using STI::Engine::EngineJobSourceID;
using STI::TNetwork::TEngineJobSourceID;
using STI::Engine::SequenceIndex;
using STI::TNetwork::TSequenceIndex;
using STI::Engine::SequenceType;
using STI::TNetwork::TSequenceType;
using STI::TNetwork::TSequenceEntry;
using STI::Engine::SequenceEntry;
using STI::Engine::ParsedVar;
using STI::TNetwork::TParsedVar;
using STI::Engine::ShotID;
using STI::TNetwork::TShotID;
using STI::Engine::EngineJobStatus;
using STI::TNetwork::TEngineJobStatus;


//SequenceResult
template<>
bool STI::Network::convert<TSequenceResult, std::shared_ptr<SequenceResult>>(
        const TSequenceResult& tSequenceResult, std::shared_ptr<SequenceResult>& sequenceResult)
{
    std::shared_ptr<Sequence> sequence;

    convert<TSequence, std::shared_ptr<Sequence>>(tSequenceResult.sequenceData, sequence);

    sequenceResult = std::make_shared<SequenceResult>(
        convert<TSequenceID, SequenceID>(tSequenceResult.seqid), 
        sequence);

    for (unsigned i = 0; i < tSequenceResult.shots.length(); ++i) {
        auto seqIndex = convert<TSequenceIndex, SequenceIndex>(tSequenceResult.shots[i].index);
        sequenceResult->shots[seqIndex] = convert<TShotID, ShotID>(tSequenceResult.shots[i].sid);
    }

    for (unsigned i = 0; i < tSequenceResult.status.length(); ++i) {
        auto seqIndex = convert<TSequenceIndex, SequenceIndex>(tSequenceResult.status[i].index);
        sequenceResult->status[seqIndex] = convert<TEngineJobStatus, EngineJobStatus>(tSequenceResult.status[i].status);
    }

    return (sequenceResult != 0);
}

template<>
bool STI::Network::convert<std::shared_ptr<SequenceResult>, TSequenceResult>(
        const std::shared_ptr<SequenceResult>& sequenceResult, TSequenceResult& tSequenceResult)
{
    if (sequenceResult == 0) return false;

    bool success = convert<SequenceID, TSequenceID>(sequenceResult->seqid, tSequenceResult.seqid);

    if (sequenceResult->sequence != 0) {
        success &= convert<std::shared_ptr<Sequence>, TSequence>(sequenceResult->sequence, tSequenceResult.sequenceData);
    }

    tSequenceResult.shots.length(sequenceResult->shots.size());
    unsigned i = 0;
    for (auto& tuple : sequenceResult->shots) {
        tSequenceResult.shots[i].index = convert<SequenceIndex, TSequenceIndex>(tuple.first);
        tSequenceResult.shots[i].sid = convert<ShotID, TShotID>(tuple.second);
        i++;
    }

    tSequenceResult.status.length(sequenceResult->status.size());
    unsigned j = 0;
    for (auto& tuple : sequenceResult->status) {
        tSequenceResult.status[j].index = convert<SequenceIndex, TSequenceIndex>(tuple.first);
        tSequenceResult.status[j].status = convert<EngineJobStatus, TEngineJobStatus>(tuple.second);
        j++;
    }

    return success;
}


//SequenceID
template<>
SequenceID STI::Network::convert<TSequenceID, SequenceID>(const TSequenceID& tSequenceID)
{
    SequenceID sequenceID;
    convert<TSequenceID, SequenceID>(tSequenceID, sequenceID);
    return sequenceID;
}

template<>
TSequenceID STI::Network::convert<SequenceID, TSequenceID>(const SequenceID& sequenceID)
{
    TSequenceID tSequenceID;
    convert<SequenceID, TSequenceID>(sequenceID, tSequenceID);
    return tSequenceID;
}

template<>
bool STI::Network::convert<TSequenceID, SequenceID>(const TSequenceID& tSequenceID, SequenceID& sequenceID)
{
    sequenceID.timestamp = convert<TTimeStamp, TimeStamp>(tSequenceID.timestamp);
    sequenceID.jobSourceID = convert<TEngineJobSourceID, EngineJobSourceID>(tSequenceID.jobSourceID);
    return true;
}

template<>
bool STI::Network::convert<SequenceID, TSequenceID>(const SequenceID& sequenceID, TSequenceID& tSequenceID)
{
    tSequenceID.timestamp = convert<TimeStamp, TTimeStamp>(sequenceID.timestamp);
    tSequenceID.jobSourceID = convert<EngineJobSourceID, TEngineJobSourceID>(sequenceID.jobSourceID);
    return true;
}


//SequenceEntryID
template<>
SequenceEntryID STI::Network::convert<TSequenceEntryID, SequenceEntryID>(const TSequenceEntryID& tSequenceEntryID)
{
    SequenceEntryID sequenceEntryID;

    STI::Network::convert<TSequenceID, SequenceID>(tSequenceEntryID.seqID, sequenceEntryID.seqID);
    sequenceEntryID.seqIndex = convert<TSequenceIndex, SequenceIndex>(tSequenceEntryID.seqIndex);

    return sequenceEntryID;
}

template<>
TSequenceEntryID STI::Network::convert<SequenceEntryID, TSequenceEntryID>(const SequenceEntryID& sequenceEntryID)
{
    TSequenceEntryID tSequenceEntryID;

    STI::Network::convert<SequenceID, TSequenceID>(sequenceEntryID.seqID, tSequenceEntryID.seqID);
    tSequenceEntryID.seqIndex = convert<SequenceIndex, TSequenceIndex>(sequenceEntryID.seqIndex);

    return tSequenceEntryID;
}



//SequenceType
template<>
SequenceType STI::Network::convert<TSequenceType, SequenceType>(const TSequenceType& tSequenceType)
{
    //{ SequenceTypeOpen, SequenceTypeClosed };

    SequenceType type;

    switch(tSequenceType) {
        case TSequenceType::SequenceTypeOpen:
            type = SequenceType::Open;
            break;
        case TSequenceType::SequenceTypeClosed:
            type = SequenceType::Closed;
            break;
        default:
            type = SequenceType::Open;
            break;
    }
    return type;
}

template<>
TSequenceType STI::Network::convert<SequenceType, TSequenceType>(const SequenceType& sequenceType)
{
    //{ Open, Closed }

    TSequenceType tType;

    switch(sequenceType) {
        case SequenceType::Open:
            tType = TSequenceType::SequenceTypeOpen;
            break;
        case SequenceType::Closed:
            tType = TSequenceType::SequenceTypeClosed;
            break;
        default:
            tType = TSequenceType::SequenceTypeOpen;
            break;
    }
    return tType;
}


//SequenceEntry
template<>
bool STI::Network::convert<TSequenceEntry, SequenceEntry>(const TSequenceEntry& tSequenceEntry, SequenceEntry& sequenceEntry)
{
    // sequenceEntry.index = static_cast<int>(tSequenceEntry.index);
    
    convert<TSequenceIndex, SequenceIndex>(tSequenceEntry.index, sequenceEntry.index);
    bool success = convert<TParsedVar, ParsedVar>(tSequenceEntry.overwritten, sequenceEntry.overwritten);
    return success;
}

template<>
bool STI::Network::convert<SequenceEntry, TSequenceEntry>(const SequenceEntry& sequenceEntry, TSequenceEntry& tSequenceEntry)
{
    // tSequenceEntry.index = static_cast<CORBA::Long>(sequenceEntry.index);
    convert<SequenceIndex, TSequenceIndex>(sequenceEntry.index, tSequenceEntry.index);
    bool success = convert<ParsedVar, TParsedVar>(sequenceEntry.overwritten, tSequenceEntry.overwritten);
    return success;
}


//Sequence
template<>
bool STI::Network::convert<TSequence, std::shared_ptr<Sequence>>(const TSequence& tSequence, std::shared_ptr<Sequence>& sequence)
{
    sequence = std::make_shared<Sequence>(convert<TSequenceType, SequenceType>(tSequence.type));

    sequence->repeats = static_cast<unsigned>(tSequence.repeats);

    auto len = tSequence.sequenceTable.length();

    for (unsigned i = 0; i < len; ++i) {
        // unsigned index = static_cast<unsigned>(tSequence.sequenceTable[i].index);
        SequenceIndex index = convert<TSequenceIndex, SequenceIndex>(tSequence.sequenceTable[i].index);
        convert<TSequenceEntry, SequenceEntry>(tSequence.sequenceTable[i].entry, sequence->sequenceTable[index]);
    }

    return true;
}

template<>
bool STI::Network::convert<std::shared_ptr<Sequence>, TSequence>(const std::shared_ptr<Sequence>& sequence, TSequence& tSequence)
{
    if (sequence == 0) return false;

    tSequence.type = convert<SequenceType, TSequenceType>(sequence->type);
    tSequence.repeats = static_cast<CORBA::Long>(sequence->repeats);
    
    tSequence.sequenceTable.length(sequence->sequenceTable.size());

    unsigned i = 0;
    for (auto& tuple : sequence->sequenceTable) {
        // tSequence.sequenceTable[i].index = static_cast<CORBA::Long>(tuple.first);
        tSequence.sequenceTable[i].index = convert<SequenceIndex, TSequenceIndex>(tuple.first);
        convert<SequenceEntry, TSequenceEntry>(tuple.second, tSequence.sequenceTable[i].entry);
        i++;
    }

    return true;
}


//SequenceIndex
template<>
SequenceIndex STI::Network::convert<TSequenceIndex, SequenceIndex>(const TSequenceIndex& tSequenceIndex)
{
    SequenceIndex sequenceIndex;

    sequenceIndex.index = static_cast<int>(tSequenceIndex.index);
    sequenceIndex.repeat = static_cast<int>(tSequenceIndex.repeat);

    return sequenceIndex;
}

template<>
TSequenceIndex STI::Network::convert<SequenceIndex, TSequenceIndex>(const SequenceIndex& sequenceIndex)
{
    TSequenceIndex tSequenceIndex;

    tSequenceIndex.index = static_cast<CORBA::Long>(sequenceIndex.index);
    tSequenceIndex.repeat = static_cast<CORBA::Long>(sequenceIndex.repeat);

    return tSequenceIndex;
}

template<>
bool STI::Network::convert<TSequenceIndex, SequenceIndex>(
        const TSequenceIndex& tSequenceIndex, SequenceIndex& sequenceIndex)
{
    sequenceIndex.index = static_cast<int>(tSequenceIndex.index);
    sequenceIndex.repeat = static_cast<int>(tSequenceIndex.repeat);
    return true;
}

template<>
bool STI::Network::convert<SequenceIndex, TSequenceIndex>(
        const SequenceIndex& sequenceIndex, TSequenceIndex& tSequenceIndex)
{
    tSequenceIndex.index = static_cast<CORBA::Long>(sequenceIndex.index);
    tSequenceIndex.repeat = static_cast<CORBA::Long>(sequenceIndex.repeat);
    return true;
}


