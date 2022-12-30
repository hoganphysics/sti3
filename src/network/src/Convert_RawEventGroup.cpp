#include "Convert_RawEventGroup.h"
#include "Convert_ShotResult.h"
#include "Convert_EventEngine.h"
#include "Convert_StackTrace.h"

#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventTarget.h>

#include <sti/engine/RawEventGroup.h>

#include <set>

using STI::Engine::RawEventGroup;
using STI::TNetwork::TRawEventGroup;
using STI::Engine::StackTraceData;
using STI::TNetwork::TStackTraceData;
using STI::Engine::ParsedVar;
using STI::TNetwork::TParsedVar;
using STI::Engine::ParsedTag;
using STI::TNetwork::TParsedTag;
using STI::TNetwork::TRawEvent;
using STI::Engine::RawEvent;
using STI::TNetwork::TMixedValue;
using STI::Utils::MixedValue;


//RawEventGroup
template<>
bool STI::Network::convert<std::shared_ptr<RawEventGroup>, TRawEventGroup>(
        const std::shared_ptr<RawEventGroup>& rawEventGroup, TRawEventGroup& tRawEventGroup)
{
    if (rawEventGroup == 0) return false;

    convert<std::shared_ptr<StackTraceData>, TStackTraceData>(rawEventGroup->getStackTraceData(), tRawEventGroup.stackTraceData);

    return convertGroup(rawEventGroup, tRawEventGroup);
}


template<>
bool STI::Network::convert<TRawEventGroup, std::shared_ptr<RawEventGroup>>(
        const TRawEventGroup& tRawEventGroup, std::shared_ptr<RawEventGroup>& rawEventGroup)
{
    std::shared_ptr<StackTraceData> traceData;
    convert<TStackTraceData, std::shared_ptr<StackTraceData>>(tRawEventGroup.stackTraceData, traceData);
    
    rawEventGroup = std::make_shared<RawEventGroup>(
        convert<CORBA::String_member, std::string>(tRawEventGroup.name),
        convert<CORBA::String_member, std::string>(tRawEventGroup.parentName),
        traceData
    );

    return convertGroup(tRawEventGroup, rawEventGroup);
}


bool STI::Network::convertGroup(const std::shared_ptr<RawEventGroup>& rawEventGroup, TRawEventGroup& tRawEventGroup)
{
    if (rawEventGroup == 0) return false;

    tRawEventGroup.name = convert<std::string, CORBA::String_member>(rawEventGroup->getName());
    tRawEventGroup.parentName = convert<std::string, CORBA::String_member>(rawEventGroup->getParentGroupName());

    tRawEventGroup.timeOffset = static_cast<CORBA::Double>(rawEventGroup->getTimeOffset());

    auto referencePoints = rawEventGroup->getReferencePoints();
    tRawEventGroup.referencePoints.length(referencePoints.size());
    unsigned i = 0;
    for (auto& point : referencePoints) {
        tRawEventGroup.referencePoints[i].name = convert<std::string, CORBA::String_member>(point.first);
        tRawEventGroup.referencePoints[i].time = static_cast<CORBA::Double>(point.second);
        i++;
    }

    auto events = rawEventGroup->getEvents();
    if (events != 0) {
        convert<RawEvent, TRawEvent>(*events, tRawEventGroup.events);
    }
    else {
        tRawEventGroup.events.length(0);
    }

    tRawEventGroup.metaData = convert<MixedValue, TMixedValue>(rawEventGroup->getMetaData());

    convert<ParsedTag, TParsedTag>(rawEventGroup->getTags(), tRawEventGroup.parsedTags);
    convert<ParsedVar, TParsedVar>(rawEventGroup->getVars(), tRawEventGroup.parsedVars);
    convert<ParsedVar, TParsedVar>(rawEventGroup->getOverwrittenVars(), tRawEventGroup.overwrittenVars);

    auto subGroups = rawEventGroup->getSubgroups();
    tRawEventGroup.subgroups.length(subGroups.size());
    unsigned k = 0;
    for (auto& g : subGroups) {
        convertGroup(g, tRawEventGroup.subgroups[k]);
        k++;
    }

    return true;
}


bool STI::Network::convertGroup(const TRawEventGroup& tRawEventGroup, std::shared_ptr<RawEventGroup>& rawEventGroup)
{
    if (rawEventGroup == 0) return false;

    STI::Utils::MetaData metaData(convert<TMixedValue, MixedValue>(tRawEventGroup.metaData));
    rawEventGroup->addMetaData(metaData);

    STI::Engine::RawEventVector events;
    convert<TRawEvent, RawEvent>(tRawEventGroup.events, events);

    for (auto& e : events) {
        e.setStackTraceData(rawEventGroup->getStackTraceData());
        e.setParentGroup(rawEventGroup.get());
    }

    rawEventGroup->addEvents(events);
    
    rawEventGroup->shiftStartTimeTo(static_cast<double>(tRawEventGroup.timeOffset));
    
    auto& tReferencePoints = tRawEventGroup.referencePoints;
    for (unsigned i = 0; i < tReferencePoints.length(); ++i) {
        rawEventGroup->addReferencePoint(
            convert<CORBA::String_member, std::string>(tReferencePoints[i].name),
            static_cast<double>(tReferencePoints[i].time)
        );
    }

    std::vector<ParsedVar> vars;
    convert<TParsedVar, ParsedVar>(tRawEventGroup.parsedVars, vars);

    for (auto& v : vars) {
        v.parentGroup = rawEventGroup.get();
        v.stackTraceData = rawEventGroup->getStackTraceData();
    }

    rawEventGroup->setVars(vars);

    std::vector<ParsedTag> tags;
    convert<TParsedTag, ParsedTag>(tRawEventGroup.parsedTags, tags);
    rawEventGroup->setTags(tags);

    std::set<ParsedVar> ovars;
    convert<TParsedVar, ParsedVar>(tRawEventGroup.overwrittenVars, ovars);
    rawEventGroup->bindVars(ovars);

    auto& tSubgroups = tRawEventGroup.subgroups;
    for (unsigned i = 0; i < tSubgroups.length(); ++i) {
        auto g = rawEventGroup->group(
            convert<CORBA::String_member, std::string>(tSubgroups[i].name));

            // tSubgroups[i].parentName
        
        convertGroup(tSubgroups[i], g);
    }

    return true;
}

