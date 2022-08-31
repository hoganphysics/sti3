
#include "STIPyShot.h"
#include <sti/engine/RawEvent.h>
#include <sti/engine/ParseTicket.h>
#include "MixedValuePy.h"
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/StackTrace.h>
#include "NetworkFileHolder.h"

#include "ParseResult.h"
#include "ParsedVar.h"
#include "ParsedTag.h"
// #include "ParsedVarPy.h"

#include <sti/engine/ParseID.h>

#include <pybind11/pybind11.h>

#include <iostream>

using STI::Python::STIPyShot;
using STI::Python::ParseTicket;
using STI::Engine::RawEvent;
using STI::Engine::StackTrace;
using STI::Engine::RawEventType;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventGroup;
// using STI::Python::StackTracePy;
// using STI::Engine::RawEventGroupManager;
using STI::Engine::ParsedVar;
using STI::Engine::ParsedTag;




STIPyShot::STIPyShot(const std::shared_ptr<STI::Engine::LocalShot>& shot)
: shot(shot)
{
}


STIPyShot::STIPyShot(const std::shared_ptr<STI::Engine::LocalShot>& shot, const std::string& name)
: shot(shot), baseGroup("/" + name, 0), nextGroupIndex(1)
{
    eventNumber = 0;
//    events = std::make_shared<std::vector<STI::Engine::RawEvent>>();

    if (shot != 0) {
        shot->getParseResult(parseResult);        
    }

    if (parseResult != 0) {
        functionMap = std::make_shared<STIPyShot::VectorMapString>(parseResult->functionNames);
        fileMap = std::make_shared<STIPyShot::VectorMapFileHolder>(parseResult->timingFiles); 
        groupMap = std::make_shared<RawEventGroupManager>(parseResult->eventGroups);
        varMap = std::make_shared<STIPyShot::VectorMapParsedVar>(parseResult->parsedVars); 
        tagMap = std::make_shared<STIPyShot::VectorMapParsedTag>(parseResult->parsedTags); 
    }
    else {
        functionMap = std::make_shared<STIPyShot::VectorMapString>();
        fileMap = std::make_shared<STIPyShot::VectorMapFileHolder>(); 
        groupMap = std::make_shared<RawEventGroupManager>();
        varMap = std::make_shared<STIPyShot::VectorMapParsedVar>(); 
        tagMap = std::make_shared<STIPyShot::VectorMapParsedTag>();        
    }

    groupMap->addGroup(baseGroup.getName());
    groupMap->get(baseGroup.getName(), baseGroup);
    // groupMap->add(baseGroup.getName(), baseGroup);

// parseResult->

    // parseResult = std::make_shared<STI::Engine::ParseResult>();

    // vars = std::make_shared<std::map<std::string, pybind11::object>>();
}

// std::vector<STI::Engine::RawEvent> STIPyShot::getEvents()
// {
//     return (*events);
// }

// void STIPyShot::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts)
// {
//     evts = events;
// }

// STI::Engine::RawEventGroup STIPyShot::getGroup(const std::string& fullName)
// {
//     std::vector<std::string> names;
//     STI::Utils::splitString(fullName, "/", names);
// }

std::string STIPyShot::getAbsoluteGroupName(const std::string& groupName)
{

    /*
    group("MOT/Shutter")
    group("/Frame 1/MOT/Shutter")

    */


    std::vector<std::string> names;
    STI::Utils::splitString(groupName, "/", names);
    std::stringstream absGroupName;

    // if (groupName.size() > 0 && groupName.at(0) == '/') {
    //     //absolute group name given
    //     absGroupName << groupName;
    // }

    if (names.size() > 0 && groupName.size() > 0 && groupName.at(0) == '/' && names.at(1) == baseGroup.getName()) {
        //absolute group name given
        absGroupName << groupName;
    }
    else {
        absGroupName << "/" << baseGroup.getName();

        for (auto& n : names) {
            if (n != "") {
                absGroupName << "/" << n;
            }
        }
    }
    return absGroupName.str();
}



STI::Engine::RawEventGroup STIPyShot::group(const std::string& fullName)
{
    std::string absName = getAbsoluteGroupName(fullName);

    STI::Engine::RawEventGroup group;
    std::cout << "groupMap null: " << (groupMap == 0 ? "Yes" : "No") << std::endl;
    std::cout << "group absName: " << absName << std::endl;
    
    if (!groupMap->get(absName, group)) {
        //new group
        groupMap->addGroup(absName);
        groupMap->get(absName, group);

        for (auto& m : groupMap->indexMap()) {
            std::cout << "** keys: " << m.first << std::endl;
        }
        

        // group = baseGroup.makeSubgroup(name, nextGroupIndex);
        // nextGroupIndex++;
        // groupMap->add(name, group);
    }
    std::cout << "group: " << group.getName() << std::endl;
    return group;
}

STI::Engine::RawEventGroup STIPyShot::group(const std::string& name, const STI::Engine::RawEventGroup& parentGroup)
{
    std::string absGroupName;
    
    if(!groupMap->getAbsGroupName(parentGroup, absGroupName)) {
        //parentGroup name not found
        // group(absGroupName); //unneeded, since the parent group will be added later if not found
    }

    return group(absGroupName + "/" + name);
}

// STI::Engine::RawEventGroup STIPyShot::makeGroup(const std::string& fullName)
// {
//     std::vector<std::string> names;
//     STI::Utils::splitString(fullName, "/", names);
// }

void STIPyShot::setvar(const std::string& name, const pybind11::object& value, const StackTracePy& stackTrace)
{
    setvar(name, value, stackTrace, baseGroup);
}

void STIPyShot::setvar(const std::string& name, const pybind11::object& value, const StackTracePy& stackTrace, const RawEventGroup& scope)
{
    // std::unique_lock<std::mutex> evtLock(eventMutex);

    // if (vars == 0) return;

    // auto it = vars->find(name);

    // if (it == vars->end()) {    //not found
    //     (*vars)[name] = value;
    // }

    if(varMap->exists(getScopedName(name, scope))) {
        std::cout << "var exists" << std::endl;
        throw pybind11::key_error("Var already defined!");
    }

    ParsedVar var;
    var.name = name;
    // var.value = MixedValuePy(value);
    var.scope = scope;

    auto mvpy = MixedValuePy(value);
    const STI::Utils::MixedValue& ref = mvpy;
    var.value.setValue(ref);
    
    addStackTrace(stackTrace, var.trace);

    varMap->add(getScopedName(name, scope), var);

}


std::string STIPyShot::getScopedName(const std::string& name, const RawEventGroup& scope)
{
    std::string absGroupName;
    
    if(!groupMap->getAbsGroupName(scope, absGroupName)) {
        //scope not found; add to groups
        group(absGroupName);
    }
    return absGroupName + "/" + name;
}

pybind11::object STIPyShot::getvar(const std::string& name)
{
    ParsedVar var;

    if (varMap->get(name, var)) {
        //fully scoped var name found (absolute name)
        MixedValuePy pyVal;
        pyVal.setValue(var.value);
        return pyVal.getValue_py();
    }
    //Assume this var name is in the default baseGroup scope
    return getvar(name, baseGroup);
}

pybind11::object STIPyShot::getvar(const std::string& name, const STI::Engine::RawEventGroup& scope)
{
    ParsedVar var;
    if (varMap->get(getScopedName(name, scope), var)) {
        //found
        MixedValuePy pyVal;
        pyVal.setValue(var.value);
        return pyVal.getValue_py();
    }
    throw pybind11::key_error("Var not found!");
}

void STIPyShot::settag(const std::string& name, const StackTracePy& stackTrace, const RawEventGroup& scope)
{

}

void STIPyShot::event(const RawEventTarget& target, double time, const pybind11::object& value, 
                        const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group)
{
    addEventPy(target, time, value, RawEventType::Play, stackTrace, group);
}

void STIPyShot::meas(const RawEventTarget& target, double time, const StackTracePy& stackTrace, 
                        const STI::Engine::RawEventGroup& group)
{
    STI::Python::MixedValuePy valuepy;  //empty
    addEvent(target, time, valuepy, RawEventType::Measurement, stackTrace, group);
}

void STIPyShot::meas(const RawEventTarget& target, double time, const pybind11::object& value, 
                        const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group)
{
    addEventPy(target, time, value, RawEventType::Measurement, stackTrace, group);
}

void STIPyShot::addEventPy(const RawEventTarget& target, double time, const pybind11::object& value, 
                        const RawEventType& type, const StackTracePy& stackTrace, const STI::Engine::RawEventGroup& group)
{
    STI::Python::MixedValuePy valuepy;
    valuepy.setValue_py(value);
    std::cout << "MixedValuePy: " << valuepy.print() << std::endl;

    // addEvent(target, time, valuepy, type, stackTrace, group);
    addEvent(target, time, MixedValuePy(value), type, stackTrace, group);
}

//addEvent should add timing files and stack information. Create FileHolders for all files.
void STIPyShot::addEvent(const RawEventTarget& target, double time, const MixedValuePy& valuepy, 
                            const STI::Engine::RawEventType& type, const StackTracePy& stackTrace, 
                            const STI::Engine::RawEventGroup& group)
{
    std::unique_lock<std::mutex> evtLock(eventMutex);

    // std::string description = "";
    ;
//    eventNumber++;
    std::shared_ptr<std::vector<STI::Engine::RawEvent>> events;

    if (shot != 0) {
        shot->getEvents(events);

        if (events != 0) {
            // events->push_back( RawEvent(channel.device()->id(), time, channel.channel(), 
            //                             valuepy, stackTrace, events->size(), type) );

            // STI::Engine::RawEventTarget target(channel.device()->id(), channel.channel());
 
            //parseResult.eventGroups.push_back(group);
            StackTrace trace;
            addStackTrace(stackTrace, trace);
            
            events->emplace_back(target, time, valuepy, events->size(), type, trace, group);
        }

    }

    // if (events != 0) {
    //     events->push_back( RawEvent(channel.device()->id(), time, channel.channel(), 
    //                                 valuepy, stackTrace, events->size(), type) );        
    // }

}



// class RawEventGroupIndexedMap : public IndexedMap<std::string, RawEventGroup>
// {

//     bool mergeGroups(RawEventGroupIndexedMap& other)
//     {
//         //shift other indices
//         unsigned shift = indices.size();

//         for (auto& g : other.items) {
//             g.shiftIndicis(shift);  //shifts index and parent indicies
//         }
//     }
// };

// class IndexedMap2
// {

//     unsigned add(const std::string& name, const std::string& item)
//     {
//         auto it = indices.find(name);
//         if (it != indices.end()) {
//             //exists
//             return it->second;
//         }
//         items.push_back(item);
//         indices[name] = items.size() - 1;

//     }

//     std::map<std::string, unsigned> indices;
//     std::vector<std::string> items;
// };

unsigned STIPyShot::addFile(const std::string& filename)
{
    unsigned index;

    if (fileMap->getIndex(filename, index)) {
        return index;
    }
    //new file
    auto file = std::make_shared<STI::Network::NetworkFileHolder>(filename);
    return fileMap->add(filename, file);
}

void STIPyShot::addStackTrace(const StackTracePy& pyStackTrace, STI::Engine::StackTrace& stackTrace)
{

    // fileMap->exists()

    for (auto& frame : pyStackTrace.getFrames()) {

        stackTrace.appendFrame(
            addFile(frame.file), 
            frame.line, 
            functionMap->add(frame.func, frame.func));
        ;        
    }
    ;
}

// ParseTicket STIPyShot::parse()
// {
//     STI::Engine::ParseID pid;
//     pid.parseTimestamp.timestamp = 1.1;
    
//  //   ParseTicket ticket(pid);
//     ParseTicket ticket = libDevice->makeParseTicket(pid);

//  //   ParseTicket ticket;
//     return ticket;
// }

// ParseTicket STIPyShot::parse(const pybind11::dict& channels)
// {

//     STI::Engine::ParseID pid;
//     pid.parseTimestamp.timestamp = 1.1;
    
//     ParseTicket ticket(pid);
// //    ParseTicket ticket;
//     return ticket;
// }


void STIPyShot::append(pybind11::object func)
{
}

void STIPyShot::append(const STI::Engine::RawEvent& evt)
{
}

std::vector<STI::Engine::RawEvent> STIPyShot::getEvents()
{
    std::shared_ptr<std::vector<STI::Engine::RawEvent>> events;

    if (shot != 0) {
        shot->getEvents(events);
    }

    if (events != 0) {
        return *events;
    }
    else {
        // auto nullEvts = std::make_shared<std::vector<STI::Engine::RawEvent>>();
        // return *nullEvts;
        std::vector<STI::Engine::RawEvent> nullEvts;
        return nullEvts;
    }
}

std::vector<STI::Engine::RawEvent> STIPyShot::getEvents(const STI::Engine::RawEventGroup& group)
{
    std::shared_ptr<std::vector<STI::Engine::RawEvent>> events;

    if (shot != 0) {
        shot->getEvents(events);
    }

    std::vector<STI::Engine::RawEvent> groupEvents;

    if (events != 0) {
        for (auto& e : *events) {
            if (e.groupIndex() == group.getFullIndex()) {
                groupEvents.push_back(e);
            }
        }
    }
    return groupEvents;
}

std::vector<STI::Engine::RawEvent> STIPyShot::getEvents(const std::string& groupName)
{
    STI::Engine::RawEventGroup group;
    
    if (!groupMap->get(groupName, group)) {
        //not found
        return std::vector<STI::Engine::RawEvent>();
    }
    return getEvents(group);
}


std::vector<ParsedVar> STIPyShot::getVars()
{
    std::vector<ParsedVar> vars;
    
    for (auto& v : varMap->vec()) {
        vars.emplace_back(v, parseResult);
    }
    return vars;
}

std::vector<ParsedVar> STIPyShot::getVars(const STI::Engine::RawEventGroup& group)
{
    std::vector<ParsedVar> vars;
    
    for (auto& v : varMap->vec()) {
        if (v.scope == group) {
            vars.emplace_back(v, parseResult);
        }
    }
    return vars;
}

std::vector<ParsedVar> STIPyShot::getVars(const std::string& groupName)
{
    STI::Engine::RawEventGroup group;
    
    if (!groupMap->get(groupName, group)) {
        //not found
        return std::vector<STI::Python::ParsedVarPy>();
    }
    return getVars(group);
}


// void STIPyShot::getEvents(std::shared_ptr<std::vector<STI::Engine::RawEvent>>& evts)
// {
//     evts = events;
// }

