
#ifndef STI_PYTHON_DEVICEPY_H
#define STI_PYTHON_DEVICEPY_H

#include <sti/device/Device.h>
#include <sti/device/DeviceID.h>
#include "ChannelManagerPy.h"
#include <sti/device/DeviceMessageDispatcher.h>
#include "DeviceCollectionPy.h"

#include <memory>

#include <pybind11/pybind11.h>

namespace STI
{
namespace Python
{

class ChannelManagerPy;
// class EventEngineSchedulerPy;
class AttributeManagerPy;
class PersistenceManagerPy;


class DevicePy
{
public:
    DevicePy() {}
    DevicePy(const std::shared_ptr<STI::Device::Device>& device);
    virtual ~DevicePy();

    void setDevice(const std::shared_ptr<STI::Device::Device>& device);
    std::shared_ptr<STI::Device::Device> getDevice();

    const STI::Device::DeviceID getID() const;
    void kill();

    std::shared_ptr<STI::Python::DeviceCollectionPy> getDeviceCollection();
    std::shared_ptr<STI::Device::DeviceMessageDispatcher> getMessageDispatcher();
    std::shared_ptr<STI::Engine::EventEngineScheduler> getEngineScheduler();
    std::shared_ptr<ChannelManagerPy> getChannelManager();
    std::shared_ptr<AttributeManagerPy> getAttributeManager();
    std::shared_ptr<PersistenceManagerPy> getPersistenceManager();


private:

    std::shared_ptr<STI::Device::Device> device_;

};




// class DevicePy
// {
// public:

// //    DevicePy(const std::shared_ptr<STI::Device::Device>& device);
//     virtual ~DevicePy() {}

//     //virtual const STI::Device::DeviceID getIDpy() const = 0;

// //    virtual void getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection) = 0;
//     virtual std::shared_ptr<STI::Device::DeviceMessageDispatcher> getMessageDispatcher() = 0;
// //    virtual std::shared_ptr<EventEngineSchedulerPy> getEngineScheduler() = 0;    
//     virtual std::shared_ptr<ChannelManagerPy> getChannelManager() = 0;

//     virtual STI::Device::DeviceID getIDpy() = 0;
//     virtual int test2(int x) = 0;

// private:

//     //std::shared_ptr<STI::Device::Device> device;

// };

// class DevicePyTrampoline : public DevicePy {
// public:
//     /* Inherit the constructors */
//     using DevicePy::DevicePy;

//     // /* Trampoline (need one for each virtual function) */
//     // const STI::Device::DeviceID getIDpy() const override {
//     //     PYBIND11_OVERRIDE_PURE(
//     //         const STI::Device::DeviceID, /* Return type */
//     //         DevicePy,      /* Parent class */
//     //         getIDpy,          /* Name of function in C++ (must match Python name) */
//     //                           /* Argument(s) */
//     //     );
//     // }

//     std::shared_ptr<STI::Device::DeviceMessageDispatcher> getMessageDispatcher() override
//     {
//         PYBIND11_OVERRIDE_PURE(
//             std::shared_ptr<STI::Device::DeviceMessageDispatcher>, /* Return type */
//             DevicePy,      /* Parent class */
//             getMessageDispatcher,          /* Name of function in C++ (must match Python name) */
//                               /* Argument(s) */
//         );
//     }

    
//     std::shared_ptr<ChannelManagerPy> getChannelManager() override
//     {
//         PYBIND11_OVERRIDE_PURE(
//             std::shared_ptr<ChannelManagerPy>, /* Return type */
//             DevicePy,      /* Parent class */
//             getChannelManager,          /* Name of function in C++ (must match Python name) */
//                               /* Argument(s) */
//         );
//     }



//     STI::Device::DeviceID getIDpy() override
//     {
//         PYBIND11_OVERRIDE_PURE(
//             STI::Device::DeviceID, /* Return type */
//             DevicePy,      /* Parent class */
//             getIDpy,          /* Name of function in C++ (must match Python name) */
//                               /* Argument(s) */
//         );
//     }

//     int test2(int x) override
//     {
//         PYBIND11_OVERRIDE_PURE(
//             int, /* Return type */
//             DevicePy,      /* Parent class */
//             test2,          /* Name of function in C++ (must match Python name) */
//             x                  /* Argument(s) */
//         );
//     }

// };




} //Python
} //STI

#endif

