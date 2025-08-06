#include "Convert_ShotResult.h"
#include "Convert_EventEngine.h"
#include "Convert_Attribute.h"
#include "Convert_ResultsCollector.h"
#include "Convert_StackTrace.h"
#include "Convert_EventEngine.h"
#include "Convert_RawEventGroup.h"

#include <sti/engine/FullShotResult.h>
#include <sti/engine/ParsedVar.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/StackTraceResult.h>

#include <sti/engine/ParsedTag.h>
#include <sti/engine/RawEventGroup.h>

#include <memory>


using STI::Network::convert;

using STI::Engine::ParseID;
using STI::Engine::ShotResult;
using STI::TNetwork::TShotResult;
using STI::TNetwork::TTimeStamp;
using STI::Utils::TimeStamp;
using STI::TNetwork::TShotResultRecord;
using STI::Engine::ShotResultRecord;
using STI::Engine::ParseResult;
using STI::TNetwork::TParseResult;
using STI::Engine::RawEventGroup;
using STI::TNetwork::TRawEventGroup;
using STI::Engine::ParsedVar;
using STI::TNetwork::TParsedVar;
using STI::Engine::ParsedTag;
using STI::TNetwork::TParsedTag;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;
using STI::Engine::CompressedStackTrace;
using STI::TNetwork::TStackFrameSeq;
using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValue;
using STI::TNetwork::TEventEngineDependencyTree;
using STI::Engine::ParsedDependencyTree;
using STI::TNetwork::TEngineParsingMessage;
using STI::Engine::EngineParsingMessage;
using STI::TNetwork::TStackTraceResult;
using STI::Engine::StackTraceResult;
using STI::TNetwork::TFullShotResult;
using STI::Engine::FullShotResult;
using STI::Engine::ShotID;
using STI::TNetwork::TShotID;
using STI::TNetwork::TDeviceID;
using STI::Device::DeviceID;
using STI::Engine::MeasurementVector;
using STI::Engine::MeasurementMap;
using STI::Engine::Measurement;
using STI::Engine::ShotConfig;


//ShotResult
template<>
bool STI::Network::convert<TShotResult, std::shared_ptr<ShotResult>>(
        const TShotResult& tShotResult, std::shared_ptr<ShotResult>& shotResult)
{
    shotResult = std::make_shared<ShotResult>();

    shotResult->sid = convert<TShotID, ShotID>(tShotResult.sid);
    shotResult->playTime = convert<TTimeStamp, TimeStamp>(tShotResult.playTime);

    // shotResult->measurements = std::make_shared<STI::Engine::MeasurementVector>();
    // convert<STI::TNetwork::TMeasurement, std::shared_ptr<STI::Engine::Measurement>>(tShotResult.measurements, *(shotResult->measurements));


    // shotResult->measurements = std::make_shared<MeasurementMap>();
    // for (unsigned i = 0; i < tShotResult.measurements.length(); ++i) {
    //     // convert<::STI::TNetwork::TMeasurementSeq, std::shared_ptr<MeasurementVector>>
    //     auto newMeasurements = std::make_shared<MeasurementVector>();
    //     (*shotResult->measurements)[convert<TDeviceID, DeviceID>(tShotResult.measurements[i].id)] = newMeasurements;

    //     convert<::STI::TNetwork::TMeasurement, std::shared_ptr<Measurement>>(
    //             tShotResult.measurements[i].measurements,
    //             *newMeasurements
    //         );
    // }

    convert<::STI::TNetwork::TDeviceIDMeasurementsTupleSeq, std::shared_ptr<MeasurementMap>>(
        tShotResult.measurements, shotResult->measurements);

    for (unsigned i = 0; i < tShotResult.attributes.length(); ++i) {

        convert<::STI::TNetwork::TAttributeTupleSeq, std::map<std::string, std::string>>(
                tShotResult.attributes[i].attributes,
                (shotResult->attributes)[convert<TDeviceID, DeviceID>(tShotResult.attributes[i].id)]
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

    tShotResult.sid = convert<ShotID, TShotID>(shotResult->sid);
    tShotResult.playTime = convert<TimeStamp, TTimeStamp>(shotResult->playTime);

    // if (shotResult->measurements != 0) {
    //     convert<std::shared_ptr<STI::Engine::Measurement>, STI::TNetwork::TMeasurement>(*(shotResult->measurements), tShotResult.measurements);        
    // }


    // if (shotResult->measurements != 0) {
    //     tShotResult.measurements.length( shotResult->measurements->size() );
    //     unsigned i = 0;
    //     for (auto& tuple : *shotResult->measurements) { //tuple: {DeviceID, shared_ptr<MeasurementVector>}
    //         tShotResult.measurements[i].id = convert<DeviceID, TDeviceID>(tuple.first);

    //         if (tuple.second != 0) {
    //             convert<std::shared_ptr<STI::Engine::Measurement>, STI::TNetwork::TMeasurement>(*(tuple.second), tShotResult.measurements[i].measurements);
    //         }
    //         else {
    //             tShotResult.measurements[i].measurements.length(0);
    //         }
    //     }
    // }
    // else {
    //     tShotResult.measurements.length(0);
    // }

    convert<std::shared_ptr<MeasurementMap>, STI::TNetwork::TDeviceIDMeasurementsTupleSeq>(
        shotResult->measurements, tShotResult.measurements);




    tShotResult.attributes.length( shotResult->attributes.size() );
    unsigned i = 0;
    for (auto& deviceAttributes : shotResult->attributes) {
        
        tShotResult.attributes[i].id = convert<DeviceID, TDeviceID>(deviceAttributes.first);

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
    convert<STI::TNetwork::TParseID, ParseID>(tParseResult.parseID, parseResult.pid);
    parseResult.shotConfig = convert<STI::TNetwork::TShotConfig, ShotConfig>(tParseResult.shotConfig);
    convert<TRawEventGroup, std::shared_ptr<STI::Engine::RawEventGroup>>(tParseResult.baseEventGroup, parseResult.baseEventGroup);
    convert<TEventEngineDependencyTree, std::shared_ptr<ParsedDependencyTree>>(tParseResult.parsedDevices, parseResult.parsedDevices);
    convert<TEngineParsingMessage, EngineParsingMessage>(tParseResult.messages, parseResult.messages);
    convert<TStackTraceResult, std::shared_ptr<StackTraceResult>>(tParseResult.stackTraceResult, parseResult.stackTraceResult);

    return true;
}

template<>
bool STI::Network::convert<ParseResult, TParseResult>(
        const ParseResult& parseResult, TParseResult& tParseResult)
{

    convert<ParseID, STI::TNetwork::TParseID>(parseResult.pid, tParseResult.parseID);
    tParseResult.shotConfig = convert<ShotConfig, STI::TNetwork::TShotConfig>(parseResult.shotConfig);
    convert<std::shared_ptr<STI::Engine::RawEventGroup>, TRawEventGroup>(parseResult.baseEventGroup, tParseResult.baseEventGroup);
    convert<std::shared_ptr<ParsedDependencyTree>, TEventEngineDependencyTree>(parseResult.parsedDevices, tParseResult.parsedDevices);
    convert<EngineParsingMessage, TEngineParsingMessage>(parseResult.messages, tParseResult.messages);
    convert<std::shared_ptr<StackTraceResult>, TStackTraceResult>(parseResult.stackTraceResult, tParseResult.stackTraceResult);

    return true;
}


template<>
bool STI::Network::convert<TParseResult, std::shared_ptr<ParseResult>>(
        const TParseResult& tParseResult, std::shared_ptr<ParseResult>& parseResult)
{
    parseResult = std::make_shared<ParseResult>();
    return convert<TParseResult, ParseResult>(tParseResult, *parseResult);
}

template<>
bool STI::Network::convert<std::shared_ptr<ParseResult>, TParseResult>(
        const std::shared_ptr<ParseResult>& parseResult, TParseResult& tParseResult)
{
    if (parseResult == 0) return false;
    return convert<ParseResult, TParseResult>(*parseResult, tParseResult);
}


//FullShotResult
template<>
bool STI::Network::convert<TFullShotResult, std::shared_ptr<FullShotResult>>(
        const TFullShotResult& tFullShotResult, std::shared_ptr<FullShotResult>& fullShotResult)
{
    fullShotResult = std::make_shared<FullShotResult>();

    convert<TParseResult, std::shared_ptr<ParseResult>>(tFullShotResult.parseResult, fullShotResult->parseResult);
    convert<TShotResult, std::shared_ptr<ShotResult>>(tFullShotResult.shotResult, fullShotResult->shotResult);

    return true;
}

template<>
bool STI::Network::convert<std::shared_ptr<FullShotResult>, TFullShotResult>(
        const std::shared_ptr<FullShotResult>& fullShotResult, TFullShotResult& tFullShotResult)
{
    if (fullShotResult == 0) return false;

    convert<std::shared_ptr<ParseResult>, TParseResult>(fullShotResult->parseResult, tFullShotResult.parseResult);
    convert<std::shared_ptr<ShotResult>, TShotResult>(fullShotResult->shotResult, tFullShotResult.shotResult);

    return true;
}



//ParsedVar
template<>
bool STI::Network::convert<TParsedVar, ParsedVar>(const TParsedVar& tParsedVar, ParsedVar& parsedVar)
{
    parsedVar.setParentGroup(nullptr);
    parsedVar.name = convert<CORBA::String_member, std::string>(tParsedVar.name);
    parsedVar.setGroupName(convert<CORBA::String_member, std::string>(tParsedVar.groupName));
    convert<TStackFrameSeq, CompressedStackTrace>(tParsedVar.trace, parsedVar.trace);
    convert<TMixedValue, MixedValue>(tParsedVar.value, parsedVar.value);

    return true;
}

template<>
bool STI::Network::convert<ParsedVar, TParsedVar>(const ParsedVar& parsedVar, TParsedVar& tParsedVar)
{
    convert<std::string, CORBA::String_member>(parsedVar.name, tParsedVar.name);
    convert<std::string, CORBA::String_member>(parsedVar.getGroupName(), tParsedVar.groupName);
    convert<CompressedStackTrace, TStackFrameSeq>(parsedVar.trace, tParsedVar.trace);
    convert<MixedValue, TMixedValue>(parsedVar.value, tParsedVar.value);

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
    parsedTag.setParentGroup(nullptr);
    parsedTag.name = convert<CORBA::String_member, std::string>(tParsedTag.name);
    parsedTag.setGroupName(convert<CORBA::String_member, std::string>(tParsedTag.groupName));
    convert<TStackFrameSeq, CompressedStackTrace>(tParsedTag.trace, parsedTag.trace);

    return true;
}

template<>
bool STI::Network::convert<ParsedTag, TParsedTag>(const ParsedTag& parsedTag, TParsedTag& tParsedTag)
{
    convert<std::string, CORBA::String_member>(parsedTag.name, tParsedTag.name);
    convert<std::string, CORBA::String_member>(parsedTag.getGroupName(), tParsedTag.groupName);
    convert<CompressedStackTrace, TStackFrameSeq>(parsedTag.trace, tParsedTag.trace);

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

