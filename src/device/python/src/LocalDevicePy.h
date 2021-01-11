
#ifndef STI_PYTHON_LOCALDEVICEPY_H
#define STI_PYTHON_LOCALDEVICEPY_H

#include "LocalDevice.h"

//#include "MixedValuePy.h"
#include "DevicePy.h"
#include <memory>

#include <pybind11/pybind11.h>

namespace STI
{
namespace Python
{



class Dog2 : public Animal2 {
public:
    std::string go(int n_times) override {
        std::string result;
        for(int i=0; i<n_times; ++i)
            result += "woof! ";
        return result;
    }
    
    std::string getID() override {
        std::string result;
        for(int i=0; i<5; ++i)
            result += "hi! ";
        return result;
    }
};




class DevicePy;
class ChannelManagerPy;
class MixedValuePy;


class LocalDevicePy : public DevicePy, public STI::Device::LocalDevice 
                                                        //,      //For python wrapper
//                                          public std::enable_shared_from_this<LocalDevicePy>    //needed to construct DevicePy
{
public:

//    LocalDevicePy();
    LocalDevicePy(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
    virtual ~LocalDevicePy();

//    const STI::Device::DeviceID test() const { return LocalDevice::getID(); }

    //DevicePy
//    const STI::Device::DeviceID getIDpy() const override;
//    std::shared_ptr<STI::Device::DeviceMessageDispatcher> getMessageDispatcher() override;
//    std::shared_ptr<EventEngineSchedulerPy> getEngineScheduler();    
//    std::shared_ptr<ChannelManagerPy> getChannelManager() override;

    std::shared_ptr<STI::Device::DeviceMessageDispatcher> getMessageDispatcher() override;
    
    std::shared_ptr<ChannelManagerPy> getChannelManager() override;

    STI::Device::DeviceID getIDpy() override;

    int test2(int x) override
    {
        int y = 3*x;
        return y;
    }

    //Hooks to be implemented in python:
//    virtual bool writeChannelPy(short channel, const STI::Python::MixedValuePy& value) = 0;
//    virtual pybind11::object readChannelPy(short channel, const STI::Python::MixedValuePy& value) = 0;

private:

    //Overrides for STI::Device::LocalDevice
	bool writeChannel(short channel, const STI::Utils::MixedValue& value);
	bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);


};


// class LocalDevicePyTrampoline : public LocalDevicePy {
// public:
//     /* Inherit the constructors */
//     using LocalDevicePy::LocalDevicePy;

//     /* Trampoline (need one for each virtual function) */
//     const STI::Device::DeviceID getIDpy() const override { PYBIND11_OVERRIDE(const STI::Device::DeviceID, LocalDevicePy, getIDpy,); }
//     int test2(int x) override { PYBIND11_OVERRIDE(int, LocalDevicePy, test2, x); }



// };




} //Python
} //STI

#endif

