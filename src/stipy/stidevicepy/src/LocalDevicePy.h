#ifndef STI_PYTHON_LOCALDEVICEPY_H
#define STI_PYTHON_LOCALDEVICEPY_H

#include <sti/LocalDevice.h>
#include <sti/device/AutoMonitor.h>
#include <sti/device/DeviceID.h>

#include "DevicePy.h"
#include "TaskPy.h"
#include "PartnerDevicePy.h"
#include "DeviceMessageReceiverPy.h"

#include <memory>
#include <pybind11/pybind11.h>


namespace STI
{
namespace Python
{

class DevicePy;
class ChannelManagerPy;
class MixedValuePy;
class DeviceMessageReceiverPy;


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

    //for exception handling
    virtual void parseEventsWrapper(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) {}
    void parseEventsDefault(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents);

    std::shared_ptr<STI::Device::LocalChannel> addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
		STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName);
	std::shared_ptr<STI::Device::LocalChannel> addInputChannel(unsigned short channelNumber, 
                                                               STI::Utils::MixedValueType inputType, const std::string& defaultName);
	std::shared_ptr<STI::Device::LocalChannel> addInputChannel(unsigned short channelNumber, STI::Utils::MixedValueType inputType, 
                                                               STI::Utils::MixedValueType outputType, const std::string& defaultName);
	std::shared_ptr<STI::Device::LocalChannel> addOutputChannel(unsigned short channelNumber, STI::Utils::MixedValueType outputType, 
                                                                const std::string& defaultName);

    void addEventEngine(const STI::Engine::EngineID& engineID);
    void addPartner(const STI::Device::DeviceID& id);
    void addPartner(const STI::Device::DeviceID& id, const std::string& alias);
    void addEventTarget(const STI::Device::DeviceID& id, const std::string& alias);
    void addEventTarget(const STI::Device::DeviceID& id);

    std::shared_ptr<STI::Device::LocalAttribute> addAttribute(const std::string& key, const std::string& initialValue);
    std::shared_ptr<STI::Device::LocalAttribute> addAttribute(const std::string& key, const std::string& initialValue, const std::vector<std::string>& allowedValues);
    std::shared_ptr<STI::Device::LocalMonitor> addMonitor(const std::string& id);
    std::shared_ptr<STI::Device::LocalMonitor> addMonitor(const std::shared_ptr<STI::Device::LocalMonitor>& monitor);
    std::shared_ptr<STI::Device::AutoMonitor> addAutoMonitor(
        const std::string& id,
        double updateInterval_s,
        const std::function<pybind11::object(void)>& updater);

    void addTask(const std::shared_ptr<STI::Utils::Task>& task);
    void addTask(const std::shared_ptr<STI::Python::TaskPy>& task);

    void addTask(const std::shared_ptr<STI::Python::TaskPy>& task, const pybind11::object& taskObj);

    std::shared_ptr<STI::Device::Logger> log();
    std::shared_ptr<STI::Device::Logger> log(const std::string& name);

	PartnerDevicePy partner(const STI::Device::DeviceID& id);
	PartnerDevicePy partner(const std::string& alias);

	STI::Engine::EngineParsingMessage& addInfo(unsigned id, const std::string& name);
	STI::Engine::EngineParsingMessage& addWarning(unsigned id, const std::string& name);

    void throwConflictException(const STI::Engine::RawEvent& evt, const std::string& message);
    void throwConflictException(const STI::Engine::RawEvent& event1, const STI::Engine::RawEvent& event2, const std::string& message);
    void throwParsingException(const STI::Engine::RawEvent& evt, const std::string& message);
    void throwPythonException(const std::string& message);

    std::shared_ptr<STI::Python::DeviceMessageReceiverPy> getMessageReceiver();   

private:

    class CachedExceptions
    {
    public:

        CachedExceptions();

        void clear();
        void throwException();

        void addConflictException(const STI::Engine::RawEvent& event1, const STI::Engine::RawEvent& event2, const std::string& message);
        void addParsingException(const STI::Engine::RawEvent& evt, const std::string& message);
        void addPythonException(const std::string& message);

    private:

        int conflictCount;
        int parseCount;
        int pyExceptCount;
        
        std::shared_ptr<STI::Engine::EventConflictException> conflictException;
        std::shared_ptr<STI::Engine::EventParsingException> parseException;
        std::shared_ptr<STI::Engine::STI_Exception> pyException;
    };

    CachedExceptions cachedExceptions;


    class LocalDeviceDelegate : public STI::Device::LocalDevice
    {
    public:

        LocalDeviceDelegate(LocalDevicePy* localDevicePy, const std::map<std::string, std::string>& config);
        LocalDeviceDelegate(LocalDevicePy* localDevicePy, const STI::Utils::Configuration& config, const std::string& section="");
        LocalDeviceDelegate(LocalDevicePy* localDevicePy, 
                            const std::string& name, const std::string& address, unsigned short module,
                            const std::string& targetServer, 
                            const STI::Utils::Configuration& config=STI::Utils::Configuration());

        bool writeChannel(short channel, const STI::Utils::MixedValue& value);
	    bool readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data);
        void parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents);
    
    private:

        LocalDevicePy* localDevicePy;
    };

    std::shared_ptr<STI::Device::LocalDevice> device;
    std::shared_ptr<STI::Python::DeviceMessageReceiverPy> messageReceiverPy;
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

    void parseEventsWrapper(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents) override
    {        
        pybind11::gil_scoped_acquire acquire;
        
        pybind11::object dummy = pybind11::cast(synchedEvents, pybind11::return_value_policy::reference);

        PYBIND11_OVERRIDE(
            void,                   /* Return type */
            LocalDevicePy,          /* Parent class */
            parseEventsWrapper,     /* Name of function in C++ (must match Python name) */
            events, synchedEvents   /* Argument(s) */
        );
    }

};


} //Python
} //STI

#endif
