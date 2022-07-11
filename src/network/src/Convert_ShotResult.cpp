
#include "Convert_ShotResult.h"
#include "Convert_EventEngine.h"
#include "Convert_Attribute.h"
#include "Convert_ResultsCollector.h"
#include "Convert_StackTrace.h"
#include "Convert_EventEngine.h"

#include "ShotResult.h"
#include <sti/engine/RawEvent.h>

#include "RawEventGroup.h"
#include "ParsedVar.h"
#include "ParsedTag.h"

#include <memory>


using STI::Network::convert;

using STI::Engine::ShotResult;
using STI::TNetwork::TShotResult;
using STI::TNetwork::TTimeStamp;
using STI::Engine::TimeStamp;
using STI::TNetwork::TShotResultRecord;
using STI::Engine::ShotResultRecord;
using STI::Engine::ParseResult;
using STI::TNetwork::TParseResult;
using STI::Engine::EngineParseResult;
using STI::TNetwork::TEngineParseResult;
using STI::Engine::RawEventGroup;
using STI::TNetwork::TRawEventGroup;
using STI::Engine::ParsedVar;
using STI::TNetwork::TParsedVar;
using STI::Engine::ParsedTag;
using STI::TNetwork::TParsedTag;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;
using STI::Engine::StackTrace;
using STI::TNetwork::TStackFrameSeq;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValue;
using STI::TNetwork::TEventEngineDependencyTree;
using STI::Engine::ParsedDependencyTree;
using STI::TNetwork::TEngineParsingMessage;
using STI::Engine::EngineParsingMessage;



//ShotResult
template<>
bool STI::Network::convert<TShotResult, std::shared_ptr<ShotResult>>(
        const TShotResult& tShotResult, std::shared_ptr<ShotResult>& shotResult)
{
    shotResult = std::make_shared<ShotResult>();

    shotResult->sid = convert<TNetwork::TShotID, Engine::ShotID>(tShotResult.sid);
    shotResult->playTime = convert<TTimeStamp, TimeStamp>(tShotResult.playTime);

    convert<::STI::TNetwork::TParseResult, STI::Engine::ParseResult>(tShotResult.parseResult, shotResult->parseResult);
    convert<::STI::TNetwork::TEngineParseResult, STI::Engine::EngineParseResult>(tShotResult.engineParseResult, shotResult->engineParseResult);

    shotResult->measurements = std::make_shared<STI::Engine::MeasurementVector>();
    convert<STI::TNetwork::TMeasurement, std::shared_ptr<STI::Engine::Measurement>>(tShotResult.measurements, *(shotResult->measurements));

    for (unsigned i = 0; i < tShotResult.attributes.length(); ++i) {

        convert<::STI::TNetwork::TAttributeTupleSeq, std::map<std::string, std::string>>(
                tShotResult.attributes[i].attributes,
                (shotResult->attributes)[convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tShotResult.attributes[i].id)]
            );
    }

    convert<TShotResultRecord, ShotResultRecord>(tShotResult.shotResultRecord, shotResult->shotResultRecord);

    return true;
}

template<>
bool STI::Network::convert<std::shared_ptr<ShotResult>, TShotResult>(
        const std::shared_ptr<ShotResult>& shotResult, TShotResult& tShotResult)
{
    if (shotResult == 0) return false;

    tShotResult.sid = convert<Engine::ShotID, TNetwork::TShotID>(shotResult->sid);
    tShotResult.playTime = convert<TimeStamp, TTimeStamp>(shotResult->playTime);

    if (shotResult->measurements != 0) {
        convert<std::shared_ptr<STI::Engine::Measurement>, STI::TNetwork::TMeasurement>(*(shotResult->measurements), tShotResult.measurements);        
    }

    tShotResult.attributes.length( shotResult->attributes.size() );
    unsigned i = 0;
    for (auto& deviceAttributes : shotResult->attributes) {
        
        tShotResult.attributes[i].id = convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(deviceAttributes.first);

        convert<std::map<std::string, std::string>, ::STI::TNetwork::TAttributeTupleSeq>(
                deviceAttributes.second, 
                tShotResult.attributes[i].attributes);
        i++;
    }

    convert<ShotResultRecord, TShotResultRecord>(shotResult->shotResultRecord, tShotResult.shotResultRecord);

    return true;
}


//ParseResult
template<>
bool STI::Network::convert<TParseResult, ParseResult>(
        const TParseResult& tParseResult, ParseResult& parseResult)
{
    convert<STI::TNetwork::TFileHolderSeq, std::vector<std::shared_ptr<STI::Utils::FileHolder>>>(tParseResult.timingFiles, parseResult.timingFiles);
    
    convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tParseResult.timingFileNames, parseResult.timingFileNames);
    convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tParseResult.functionNames, parseResult.functionNames);

    convert<TRawEventGroup, STI::Engine::RawEventGroup>(tParseResult.groups, parseResult.eventGroups);
    convert<TParsedVar, ParsedVar>(tParseResult.parsedVars, parseResult.parsedVars);
    convert<TParsedTag, ParsedTag>(tParseResult.parsedTags, parseResult.parsedTags);

    return true;
}

template<>
bool STI::Network::convert<ParseResult, TParseResult>(
        const ParseResult& parseResult, TParseResult& tParseResult)
{
    convert<std::vector<std::shared_ptr<STI::Utils::FileHolder>>, STI::TNetwork::TFileHolderSeq>(parseResult.timingFiles, tParseResult.timingFiles);

    convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(parseResult.timingFileNames, tParseResult.timingFileNames);
    convert<std::vector<std::string>, STI::TNetwork::TStringSeq>(parseResult.functionNames, tParseResult.functionNames);

    convert<STI::Engine::RawEventGroup, TRawEventGroup>(parseResult.eventGroups, tParseResult.groups);
    convert<ParsedVar, TParsedVar>(parseResult.parsedVars, tParseResult.parsedVars);
    convert<ParsedTag, TParsedTag>(parseResult.parsedTags, tParseResult.parsedTags);

    return true;
}


//EngineParseResult
template<>
bool STI::Network::convert<TEngineParseResult, EngineParseResult>(
        const TEngineParseResult& tEngineParseResult, EngineParseResult& engineParseResult)
{
    convert<::STI::TNetwork::TDeviceEventsSeq, STI::Engine::DeviceEventMap>(tEngineParseResult.parsedEvents, engineParseResult.parsedEvents);
    convert<TEventEngineDependencyTree, std::shared_ptr<ParsedDependencyTree>>(tEngineParseResult.dependencies, engineParseResult.parsedDevices);
    convert<TEngineParsingMessage, EngineParsingMessage>(tEngineParseResult.messages, engineParseResult.messages);

    return true;
}

template<>
bool STI::Network::convert<EngineParseResult, TEngineParseResult>(
        const EngineParseResult& engineParseResult, TEngineParseResult& tEngineParseResult)
{
    convert<STI::Engine::DeviceEventMap, ::STI::TNetwork::TDeviceEventsSeq>(engineParseResult.parsedEvents, tEngineParseResult.parsedEvents);
    convert<std::shared_ptr<ParsedDependencyTree>, TEventEngineDependencyTree>(engineParseResult.parsedDevices, tEngineParseResult.dependencies);
    convert<EngineParsingMessage, TEngineParsingMessage>(engineParseResult.messages, tEngineParseResult.messages);

    return true;
}


//RawEventGroup
template<>
bool STI::Network::convert<TRawEventGroup, RawEventGroup>(
        const TRawEventGroup& tRawEventGroup, RawEventGroup& rawEventGroup)
{
    STI::Utils::GraphPathLabel groupIndex;
    convertEventGraphPath(tRawEventGroup.groupIndex, groupIndex);

    rawEventGroup = RawEventGroup(
        convert<CORBA::String_member, std::string>(tRawEventGroup.name),
        groupIndex
    );

    rawEventGroup.setStartTime( static_cast<double>(tRawEventGroup.startTime) );
    rawEventGroup.setEndTime( static_cast<double>(tRawEventGroup.endTime) );

    return true;
}

template<>
bool STI::Network::convert<RawEventGroup, TRawEventGroup>(
        const RawEventGroup& rawEventGroup, TRawEventGroup& tRawEventGroup)
{
    convert<std::string, CORBA::String_member>(rawEventGroup.getName(), tRawEventGroup.name);
    tRawEventGroup.startTime = static_cast<CORBA::Double>(rawEventGroup.startTime());
    tRawEventGroup.endTime = static_cast<CORBA::Double>(rawEventGroup.endTime());

    convertEventGraphPath(rawEventGroup.getFullIndex(), tRawEventGroup.groupIndex);

    return true;
}

template<>
RawEventGroup STI::Network::convert<TRawEventGroup, RawEventGroup>(const TRawEventGroup& tRawEventGroup)
{
    RawEventGroup rawEventGroup;
    convert<TRawEventGroup, RawEventGroup>(tRawEventGroup, rawEventGroup);
    return rawEventGroup;
}

template<>
TRawEventGroup STI::Network::convert<RawEventGroup, TRawEventGroup>(const RawEventGroup& rawEventGroup)
{
    TRawEventGroup tRawEventGroup;
    convert<RawEventGroup, TRawEventGroup>(rawEventGroup, tRawEventGroup);
    return tRawEventGroup;
}

template<>
std::shared_ptr<RawEventGroup> STI::Network::convert<TRawEventGroup, std::shared_ptr<RawEventGroup>>(
        const TNetwork::TRawEventGroup& tRawEventGroup)
{
    auto rawEventGroup = std::make_shared<RawEventGroup>();
    convert<TRawEventGroup, RawEventGroup>(tRawEventGroup, *rawEventGroup);
    return rawEventGroup;
}

template<>
TRawEventGroup STI::Network::convert<std::shared_ptr<RawEventGroup>, TRawEventGroup>(
        const std::shared_ptr<RawEventGroup>& rawEventGroup)
{
    if (rawEventGroup != 0) {
        return convert<RawEventGroup, TRawEventGroup>(*rawEventGroup);
    }
    
    TRawEventGroup tRawEventGroup;
    return tRawEventGroup;
}

template<>
bool STI::Network::convert<std::shared_ptr<RawEventGroup>, TRawEventGroup>(
        const std::shared_ptr<RawEventGroup>& rawEventGroup, TRawEventGroup& tRawEventGroup)
{
    if (rawEventGroup != 0) {
        return convert<RawEventGroup, TRawEventGroup>(*rawEventGroup, tRawEventGroup);
    }
    return false;
}


//ParsedVar
template<>
bool STI::Network::convert<TParsedVar, ParsedVar>(const TParsedVar& tParsedVar, ParsedVar& parsedVar)
{
    parsedVar.name = convert<CORBA::String_member, std::string>(tParsedVar.name);
    // convert<TDeviceID, DeviceID>(tParsedVar.targetServerID, parsedVar.targetServerID);
    convert<TStackFrameSeq, StackTrace>(tParsedVar.trace, parsedVar.trace);
    convert<TMixedValue, MixedValue>(tParsedVar.value, parsedVar.value);
    convert<TRawEventGroup, RawEventGroup>(tParsedVar.scope, parsedVar.scope);

    return true;
}

template<>
bool STI::Network::convert<ParsedVar, TParsedVar>(const ParsedVar& parsedVar, TParsedVar& tParsedVar)
{
    convert<std::string, CORBA::String_member>(parsedVar.name, tParsedVar.name);
    // convert<DeviceID, TDeviceID>(parsedVar.targetServerID, tParsedVar.targetServerID);
    convert<StackTrace, TStackFrameSeq>(parsedVar.trace, tParsedVar.trace);
    convert<MixedValue, TMixedValue>(parsedVar.value, tParsedVar.value);
    convert<RawEventGroup, TRawEventGroup>(parsedVar.scope, tParsedVar.scope);

    return true;
}

template<>
ParsedVar STI::Network::convert<TParsedVar, ParsedVar>(const TParsedVar& tParsedVar)
{
    ParsedVar parsedVar;
    convert<TParsedVar, ParsedVar>(tParsedVar, parsedVar);
    return parsedVar;
}

template<>
TParsedVar STI::Network::convert<ParsedVar, TParsedVar>(const ParsedVar& parsedVar)
{
    TParsedVar tParsedVar;
    convert<ParsedVar, TParsedVar>(parsedVar, tParsedVar);
    return tParsedVar;
}




//ParsedTag
template<>
bool STI::Network::convert<TParsedTag, ParsedTag>(const TParsedTag& tParsedTag, ParsedTag& parsedTag)
{
    parsedTag.name = convert<CORBA::String_member, std::string>(tParsedTag.name);
    // convert<TDeviceID, DeviceID>(tParsedTag.targetServerID, parsedTag.targetServerID);
    convert<TStackFrameSeq, StackTrace>(tParsedTag.trace, parsedTag.trace);
    convert<TRawEventGroup, RawEventGroup>(tParsedTag.scope, parsedTag.scope);

    return true;
}

template<>
bool STI::Network::convert<ParsedTag, TParsedTag>(const ParsedTag& parsedTag, TParsedTag& tParsedTag)
{
    convert<std::string, CORBA::String_member>(parsedTag.name, tParsedTag.name);
    // convert<DeviceID, TDeviceID>(parsedTag.targetServerID, tParsedTag.targetServerID);
    convert<StackTrace, TStackFrameSeq>(parsedTag.trace, tParsedTag.trace);
    convert<RawEventGroup, TRawEventGroup>(parsedTag.scope, tParsedTag.scope);

    return true;
}

template<>
ParsedTag STI::Network::convert<TParsedTag, ParsedTag>(const TParsedTag& tParsedTag)
{
    ParsedTag parsedTag;
    convert<TParsedTag, ParsedTag>(tParsedTag, parsedTag);
    return parsedTag;
}

template<>
TParsedTag STI::Network::convert<ParsedTag, TParsedTag>(const ParsedTag& parsedTag)
{
    TParsedTag tParsedTag;
    convert<ParsedTag, TParsedTag>(parsedTag, tParsedTag);
    return tParsedTag;
}

