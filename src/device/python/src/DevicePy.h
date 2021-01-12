
#ifndef STI_PYTHON_DEVICEPY_H
#define STI_PYTHON_DEVICEPY_H

#include "Device.h"
#include "DeviceID.h"
#include "ChannelManagerPy.h"
#include "DeviceMessageDispatcher.h"
#include "DeviceCollectionPy.h"

#include <memory>

#include <pybind11/pybind11.h>

namespace STI
{
namespace Python
{



class Animal2 {
public:
    virtual ~Animal2() { }
    virtual std::string go(int n_times) = 0;
    virtual std::string getID() = 0;
};

class PyAnimal2 : public Animal2 {
public:
    /* Inherit the constructors */
    using Animal2::Animal2;

    /* Trampoline (need one for each virtual function) */
    std::string go(int n_times) override {
        PYBIND11_OVERLOAD_PURE(
            std::string, /* Return type */
            Animal2,      /* Parent class */
            go,          /* Name of function in C++ (must match Python name) */
            n_times      /* Argument(s) */
        );
    }


    std::string getID() override {
        PYBIND11_OVERLOAD_PURE(
            std::string, /* Return type */
            Animal2,      /* Parent class */
            getID,          /* Name of function in C++ (must match Python name) */
            
        );
    }
};






class ChannelManagerPy;



class DevicePy2
{
public:
    DevicePy2() {}
    DevicePy2(const std::shared_ptr<STI::Device::Device>& device);
    virtual ~DevicePy2();

    void setDevice(const std::shared_ptr<STI::Device::Device>& device);
    std::shared_ptr<STI::Device::Device> getDevice();
    //virtual const STI::Device::DeviceID getIDpy() const = 0;

    std::shared_ptr<STI::Python::DeviceCollectionPy> getDeviceCollection();
//    virtual void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection) = 0;
    std::shared_ptr<STI::Device::DeviceMessageDispatcher> getMessageDispatcher();
//    virtual std::shared_ptr<EventEngineSchedulerPy> getEngineScheduler() = 0;    
    std::shared_ptr<ChannelManagerPy> getChannelManager();

    const STI::Device::DeviceID getID() const;
//    int test2(int x);

private:

    std::shared_ptr<STI::Device::Device> device_;

};


class DevicePy
{
public:

//    DevicePy(const std::shared_ptr<STI::Device::Device>& device);
    virtual ~DevicePy() {}

    //virtual const STI::Device::DeviceID getIDpy() const = 0;

//    virtual void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection) = 0;
    virtual std::shared_ptr<STI::Device::DeviceMessageDispatcher> getMessageDispatcher() = 0;
//    virtual std::shared_ptr<EventEngineSchedulerPy> getEngineScheduler() = 0;    
    virtual std::shared_ptr<ChannelManagerPy> getChannelManager() = 0;

    virtual STI::Device::DeviceID getIDpy() = 0;
    virtual int test2(int x) = 0;

private:

    //std::shared_ptr<STI::Device::Device> device;

};

class DevicePyTrampoline : public DevicePy {
public:
    /* Inherit the constructors */
    using DevicePy::DevicePy;

    // /* Trampoline (need one for each virtual function) */
    // const STI::Device::DeviceID getIDpy() const override {
    //     PYBIND11_OVERRIDE_PURE(
    //         const STI::Device::DeviceID, /* Return type */
    //         DevicePy,      /* Parent class */
    //         getIDpy,          /* Name of function in C++ (must match Python name) */
    //                           /* Argument(s) */
    //     );
    // }

    std::shared_ptr<STI::Device::DeviceMessageDispatcher> getMessageDispatcher() override
    {
        PYBIND11_OVERRIDE_PURE(
            std::shared_ptr<STI::Device::DeviceMessageDispatcher>, /* Return type */
            DevicePy,      /* Parent class */
            getMessageDispatcher,          /* Name of function in C++ (must match Python name) */
                              /* Argument(s) */
        );
    }

    
    std::shared_ptr<ChannelManagerPy> getChannelManager() override
    {
        PYBIND11_OVERRIDE_PURE(
            std::shared_ptr<ChannelManagerPy>, /* Return type */
            DevicePy,      /* Parent class */
            getChannelManager,          /* Name of function in C++ (must match Python name) */
                              /* Argument(s) */
        );
    }



    STI::Device::DeviceID getIDpy() override
    {
        PYBIND11_OVERRIDE_PURE(
            STI::Device::DeviceID, /* Return type */
            DevicePy,      /* Parent class */
            getIDpy,          /* Name of function in C++ (must match Python name) */
                              /* Argument(s) */
        );
    }

    int test2(int x) override
    {
        PYBIND11_OVERRIDE_PURE(
            int, /* Return type */
            DevicePy,      /* Parent class */
            test2,          /* Name of function in C++ (must match Python name) */
            x                  /* Argument(s) */
        );
    }

};

} //Python
} //STI

#endif

