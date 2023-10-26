#ifndef STI_PYTHON_LOCALDEVICEPY_H
#define STI_PYTHON_LOCALDEVICEPY_H

#include <sti/LocalDevice.h>
#include <sti/device/DeviceID.h>

#include "DevicePy.h"
#include "PartnerDevicePy.h"

#include <memory>
#include <pybind11/pybind11.h>


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

    LocalDevicePy(const std::map<std::string, std::string>& config);
	LocalDevicePy(const STI::Utils::Configuration& config, const std::string& section="");
	LocalDevicePy(const std::string& name, const std::string& address, unsigned short module,
                  const std::string& targetServer, 
                  const STI::Utils::Configuration& config=STI::Utils::Configuration());

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
	std::shared_ptr<STI::Device::LocalChannel> addInputChannel(unsigned short channelNumber, 
                                                               STI::Utils::MixedValueType inputType, const std::string& defaultName)
    {
        return addChannel(channelNumber, STI::Device::ChannelType::Input, inputType, STI::Utils::MixedValueType::Empty, defaultName);
    }
	std::shared_ptr<STI::Device::LocalChannel> addInputChannel(unsigned short channelNumber, STI::Utils::MixedValueType inputType, 
                                                               STI::Utils::MixedValueType outputType, const std::string& defaultName)
    {
        return addChannel(channelNumber, STI::Device::ChannelType::Input, inputType, outputType, defaultName);
    }
	std::shared_ptr<STI::Device::LocalChannel> addOutputChannel(unsigned short channelNumber, STI::Utils::MixedValueType outputType, const std::string& defaultName)
    {
        return addChannel(channelNumber, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, outputType, defaultName);
    }

    void addEventEngine(const STI::Engine::EngineID& engineID)
    {
        device->addEventEngine(engineID);
    }

    void addPartner(const STI::Device::DeviceID& id)
    {
        device->addPartner(id);
    }
    
    void addPartner(const STI::Device::DeviceID& id, const std::string& alias)
    {
        device->addPartner(id, alias);
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

    STI::Device::Logger& log()
    {
        return device->log();
    }

    STI::Device::Logger& log(const std::string& name)
    {
        return device->log(name);
    }

	PartnerDevicePy partner(const STI::Device::DeviceID& id)
    {
        PartnerDevicePy partner(device->partner(id));
        return partner;
    }

	PartnerDevicePy partner(const std::string& alias)
    {
        PartnerDevicePy partner(device->partner(alias));
        return partner;
    }

private:

    class LocalDeviceDelegate : public STI::Device::LocalDevice
    {
    public:

        LocalDeviceDelegate(LocalDevicePy* localDevicePy, const std::map<std::string, std::string>& config)
            : STI::Device::LocalDevice(config), localDevicePy(localDevicePy) {}
        LocalDeviceDelegate(LocalDevicePy* localDevicePy, const STI::Utils::Configuration& config, const std::string& section="")
            : STI::Device::LocalDevice(config, section), localDevicePy(localDevicePy) {}
        LocalDeviceDelegate(LocalDevicePy* localDevicePy, 
                            const std::string& name, const std::string& address, unsigned short module,
                            const std::string& targetServer, 
                            const STI::Utils::Configuration& config=STI::Utils::Configuration())
            : STI::Device::LocalDevice(name, address, module, targetServer, config), localDevicePy(localDevicePy) {}

        bool writeChannel(short channel, const STI::Utils::MixedValue& value)
        {
            STI::Python::MixedValuePy valuePy(value);
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
            pybind11::object dataPyObj;
            pybind11::object valuePyObj = valuePy.getValue_py();    //Must create python object before releasing GIL
            
            {
                pybind11::gil_scoped_release release;
                dataPyObj = localDevicePy->readChannel(channel, valuePyObj);
            }           

            //convert result
            STI::Python::MixedValuePy dataPy;
            dataPy.setValue_py(dataPyObj);
            data.setValue(dataPy.getMixedValue());

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

