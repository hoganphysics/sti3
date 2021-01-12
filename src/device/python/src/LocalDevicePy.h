
#ifndef STI_PYTHON_LOCALDEVICEPY_H
#define STI_PYTHON_LOCALDEVICEPY_H

#include "LocalDevice.h"

//#include "MixedValuePy.h"
#include "DevicePy.h"
#include "DeviceID.h"

#include <memory>

#include <pybind11/pybind11.h>

#include <iostream>

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



class LocalDevicePy2 : public DevicePy2
{
public:

    LocalDevicePy2(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
    virtual ~LocalDevicePy2();

    virtual int test2(int x) { return 5; }

    virtual bool writeChannel(short channel, const pybind11::object& value);
    virtual pybind11::object readChannel(short channel, const pybind11::object& value);

    void addChannel(unsigned short channelNumber)
    {
        device->addChannel(channelNumber, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Double, STI::Utils::MixedValueType::Double, "Temp");
    }
    
    void addPartner(const STI::Device::DeviceID& id)
    {
        device->addPartner(id);
    }

private:



    class PyLocalDevice : public STI::Device::LocalDevice
    {
    public:

        PyLocalDevice(LocalDevicePy2* localDevicePy, const std::string& name, const std::string& address, unsigned short module,
		    const std::string& targetServer)
            : STI::Device::LocalDevice(name, address, module, targetServer), localDevicePy(localDevicePy) {}
        

        bool writeChannel(short channel, const STI::Utils::MixedValue& value)
        {
            //std::cout << "writeChannel" << std::endl;

            STI::Python::MixedValuePy valuePy(value);
            // STI::Python::MixedValuePy valuePy;
            // valuePy.setValue(34);

            bool success = localDevicePy->writeChannel(channel, valuePy.getValue_py());
            return success;
        }

	    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data) 
        {
//            std::cout << "readChannel" << std::endl;

            STI::Python::MixedValuePy valuePy(value);
            auto dataPyObj = localDevicePy->readChannel(channel, valuePy.getValue_py());

//            std::cout << "auto dataPyObj" << std::endl;

            //convert result
            STI::Python::MixedValuePy dataPy;
            dataPy.setValue_py(dataPyObj);
//            std::cout << "setValue_py" << std::endl;
            const STI::Utils::MixedValue& ref = dataPy;
//            std::cout << "ref" << std::endl;
            data.setValue(ref);
            //data = dataPy;
//            data.setValue(static_cast<STI::Utils::MixedValue>(dataPy));

//            std::cout << "data.setValue" << std::endl;

            return true;
        }
    
    private:

        LocalDevicePy2* localDevicePy;
    };

    std::shared_ptr<STI::Device::LocalDevice> device;

};




class LocalDevicePy2Trampoline : public LocalDevicePy2 {
public:
    /* Inherit the constructors */
    using LocalDevicePy2::LocalDevicePy2;

    int test2(int x) override
    {
        PYBIND11_OVERRIDE(
            int, /* Return type */
            LocalDevicePy2,      /* Parent class */
            test2,          /* Name of function in C++ (must match Python name) */
            x                  /* Argument(s) */
        );
    }

    bool writeChannel(short channel, const pybind11::object& value) override
    {
        PYBIND11_OVERRIDE(
            bool,                  /* Return type */
            LocalDevicePy2,        /* Parent class */
            writeChannel,          /* Name of function in C++ (must match Python name) */
            channel, value         /* Argument(s) */
        );
    }

    pybind11::object readChannel(short channel, const pybind11::object& value) override
    {
        PYBIND11_OVERRIDE(
            pybind11::object,     /* Return type */
            LocalDevicePy2,       /* Parent class */
            readChannel,          /* Name of function in C++ (must match Python name) */
            channel, value        /* Argument(s) */
        );
    }


};





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

