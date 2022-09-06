#include "RawEventGroup.h"

#include <sti/engine/ParsedVar.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/RawEventTargetDevice.h>

#include <sti/utils/utils.h>

#include "ParsedTag.h"
#include "StackTraceData.h"

#include <algorithm>
#include <sstream>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/map.hpp>

using STI::Engine::RawEventGroup;
using STI::Engine::RawEvent;
using STI::Engine::RawEventVector;
using STI::Engine::ParsedVar;
using STI::Engine::ParsedTag;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventType;
using STI::Engine::RawStackTrace;
using STI::Engine::StackTraceData;


RawEventGroup::RawEventGroup()
: RawEventGroup("", "")
{
}

RawEventGroup::RawEventGroup(const std::string& name, const std::string& parentName)
: RawEventGroup(name, parentName, std::make_shared<StackTraceData>())
{
}

RawEventGroup::RawEventGroup(const std::string& name, const std::string& parentName, 
                             const std::shared_ptr<StackTraceData>& traceData)
: name(name), parentName(parentName), timeOffset(0), timeMin(0), timeMax(0), eventNumber(0), stackTraceData(traceData), 
groupMap(subgroups), varMap(parsedVars), tagMap(parsedTags)
{
    events = std::make_shared<RawEventVector>();
    
    if (stackTraceData == 0) {
        stackTraceData = std::make_shared<StackTraceData>();
    }
}

bool RawEventGroup::operator==(const RawEventGroup& other) const
{
    std::unique_lock groupLock(groupMutex);
    return getName() == other.getName();
}

bool RawEventGroup::operator<(const RawEventGroup& other) const
{
    std::unique_lock groupLock(groupMutex);
    return getName() < other.getName();
}


std::string RawEventGroup::getName() const
{
    return name;
}

std::string RawEventGroup::getFullName() const
{
    return parentName + "/" + name;
}

std::string RawEventGroup::getParentGroupName() const
{
    return parentName;
}

void RawEventGroup::setName(const std::string& newName)
{
    name = newName;
}

double RawEventGroup::startTime() const
{
    std::unique_lock groupLock(groupMutex);
    return timeMin + timeOffset;
}

double RawEventGroup::endTime() const
{
    std::unique_lock groupLock(groupMutex);
    return timeMax + timeOffset;
}


bool RawEventGroup::splitFullGroupName(const std::string& fullName, std::string& groupName, std::string& leafName)
{
    auto pos = fullName.find_last_of("/");

    if (pos != std::string::npos) {
        //fullName includes a group prefix
        groupName = fullName.substr(0, pos);
        leafName = fullName.substr(pos + 1, std::string::npos);

        return true;
    }
    return false;
}

//checks overwritten list and uses the overwritten value if found; fails if already bound and not in overwritten (cannnot call setvar twice)
bool RawEventGroup::addvar(const std::string& fullVarName, const STI::Utils::MixedValue& value, const RawStackTrace& stackTrace)
{
    std::unique_lock groupLock(groupMutex);

    if (fullVarName == "") return false;

    std::string groupName;
    std::string varName;

    if (splitFullGroupName(fullVarName, groupName, varName)) {
        //fullVarName includes a group prefix
        //need to check if it refers to this group
        auto g = group(groupName);
        return (g != 0 && g->addvar(varName, value, stackTrace));
    }

    //No group prefix found
    groupName = getName();  //belongs to local group
    varName = fullVarName;


    if (varMap.exists(varName)) {
        //Error, var already defined
        return false;
    }

    auto trace = stackTraceData->addStackTrace(stackTrace);

    ParsedVar var(varName, this, value, trace, stackTraceData);

    //Overwrite ParsedVar value if in overwrittenVars
    auto it = overwrittenVars.find(var);
    if (it != overwrittenVars.end()) {
        var.value = it->value;
    }

    varMap.add(varName, var);

    return varMap.exists(varName);
}


bool RawEventGroup::addtag(const std::string& fullTagName, const RawStackTrace& stackTrace)
{
    std::unique_lock groupLock(groupMutex);

    std::string groupName;
    std::string tagName;

    if (splitFullGroupName(fullTagName, groupName, tagName)) {
        //fullTagName includes a group prefix
        auto g = group(groupName);
        return (g != 0 && g->addtag(tagName, stackTrace));
    }

    //No group prefix found
    groupName = getName();  //belongs to local group
    tagName = fullTagName;

    if (tagMap.exists(tagName)) {
        //Error, tag already defined
        return false;
    }

    ParsedTag tag;
    tag.name = tagName;
    tag.trace = stackTraceData->addStackTrace(stackTrace);

    tagMap.add(tagName, tag);

    return tagMap.exists(tagName);
}


void RawEventGroup::addEvent(const RawEvent& evt)
{
    std::unique_lock groupLock(groupMutex);

    if (events == 0) return;

    events->push_back(evt);
    events->back().setParentGroup(this);
}

void RawEventGroup::addEvent(const RawEvent& evt, const std::string& subgroupName)
{
    std::unique_lock groupLock(groupMutex);

    auto g = group(subgroupName);
    g->addEvent(evt);
}

void RawEventGroup::addEvent(const RawEventTarget& target, double time, const STI::Utils::MixedValue& value, 
                const RawEventType& type, const RawStackTrace& stackTrace)
{
    std::unique_lock groupLock(groupMutex);

    if (events == 0 || stackTraceData == 0) return;

    auto trace = stackTraceData->addStackTrace(stackTrace);

    events->emplace_back(target, time, value, eventNumber, type, trace, stackTraceData);
    events->back().setParentGroup(this);
    eventNumber++;

    if (time < timeMin) {
        timeMin = time;
    }
    if (time > timeMax) {
        timeMax = time;
    }
}

void RawEventGroup::addEvent(const RawEventTarget& target, double time, const STI::Engine::ParsedVar& var, 
                    const RawEventType& type, const RawStackTrace& stackTrace)
{
    std::unique_lock groupLock(groupMutex);

}

ParsedVar RawEventGroup::var(const std::string& fullVarName, const RawStackTrace& stackTrace)
{
    std::unique_lock groupLock(groupMutex);

    //the value of the var, or an unbound var
    std::string groupName;
    std::string varName;

    if (splitFullGroupName(fullVarName, groupName, varName)) {
        //fullVarName includes a group prefix
        auto g = group(groupName);
        if (g != 0) {
            return g->var(varName, stackTrace);
        }
        return ParsedVar();     //error: null group
    }

    ParsedVar var;

    if(varMap.get(fullVarName, var)) {
        return var;
    }

    //create new unbound var
    var.name = fullVarName;
    var.parentGroup = this;
    if (stackTraceData != 0) {
        var.trace = stackTraceData->addStackTrace(stackTrace);
        var.stackTraceData = stackTraceData;
    }

    //Overwrite if in overwrittenVars
    auto it = overwrittenVars.find(var);
    if (it != overwrittenVars.end()) {
        var.value = it->value;
    }

    varMap.add(fullVarName, var);

    return var;
}


bool RawEventGroup::bindVars(const std::vector<ParsedVar>& overwritten)
{
//fails if it attempts to overwrite any already bound var
//sequence overwritten vars must be declared as an argument to makeshot, so that python parsing can account for them
//It's not possible to bindVars after addEvent, since in general the added events can depend on the initial bound values
    
    std::unique_lock groupLock(groupMutex);

    bool success = true;

    for (auto& ovar : overwritten) {
        ParsedVar var;
        
        if (varMap.get(ovar.name, var)) {
            if (var.isBound()) {
                success = false;
            }
            else {
                var.value = ovar.value;
                varMap.replace(ovar.name, var); //overwrite
            }
        }
    }

    overwrittenVars.insert(overwritten.begin(), overwritten.end());

    return success;
}


void RawEventGroup::bindTargets(const std::map<std::string, RawEventTarget>& targetReplacements)
{
    std::unique_lock groupLock(groupMutex);

    for (auto& t : targetReplacements) {
        targets[t.first] = t.second;
    }
}

void RawEventGroup::bindDeviceTargets(const std::map<std::string, RawEventTargetDevice>& targetDeviceReplacements)
{
    std::unique_lock groupLock(groupMutex);

    for (auto& t : targetDeviceReplacements) {
        targetDevices[t.first] = t.second;
    }
}


double RawEventGroup::getTimeOffset() const
{
    std::unique_lock groupLock(groupMutex);
    return timeOffset;
}


void RawEventGroup::shiftStartTimeTo(double time)
{
//shifts all events so that group start is at time

    std::unique_lock groupLock(groupMutex);
    timeOffset = time;
}

void RawEventGroup::shiftEndTimeTo(double time)
{
//shifts all events so that group end is at time
    // if (events == 0) return;
    // std::sort(events->begin(), events->end());

    std::unique_lock groupLock(groupMutex);
    timeOffset = time - timeMax;
}


void RawEventGroup::shiftReferenceTimeTo(const std::string& refName, double time)
{
    //shifts all events so that this reference point is at time
    double refTime;
    if (getReferencePoint(refName, refTime)) {

        std::unique_lock groupLock(groupMutex);
        timeOffset = time - refTime;
    }
}


void RawEventGroup::addReferencePoint(const std::string& refName, double time)
{
    std::unique_lock groupLock(groupMutex);

    //adds a new named reference point
    referencePoints[refName] = time;
}

bool RawEventGroup::getReferencePoint(const std::string& refName, double& time) const
{
    std::unique_lock groupLock(groupMutex);

    auto it = referencePoints.find(refName);

    if (it != referencePoints.end()) {
        time = it->second;
        return true;
    }
    return false;
}

std::map<std::string, double> RawEventGroup::getReferencePoints() const
{
    std::unique_lock groupLock(groupMutex);
    return referencePoints;
}

std::string RawEventGroup::getBaseGroup(const std::string& groupName)
{
    if (groupName == "") return groupName;

    std::vector<std::string> names;
    STI::Utils::splitString(groupName, "/", names);

    if (names.size() > 0) {
        return names.at(0);
    }
    return groupName;
}

void RawEventGroup::splitGroupName(const std::string& groupName, std::string& baseName, std::string& subName)
{
    if (groupName == "") {
        baseName = "";
        subName = "";
        return;
    }

    auto pos = groupName.find_first_of("/");

    if (pos != std::string::npos) {

        baseName = groupName.substr(0, pos);
        subName = groupName.substr(pos, std::string::npos);
        return;
    }

    baseName = groupName;
    subName = "";
}

std::shared_ptr<RawEventGroup> RawEventGroup::group(const std::string& groupName)
{
    // auto it = subgroups.find(groupName);

    // if (it != subgroups.end()) {
    //     //found group
    //     return it->second;
    // }
    // //new subgroup
    // auto newGroup = std::make_shared<RawEventGroup>(name + "/" + groupName, stackTraceData);
    // return newGroup;

    std::string trimmedGroupName;

    //remove leading "/" from group name (relative path)
    if (groupName.size() > 0 && groupName.at(0) == '/') {
        trimmedGroupName = groupName.substr(1, std::string::npos);
    }
    else {
        trimmedGroupName = groupName;
    }

    std::shared_ptr<RawEventGroup> g;

    std::string baseName;
    std::string subName;

    splitGroupName(trimmedGroupName, baseName, subName);

    if (!groupMap.get(baseName, g)) {
        //new subgroup
        g = std::make_shared<RawEventGroup>(baseName, parentName, stackTraceData);
        groupMap.add(baseName, g);            
    }

    if (subName != "" && g != 0) {

        return g->group(subName);
    }

    return g;
}

std::vector<std::shared_ptr<RawEventGroup>> RawEventGroup::getSubgroups() const
{
    // std::vector<std::shared_ptr<RawEventGroup>> groupList;

    // for (auto& g : subgroups) {
    //     groupList.push_back(g.second);
    // }
    // return groupList;
    return subgroups;
}

///moves events from other group to local
void RawEventGroup::merge(const RawEventGroup& other)
{
    std::unique_lock groupLock(groupMutex);

    auto otherEvents = other.getEvents();

    if (events == 0) {
        events = std::make_shared<RawEventVector>();
    }

    // if (otherEvents != 0) {
    //     for (auto& evt : *otherEvents) {
    //         events->push_back(evt);
    //     }
    // }

    if (otherEvents != 0) {
        events->insert(events->end(), std::make_move_iterator(otherEvents->begin()), std::make_move_iterator(otherEvents->end()) );
    }

    refreshMinMax();

    //merge other subgroups into local subgroup (add new group if needed)
    for (auto& g : other.getSubgroups()) {
        if (g != 0) {
            group( g->getName() )->merge(*g);
        }
    }
}

void RawEventGroup::swapEvents(RawEventGroup& other)
{
    std::unique_lock groupLock(groupMutex);

    events.swap(other.events);

    refreshMinMax();

    // events->swap(*other.getEvents());

    for (auto& g : other.getSubgroups()) {
        if (g != 0) {
            group( g->getName() )->swapEvents(*g);
        }
    }
}

void RawEventGroup::copyEvents(const RawEventGroup& other)
{
    std::unique_lock groupLock(groupMutex);

    if (events != 0 && other.getEvents() != 0) {
        events->insert(events->end(), other.getEvents()->begin(), other.getEvents()->end());
    }
    refreshMinMax();

    for (auto& g : other.getSubgroups()) {
        if (g != 0) {
            group( g->getName() )->copyEvents(*g);
        }
    }
}

void RawEventGroup::addEvents(const RawEventVector& newEvents)
{
    std::unique_lock groupLock(groupMutex);

    if (events == 0) {
        events = std::make_shared<RawEventVector>();
    }

    events->insert(events->end(), newEvents.begin(), newEvents.end());
    refreshMinMax();
}

bool RawEventGroup::eventsEmpty() const
{
    std::unique_lock groupLock(groupMutex);

    if (events != 0) {
        if (events->size() != 0) {
            return false;
        }
    }

    for (auto& g : subgroups) {
        if (g != 0 && !g->eventsEmpty()) {
            return false;
        }
    }
    return true;
}

void RawEventGroup::sortEvents()
{
    std::unique_lock groupLock(groupMutex);

    if (events != 0) {
        std::sort(events->begin(), events->end());
    }

    for (auto& g : subgroups) {
        if (g != 0) {
            g->sortEvents();
        }
    }
}

void RawEventGroup::refreshMinMax()
{
    if (events != 0) {
        std::sort(events->begin(), events->end());

        timeMin = events->front().time();
        timeMax = events->back().time();
    }
}

void RawEventGroup::clear()
{
    std::unique_lock groupLock(groupMutex);

    if (events != 0) {
        events->clear();
    }

    parsedVars.clear();
    parsedTags.clear();

    for (auto& g : subgroups) {
        if (g != 0) {
            g->clear();
        }
    }
    groupMap.clear();
}

std::shared_ptr<StackTraceData> RawEventGroup::getStackTraceData() const
{
    return stackTraceData;
}

std::shared_ptr<RawEventVector> RawEventGroup::getEvents() const
{
    return events;
}

std::vector<ParsedVar> RawEventGroup::getVars() const
{
    return parsedVars;
}

std::vector<ParsedTag> RawEventGroup::getTags() const
{
    return parsedTags;
}

std::set<ParsedVar> RawEventGroup::getOverwrittenVars() const
{
    return overwrittenVars;
}

void RawEventGroup::setVars(const std::vector<ParsedVar>& vars)
{
    parsedVars.clear();
    parsedVars.insert(parsedVars.end(), vars.begin(), vars.end());
}

void RawEventGroup::setTags(const std::vector<ParsedTag>& tags)
{
    parsedTags.clear();
    parsedTags.insert(parsedTags.end(), tags.begin(), tags.end());
}

bool RawEventGroup::getConcreteTarget(const RawEventTarget& abstractTarget, RawEventTarget& concreteTarget) const
{
    //returns true if a concreteTarget was found

    std::unique_lock groupLock(groupMutex);

    if (!abstractTarget.isAbstract()) {
        concreteTarget = abstractTarget;
        return true;
    }

    concreteTarget = RawEventTarget();

    if (abstractTarget.device().isAbstract()) {
        auto dev = targetDevices.find(abstractTarget.device().name());

        if (dev != targetDevices.end() && !dev->second.isAbstract()) {
            concreteTarget.getDevice().setTargetDeviceID( dev->second.deviceID() );
        }
        else {
            //not found in device replacement rules; search in target rules using device abstract name
            auto target = targets.find(abstractTarget.device().name());
            
            if (target != targets.end() && !target->second.device().isAbstract()) {
                concreteTarget.getDevice().setTargetDeviceID( target->second.device().deviceID() );
            }
        }
    }

    if (abstractTarget.channel().isAbstract()) {
        auto target = targets.find(abstractTarget.channel().name());

        if (target != targets.end()) {
            if (!target->second.channel().isAbstract()) {
                concreteTarget.getChannel().setChannel( target->second.channel().channel() );
            }
            if (concreteTarget.device().isAbstract() && !target->second.device().isAbstract()) {
                concreteTarget.getDevice().setTargetDeviceID( target->second.device().deviceID() );
            }
        }
    }

    return !concreteTarget.isAbstract();
}

RawEventGroup& RawEventGroup::addMetaData(const std::string& key, const STI::Utils::MixedValue& data)
{
    std::unique_lock groupLock(groupMutex);
    metaData.addMetaData(key, data);
    return (*this);
}

RawEventGroup& RawEventGroup::addMetaData(const STI::Utils::MetaData& data)
{
    std::unique_lock groupLock(groupMutex);
    metaData.merge(data);
    return (*this);
}

const STI::Utils::MixedValue& RawEventGroup::getMetaData() const
{
    std::unique_lock groupLock(groupMutex);
    return metaData.getMetaData();
}

STI::Utils::MixedValue RawEventGroup::getMetaData(const std::string& key) const
{
    std::unique_lock groupLock(groupMutex);
    return metaData.getMetaData(key);
}


template<class Archive>
void RawEventGroup::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("start time", timeMin), 
		cereal::make_nvp("end time", timeMax), 
		cereal::make_nvp("timeOffset", timeOffset),
        cereal::make_nvp("referencePoints", referencePoints),
        cereal::make_nvp("events", events), 
        cereal::make_nvp("parsedVars", parsedVars), 
        cereal::make_nvp("parsedTags", parsedTags), 
        cereal::make_nvp("subgroups", subgroups), 
		cereal::make_nvp("name", name),
        cereal::make_nvp("parentName", parentName)

		// cereal::make_nvp("index", index), 
		// cereal::make_nvp("groupIndex", groupIndex),
		// cereal::make_nvp("targetServerID", targetServerID)
        // cereal::make_nvp("parentGroup", parentGroup)
		);
}


template void RawEventGroup::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void RawEventGroup::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

