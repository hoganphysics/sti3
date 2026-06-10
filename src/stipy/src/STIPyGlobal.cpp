#include "STIPyGlobal.h"
#include "STIPyShot.h"
#include <sti/device/DeviceID.h>
#include <sti/engine/RawEventTargetDevice.h>
#include <sti/engine/StackTraceData.h>

#include "StackTrace.h"

#include <stdexcept>

using STI::Python::STIPyGlobal;
using STI::Python::STIPyShot;
using STI::Engine::RawEventTargetDevice;
using STI::Engine::RawEventGroup;
using STI::Engine::CompressedStackTrace;
using STI::Engine::RawEventTarget;
using STI::Engine::StackTrace;


STIPyGlobal::STIPyGlobal()
{
    makingShot = false;
}

STIPyGlobal::~STIPyGlobal()
{
}

std::shared_ptr<STIPyGlobal> STIPyGlobal::getInstance()
{
    if (!initialized) {
        instance = std::make_shared<STI::Python::Concrete_STIPyGlobal>();
        initialized = true;
    }
    return instance;
}

// void STIPyGlobal::makeShot(const std::shared_ptr<STIPyShot>& shot, const std::function<void(void)>& func)
// {
//     makeShot(shot, "", func);
// }

void STIPyGlobal::makeShot(const std::shared_ptr<STIPyShot>& shot, const std::function<void(void)>& func)
{
    makeShot(shot, func, "");
}

void STIPyGlobal::makeShot(const std::shared_ptr<STIPyShot>& shot, const std::function<void(void)>& func, const std::string& mainFile)
{
    {
        std::unique_lock<std::mutex> shotLock(shotMutex);

        if (makingShot) {
            //error
            std::runtime_error ex("Illegal reentrant call to makeshot. Shots cannot be generated recursively.");
            throw ex;
            return;
        }
        makingShot = true;

        currentShot = shot;
    }
    
    try {
        func();
    }
    catch (...) {
        {
            std::unique_lock<std::mutex> shotLock(shotMutex);
            makingShot = false;
        }
        throw;
    }

    {
        std::unique_lock<std::mutex> shotLock(shotMutex);
        makingShot = false;
    }

    // set filename (temp work around)

    if (shot == 0) return;

    const STI::Engine::ShotConfig& sc = shot->getShotConfig();
    STI::Engine::ShotConfig& sc2 = const_cast<STI::Engine::ShotConfig&>(sc);    //temp
    auto& files = shot->group()->getStackTraceData()->getTimingFiles();

    if (!mainFile.empty()) {
        sc2.file = mainFile;
    } else if (!files.empty()) {
        sc2.file = files[0].getFullFilename();
    } else {
        sc2.file = "default.shot";
    }
}


STI::Engine::ParsedVar STIPyGlobal::var(const std::string& fullVarName, const STI::Engine::StackTrace& stackTrace)
{
    std::unique_lock<std::mutex> shotLock(shotMutex);

    STI::Engine::ParsedVar v;

    if (!makingShot) {
        std::runtime_error ex("No associated shot. Global 'setvar(...)' cannot be called outside a call to makeshot.");
        throw ex;
        return v;
    }

    if (currentShot != 0) {
        v = currentShot->var(fullVarName, stackTrace);
    }
    return v;
}

void STIPyGlobal::setvar(const std::string& name, const pybind11::object& value, 
            const STI::Engine::StackTrace& stackTrace, const std::string& scope)
{
    std::unique_lock<std::mutex> shotLock(shotMutex);

    if (!makingShot) {
        std::runtime_error ex("No associated shot. Global 'setvar(...)' cannot be called outside a call to makeshot.");
        throw ex;
        return;
    }

    if (currentShot != 0) {
        currentShot->setvar(name, value, stackTrace, scope);
    }
}

void STIPyGlobal::settag(const std::string& name, const STI::Engine::StackTrace& stackTrace, const std::string& scope)
{
    std::unique_lock<std::mutex> shotLock(shotMutex);

    if (!makingShot) {
        std::runtime_error ex("No associated shot. Global 'settag(...)' cannot be called outside a call to makeshot.");
        throw ex;
        return;
    }

    if (currentShot != 0) {
        currentShot->settag(name, stackTrace, scope);
    }
}

std::shared_ptr<STI::Engine::RawEventGroup> STIPyGlobal::group(const std::string& name)
{
    std::unique_lock<std::mutex> shotLock(shotMutex);

    std::shared_ptr<STI::Engine::RawEventGroup> g;

    if (!makingShot) {
        std::runtime_error ex("No associated shot. Global 'group(...)' cannot be called outside a call to makeshot.");
        throw ex;
        return g;
    }

    if (currentShot != 0) {
        g = currentShot->group(name);
    }
    else {
        g = std::make_shared<STI::Engine::RawEventGroup>();
    }

    return g;
}

void STIPyGlobal::event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value,
            const STI::Engine::StackTrace& stackTrace)
{
    std::unique_lock<std::mutex> shotLock(shotMutex);

    if (!makingShot) {
        std::runtime_error ex("No associated shot. Global 'event(...)' cannot be called outside a call to makeshot.");
        throw ex;
        return;
    }

    if (currentShot != 0) {
        currentShot->event(target, time, value, stackTrace);
    }
}

void STIPyGlobal::event(const RawEventTarget& target, double time, const pybind11::object& value, 
                        const StackTrace& stackTrace, const std::string& scope)
{
    std::unique_lock<std::mutex> shotLock(shotMutex);

    if (!makingShot) {
        std::runtime_error ex("No associated shot. Global 'event(...)' cannot be called outside a call to makeshot.");
        throw ex;
        return;
    }

    if (currentShot != 0) {
        currentShot->event(target, time, value, stackTrace, scope);
    }
}

void STIPyGlobal::meas(const RawEventTarget& target, double time, const pybind11::object& value,
                        const StackTrace& stackTrace, const std::string& scope)
{
    std::unique_lock<std::mutex> shotLock(shotMutex);

    if (!makingShot) {
        std::runtime_error ex("No associated shot. Global 'meas(...)' cannot be called outside a call to makeshot.");
        throw ex;
        return;
    }

    if (currentShot != 0) {
        currentShot->meas(target, time, value, stackTrace, scope);
    }
}

void STIPyGlobal::meas(const RawEventTarget& target, double time, const StackTrace& stackTrace, 
                        const std::string& scope)
{
    std::unique_lock<std::mutex> shotLock(shotMutex);

    if (!makingShot) {
        std::runtime_error ex("No associated shot. Global 'meas(...)' cannot be called outside a call to makeshot.");
        throw ex;
        return;
    }

    if (currentShot != 0) {
        currentShot->meas(target, time, stackTrace, scope);
    }
}

void STIPyGlobal::set_trigger(const STI::Device::DeviceID& deviceID, const STI::Engine::StackTrace& stackTrace)
{
    std::unique_lock<std::mutex> shotLock(shotMutex);

    if (!makingShot) {
        std::runtime_error ex("No associated shot. Global 'set_trigger(...)' cannot be called outside a call to makeshot.");
        throw ex;
        return;
    }

    if (currentShot != 0) {
        currentShot->set_trigger(deviceID, stackTrace);
    }
}

std::shared_ptr<STIPyGlobal> STIPyGlobal::instance = 0;
bool STIPyGlobal::initialized = false;
