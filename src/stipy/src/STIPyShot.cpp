#include "STIPyShot.h"

#include <sti/engine/ParsedVar.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/ParseTicket.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/ShotConfig.h>
#include <sti/engine/CompressedStackTrace.h>
#include <sti/engine/StackTraceData.h>

#include "LocalShot.h"
#include "MixedValuePy.h"
#include <sti/engine/ParsedTag.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

using STI::Python::STIPyShot;
using STI::Python::ParseTicket;
using STI::Engine::RawEvent;
using STI::Engine::CompressedStackTrace;
using STI::Engine::RawEventType;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventGroup;
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
        auto rootGroup = std::make_shared<RawEventGroup>();
        this->shot = std::make_shared<LocalShot>(shotConfig, rootGroup);
    }
    shot->getRootEventGroup(rootEventGroup);
}

const ShotConfig& STIPyShot::getShotConfig() const
{
    return shot->getShotConfig();
}

void STIPyShot::setvar(const std::string& name, const pybind11::object& value, 
            const STI::Engine::StackTrace& stackTrace)
{
    MixedValuePy mixedValue;
    mixedValue.setValue_py(value);
    auto result = group()->addvar(name, mixedValue, stackTrace);

    if (!result.success) {
        throw py::value_error(result.errorMessage);
    }
}


void STIPyShot::setvar(const std::string& name, const pybind11::object& value, 
            const STI::Engine::StackTrace& stackTrace, const std::string& scope)
{
    MixedValuePy mixedValue;
    mixedValue.setValue_py(value);
    auto result = group(scope)->addvar(name, mixedValue, stackTrace);

    if (!result.success) {
        throw py::value_error(result.errorMessage);
    }
}


STI::Engine::ParsedVar STIPyShot::var(const std::string& fullVarName, const STI::Engine::StackTrace& stackTrace)
{
    return group()->var(fullVarName, stackTrace);
}


void STIPyShot::settag(const std::string& name, const STI::Engine::StackTrace& stackTrace)
{
    auto result = group()->addtag(name, stackTrace);
    
    if (!result.success) {
        throw std::runtime_error(result.errorMessage);
    }
}

void STIPyShot::settag(const std::string& name, const STI::Engine::StackTrace& stackTrace, const std::string& scope)
{
    auto result = group(scope)->addtag(name, stackTrace);
    
    if (!result.success) {
        throw std::runtime_error(result.errorMessage);
    }
}

void STIPyShot::event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
                const STI::Engine::StackTrace& stackTrace)
{
    MixedValuePy mixedValue;
    mixedValue.setValue_py(value);
    group()->addEvent(target, time, mixedValue, RawEventType::Play, stackTrace);
}

void STIPyShot::event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
            const STI::Engine::StackTrace& stackTrace, const std::string& scope)
{
    MixedValuePy mixedValue;
    mixedValue.setValue_py(value);
    group(scope)->addEvent(target, time, mixedValue, RawEventType::Play, stackTrace);
}

void STIPyShot::meas(const STI::Engine::RawEventTarget& target, double time, 
            const STI::Engine::StackTrace& stackTrace, const std::string& scope)
{
    MixedValuePy mixedValue;
    group(scope)->addEvent(target, time, mixedValue, RawEventType::Measurement, stackTrace);
}

void STIPyShot::meas(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value, 
            const STI::Engine::StackTrace& stackTrace, const std::string& scope)
{
    MixedValuePy mixedValue;
    mixedValue.setValue_py(value);
    group(scope)->addEvent(target, time, mixedValue, RawEventType::Measurement, stackTrace);
}

void STIPyShot::set_trigger(const STI::Device::DeviceID& deviceID, const STI::Engine::StackTrace& stackTrace)
{
    //set trigger device for shot

    auto tags = group()->getTags();
    for (const auto& tag : tags) {
        if (tag.name == "Set Global Trigger") {
            //already set
            auto trace = group()->getStackTraceData()->getStackTrace(tag.trace);
            throw py::value_error("Global trigger device already set for this shot: \n" + trace.print());
            return;
        }
    }

    group()->addMetaData("delegatedTriggerID", STI::Utils::MixedValue(deviceID.getID()));
    settag("Set Global Trigger", stackTrace);
}

std::shared_ptr<std::vector<STI::Engine::RawEvent>> STIPyShot::getEvents()
{
    return rootEventGroup->getEvents();
}

std::vector<STI::Engine::ParsedVar> STIPyShot::getVars()
{
    return rootEventGroup->getVars();
}


void STIPyShot::append(const pybind11::object& func)
{
}

void STIPyShot::append(const STI::Engine::RawEvent& evt)
{
}


std::shared_ptr<STI::Engine::RawEventGroup> STIPyShot::group()
{
    return rootEventGroup;
}

std::shared_ptr<STI::Engine::RawEventGroup> STIPyShot::group(const std::string& fullName)
{
    std::shared_ptr<RawEventGroup> g;

    if (fullName == "" || fullName == "/") {
        g = rootEventGroup;
    }
    else {
        g = rootEventGroup->group(fullName);
    }
    return g;
}
