#ifndef STI_PYTHON_LOCALDEVICEPY_H
#define STI_PYTHON_LOCALDEVICEPY_H

#include <sti/LocalDevice.h>
#include "DevicePy.h"
#include <sti/device/DeviceID.h>

#include <memory>
#include <pybind11/pybind11.h>

#include <iostream>

namespace STI
{
namespace Python
{

class DevicePy;
class ChannelManagerPy;
class MixedValuePy;


class LocalDevicePy : public DevicePy
{
public:

    LocalDevicePy(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer);
    virtual ~LocalDevicePy();

    virtual bool writeChannel(short channel, const pybind11::object& value);
    virtual pybind11::object readChannel(short channel, const pybind11::object& value);

    virtual void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) {}

    std::shared_ptr<STI::Device::LocalChannel> addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName)
    {
        std::shared_ptr<STI::Device::LocalChannel> channel;
        device->addChannel(channelNumber, type, inputType, outputType, defaultName, channel);
        return channel;
    }
    
    void addEventEngine(const STI::Engine::EngineID& engineID)
    {
        device->addEventEngine(engineID);
    }

    void addPartner(const STI::Device::DeviceID& id)
    {
        device->addPartner(id);
    }
    
    void addEventTarget(const STI::Device::DeviceID& id)
    {
        device->addEventTarget(id);
    }    

    std::shared_ptr<STI::Device::LocalAttribute> addAttribute(const std::string& key, const std::string& initialValue)
    {
        std::shared_ptr<STI::Device::LocalAttribute> attribute;
        device->addAttribute(key, initialValue, attribute);
        return attribute;
    }

    std::shared_ptr<STI::Device::LocalAttribute> addAttribute(const std::string& key, const std::string& initialValue, const std::vector<std::string>& allowedValues)
    {
        std::shared_ptr<STI::Device::LocalAttribute> attribute;
        device->addAttribute(key, initialValue, allowedValues, attribute);
        return attribute;
    }

    void addTask(const std::shared_ptr<STI::Utils::Task>& task)
    {
        device->addTask(task);
    }


private:

    class LocalDeviceDelegate : public STI::Device::LocalDevice
    {
    public:

        LocalDeviceDelegate(LocalDevicePy* localDevicePy, const std::string& name, const std::string& address, unsigned short module,
		    const std::string& targetServer)
            : STI::Device::LocalDevice(name, address, module, targetServer), localDevicePy(localDevicePy) {}
        

        bool writeChannel(short channel, const STI::Utils::MixedValue& value)
        {
            STI::Python::MixedValuePy valuePy(value);

            // std::cout << "LocalDeviceDelegate::writeChannel: " << value.print() << std::endl;

            pybind11::object valuePyObj = valuePy.getValue_py();    //Must create python object before releasing GIL

            bool success = false;
            {
                pybind11::gil_scoped_release release;
                success = localDevicePy->writeChannel(channel, valuePyObj);
            }
            
            return success;
        }

	    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data) 
        {
            STI::Python::MixedValuePy valuePy(value);
            // valuePy.setValue(value);
            // valuePy.addValue(33.0);
            pybind11::object dataPyObj;
            pybind11::object valuePyObj = valuePy.getValue_py();    //Must create python object before releasing GIL
            
            {
                // std::cout << "----------- LocalDeviceDelegate::readChannel start " << value.print() << std::endl;

                pybind11::gil_scoped_release release;
                
                dataPyObj = localDevicePy->readChannel(channel, valuePyObj);

                // std::cout << "----------- LocalDeviceDelegate::readChannel done localDevicePy" << std::endl;
            }           

            

            //convert result
            STI::Python::MixedValuePy dataPy;
            dataPy.setValue_py(dataPyObj);

            // const STI::Utils::MixedValue& ref = dataPy;
            // data.setValue(ref);

            data.setValue(dataPy.getMixedValue());

            // std::cout << "read channel: " << data.print() << std::endl;

            return true;
        }

        void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents);
    
    private:

        LocalDevicePy* localDevicePy;
    };

    std::shared_ptr<STI::Device::LocalDevice> device;

};




class LocalDevicePyTrampoline : public LocalDevicePy {
public:
    /* Inherit the constructors */
    using LocalDevicePy::LocalDevicePy;

    bool writeChannel(short channel, const pybind11::object& value) override
    {   
        pybind11::gil_scoped_acquire acquire;

        // std::cout << "LocalDevicePyTrampoline::writeChannel"  << std::endl;

        PYBIND11_OVERRIDE(
            bool,                  /* Return type */
            LocalDevicePy,         /* Parent class */
            writeChannel,          /* Name of function in C++ (must match Python name) */
            channel, value         /* Argument(s) */
        );
    }

    pybind11::object readChannel(short channel, const pybind11::object& value) override
    {
        pybind11::gil_scoped_acquire acquire;
        // pybind11::object dummy = value;
        // pybind11::object value2;

        PYBIND11_OVERRIDE(
            pybind11::object,     /* Return type */
            LocalDevicePy,        /* Parent class */
            readChannel,          /* Name of function in C++ (must match Python name) */
            channel, value        /* Argument(s) */
        );
    }

    void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) override
    {
        pybind11::object dummy = pybind11::cast(synchedEvents, pybind11::return_value_policy::reference);

        PYBIND11_OVERRIDE(
            void,                   /* Return type */
            LocalDevicePy,          /* Parent class */
            parseEvents,            /* Name of function in C++ (must match Python name) */
            events, synchedEvents   /* Argument(s) */
        );
    }


};


} //Python
} //STI

#endif

