
#include "STIPyShot.h"
#include <sti/engine/RawEvent.h>
#include <sti/engine/ParseTicket.h>
#include "MixedValuePy.h"
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/StackTrace.h>
// #include "NetworkFileHolder.h"

#include "LocalShot.h"
#include <sti/engine/ParsedVar.h>
#include "ParsedTag.h"

#include <sti/engine/ShotConfig.h>

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
using STI::Engine::LocalShot;
using STI::Engine::ShotConfig;
using STI::Python::MixedValuePy;


STIPyShot::STIPyShot(const std::shared_ptr<STI::Engine::Shot>& shot)
: shot(shot)
{
    if (shot == 0) {
        ShotConfig shotConfig;
        auto baseGroup = std::make_shared<RawEventGroup>();
        this->shot = std::make_shared<LocalShot>(shotConfig, baseGroup);
    }
    shot->getBaseEventGroup(baseEventGroup);
}


void STIPyShot::setvar(const std::string& name, const pybind11::object& value, 
            const STI::Engine::RawStackTrace& stackTrace)
{
    MixedValuePy mixedValue;
    mixedValue.setValue_py(value);
    group()->addvar(name, mixedValue, stackTrace);
}


void STIPyShot::setvar(const std::string& name, const pybind11::object& value, 
            const STI::Engine::RawStackTrace& stackTrace, const std::string& scope)
{
    // auto g = baseEventGroup->group(scope);

    MixedValuePy mixedValue;
    mixedValue.setValue_py(value);
    group(scope)->addvar(name, mixedValue, stackTrace);
}


STI::Engine::ParsedVar STIPyShot::var(const std::string& fullVarName, const STI::Engine::RawStackTrace& stackTrace)
{
    return group()->var(fullVarName, stackTrace);
}


void STIPyShot::settag(const std::string& name, const STI::Engine::RawStackTrace& stackTrace)
{
    group()->addtag(name, stackTrace);
}

void STIPyShot::settag(const std::string& name, const STI::Engine::RawStackTrace& stackTrace, const std::string& scope)
{
    // auto g = baseEventGroup->group(scope);
    group(scope)->addtag(name, stackTrace);
}

void STIPyShot::event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
                const STI::Engine::RawStackTrace& stackTrace)
{
    MixedValuePy mixedValue;
    mixedValue.setValue_py(value);
    group()->addEvent(target, time, mixedValue, RawEventType::Play, stackTrace);
}

void STIPyShot::event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
            const STI::Engine::RawStackTrace& stackTrace, const std::string& scope)
{
    // std::shared_ptr<RawEventGroup> g;

    // if (scope == "" || scope == "/") {
    //     g = baseEventGroup;
    // }
    // else {
    //     g = baseEventGroup->group(scope);
    // }

    // auto g = group(scope);

    MixedValuePy mixedValue;
    mixedValue.setValue_py(value);
    group(scope)->addEvent(target, time, mixedValue, RawEventType::Play, stackTrace);
}

void STIPyShot::meas(const STI::Engine::RawEventTarget& target, double time, 
            const STI::Engine::RawStackTrace& stackTrace, const std::string& scope)
{
    MixedValuePy mixedValue;
    group(scope)->addEvent(target, time, mixedValue, RawEventType::Measurement, stackTrace);
}

void STIPyShot::meas(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
            const STI::Engine::RawStackTrace& stackTrace, const std::string& scope)
{
    MixedValuePy mixedValue;
    mixedValue.setValue_py(value);
    group(scope)->addEvent(target, time, mixedValue, RawEventType::Measurement, stackTrace);
}


std::shared_ptr<std::vector<STI::Engine::RawEvent>> STIPyShot::getEvents()
{
    return baseEventGroup->getEvents();
}

std::vector<STI::Engine::ParsedVar> STIPyShot::getVars()
{
    return baseEventGroup->getVars();
}


void STIPyShot::append(const pybind11::object& func)
{
}

void STIPyShot::append(const STI::Engine::RawEvent& evt)
{
}


std::shared_ptr<STI::Engine::RawEventGroup> STIPyShot::group()
{
    return baseEventGroup;
}

std::shared_ptr<STI::Engine::RawEventGroup> STIPyShot::group(const std::string& fullName)
{
    // return baseEventGroup->group(fullName);

    std::shared_ptr<RawEventGroup> g;

    if (fullName == "" || fullName == "/") {
        g = baseEventGroup;
    }
    else {
        g = baseEventGroup->group(fullName);
    }
    return g;
}

