

#include "STIPyGlobal.h"
#include "STIPyShot.h"
#include <sti/device/DeviceID.h>
#include <sti/engine/RawEventTargetDevice.h>

#include "RawStackTrace.h"

#include <stdexcept>
// #include <iostream>

using STI::Python::STIPyGlobal;
using STI::Python::STIPyShot;
using STI::Engine::RawEventTargetDevice;
using STI::Engine::RawEventGroup;
using STI::Engine::StackTrace;
using STI::Engine::RawEventTarget;
using STI::Engine::RawStackTrace;


STIPyGlobal::STIPyGlobal()
{
    makingShot = false;

    // std::cout << "STIPyGlobal constructor" << std::endl;
}

STIPyGlobal::~STIPyGlobal()
{
    // std::cout << "STIPyGlobal destructor" << std::endl;
}

std::shared_ptr<STIPyGlobal> STIPyGlobal::getInstance()
{
    if (!initialized) {
        instance = std::make_shared<STI::Python::Concrete_STIPyGlobal>();
        initialized = true;
    }
    return instance;
}

void STIPyGlobal::makeShot(const std::shared_ptr<STIPyShot>& shot, const std::function<void(void)>& func)
{
    makeShot(shot, "", func);
}

void STIPyGlobal::makeShot(const std::shared_ptr<STIPyShot>& shot, const std::string& name, const std::function<void(void)>& func)
{
    {
        std::unique_lock<std::mutex> shotLock(shotMutex);

        if (makingShot) {
            //error
            // std::cout << "Error: reentrant makeshot" << std::endl;
            std::runtime_error ex("Illegal reentrant call to makeshot. Shots cannot be generated recursively.");
            throw ex;
            return;
        }
        makingShot = true;

        currentShot = shot;
    }
    
    func();

    {
        std::unique_lock<std::mutex> shotLock(shotMutex);
        makingShot = false;
    }
}

void STIPyGlobal::event(const STI::Engine::RawEventTarget& target, double time, const pybind11::object& value,
            const STI::Engine::RawStackTrace& stackTrace)
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
                        const RawStackTrace& stackTrace, const std::string& scope)
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
                        const RawStackTrace& stackTrace, const std::string& scope)
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

void STIPyGlobal::meas(const RawEventTarget& target, double time, const RawStackTrace& stackTrace, 
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

// STI::Engine::RawEventTargetDevice STIPyGlobal::dev(const std::string& name, const std::string& address, unsigned module)
// {
//     std::unique_lock<std::mutex> shotLock(shotMutex);

//     RawEventTargetDevice device(name, address, module);

// //     if (currentShot != 0) {
// //         device = std::make_shared<RawEventTargetDevice>(name, address, module, currentShot->getServerID().getID());
// //     }
// //     else {
// //         STI::Device::DeviceID id(name, address, module);
// // //        std::string id = STI::Device::DeviceID::generateID(name, address, module);
// //         device = std::make_shared<RawEventTargetDevice>(id);
// //     }

//     return device;
// }

std::shared_ptr<STIPyGlobal> STIPyGlobal::instance = 0;
bool STIPyGlobal::initialized = false;
