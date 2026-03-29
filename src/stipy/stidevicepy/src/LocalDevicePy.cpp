#include "LocalDevicePy.h"
#include "DevicePy.h"
#include "ChannelManagerPy.h"
#include "LocalLogManager.h"
#include "LocalMonitorManager.h"
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/utils/MixedValue.h>
#include <sti/engine/SynchronousEvent.h>

#include "SynchronousEventPy.h"
#include "SynchronousEventPyManager.h"

#include <stdexcept>
#include <thread>

#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Python::LocalDevicePy;
using STI::Device::AutoMonitor;
using STI::Device::LocalDevice;
using STI::Python::DevicePy;
using STI::Python::ChannelManagerPy;
using STI::Device::ChannelManager;
using STI::Device::LocalMonitor;
using STI::Device::LocalMonitorManager;
using STI::Device::Monitor;
using STI::Utils::MixedValue;


LocalDevicePy::LocalDevicePy(const std::map<std::string, std::string>& config)
: DevicePy()
{
    device = std::make_shared<LocalDevicePy::LocalDeviceDelegate>(this, config);
    setDevice(device);
}

LocalDevicePy::LocalDevicePy(const STI::Utils::Configuration& config, const std::string& section)
: DevicePy()
{
    device = std::make_shared<LocalDevicePy::LocalDeviceDelegate>(this, config, section);
    setDevice(device);
}

LocalDevicePy::LocalDevicePy(const std::string& name, const std::string& address, unsigned short module, 
                             const std::string& targetServer, const STI::Utils::Configuration& config)
: DevicePy()
{
    device = std::make_shared<LocalDevicePy::LocalDeviceDelegate>(this, name, address, module, targetServer, config);
    setDevice(device);
}

LocalDevicePy::~LocalDevicePy()
{
    if (device != 0) {
        device->disable();
    }
}

//Can be overridden in python
bool LocalDevicePy::writeChannel(short channel, const pybind11::object& value)
{
    bool result = false;
    auto mValue = MixedValuePy(value);

    {
        //Need to run in separate thread; release python GIL
        py::gil_scoped_release release;
        result = (device != 0) && device->writeChannelDefault(channel, mValue.getMixedValue() );
    }

    return result;
}

//Can be overridden in python
pybind11::object LocalDevicePy::readChannel(short channel, const pybind11::object& value)
{
    MixedValue data;
    MixedValuePy mValue(value);

    bool success = false;

    {
        //Need to run in separate thread; release python GIL
        py::gil_scoped_release release;
        success = (device != 0) && device->readChannelDefault(channel, mValue.getMixedValue(), data);
    }

    if (success) {

        MixedValuePy pydata;
        pydata.setValue(data);

        pybind11::object obj = pydata.getValue_py();

        return obj;
    }
    return py::none();
}

void LocalDevicePy::parseEventsDefault(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
{
    if (device == 0) {
        return;
    }
    device->parseEventsDefault(events, synchedEvents);
}


std::shared_ptr<STI::Device::LocalChannel> LocalDevicePy::addChannel(unsigned short channelNumber, STI::Device::ChannelType type,
    STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName)
{
    std::shared_ptr<STI::Device::LocalChannel> channel;
    device->addChannel(channelNumber, type, inputType, outputType, defaultName, channel);
    return channel;
}

std::shared_ptr<STI::Device::LocalChannel> LocalDevicePy::addInputChannel(unsigned short channelNumber, 
                                                            STI::Utils::MixedValueType inputType, const std::string& defaultName)
{
    return addChannel(channelNumber, STI::Device::ChannelType::Input, inputType, STI::Utils::MixedValueType::Empty, defaultName);
}

std::shared_ptr<STI::Device::LocalChannel> LocalDevicePy::addInputChannel(unsigned short channelNumber, STI::Utils::MixedValueType inputType, 
                                                            STI::Utils::MixedValueType outputType, const std::string& defaultName)
{
    return addChannel(channelNumber, STI::Device::ChannelType::Input, inputType, outputType, defaultName);
}

std::shared_ptr<STI::Device::LocalChannel> LocalDevicePy::addOutputChannel(unsigned short channelNumber, STI::Utils::MixedValueType outputType, const std::string& defaultName)
{
    return addChannel(channelNumber, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, outputType, defaultName);
}

void LocalDevicePy::addEventEngine(const STI::Engine::EngineID& engineID)
{
    device->addEventEngine(engineID);
}

void LocalDevicePy::addPartner(const STI::Device::DeviceID& id)
{
    device->addPartner(id);
}

void LocalDevicePy::addPartner(const STI::Device::DeviceID& id, const std::string& alias)
{
    device->addPartner(id, alias);
}

void LocalDevicePy::addEventTarget(const STI::Device::DeviceID& id)
{
    device->addEventTarget(id);
}

void LocalDevicePy::addEventTarget(const STI::Device::DeviceID& id, const std::string& alias)
{
    device->addEventTarget(id, alias);
}

std::shared_ptr<STI::Device::LocalAttribute> LocalDevicePy::addAttribute(const std::string& key, const std::string& initialValue)
{
    std::shared_ptr<STI::Device::LocalAttribute> attribute;
    device->addAttribute(key, initialValue, attribute);
    return attribute;
}

std::shared_ptr<STI::Device::LocalAttribute> LocalDevicePy::addAttribute(const std::string& key, const std::string& initialValue, const std::vector<std::string>& allowedValues)
{
    std::shared_ptr<STI::Device::LocalAttribute> attribute;
    device->addAttribute(key, initialValue, allowedValues, attribute);
    return attribute;
}

std::shared_ptr<LocalMonitor> LocalDevicePy::addMonitor(const std::string& id)
{
    std::shared_ptr<LocalMonitorManager> manager;

    if (device == 0 || !device->getMonitorManager(manager) || manager == 0) {
        return nullptr;
    }

    device->addMonitor(id);

    std::shared_ptr<Monitor> monitor;

    if (!manager->getMonitor(id, monitor) || monitor == 0) {
        return nullptr;
    }

    return std::dynamic_pointer_cast<LocalMonitor>(monitor);
}

std::shared_ptr<LocalMonitor> LocalDevicePy::addMonitor(const std::shared_ptr<LocalMonitor>& monitor)
{
    std::shared_ptr<LocalMonitorManager> manager;

    if (device == 0 || monitor == 0 || !device->getMonitorManager(manager) || manager == 0) {
        return nullptr;
    }

    const auto id = monitor->getID();

    manager->addMonitor(monitor);

    std::shared_ptr<Monitor> storedMonitor;

    if (!manager->getMonitor(id, storedMonitor) || storedMonitor == 0) {
        return nullptr;
    }

    return std::dynamic_pointer_cast<LocalMonitor>(storedMonitor);
}

std::shared_ptr<AutoMonitor> LocalDevicePy::addAutoMonitor(
    const std::string& id,
    double updateInterval_s,
    const std::function<pybind11::object(void)>& updater)
{
    if (device == 0) {
        return nullptr;
    }
    if (!updater) {
        throw std::invalid_argument("AutoMonitor requires a valid updater function.");
    }

    auto gil_updater = [updater]() {
        pybind11::gil_scoped_acquire acquire;

        try {
            MixedValuePy result(updater());
            return STI::Utils::MixedValue(result.getMixedValue());
        }
        catch (pybind11::error_already_set& e) {
            e.discard_as_unraisable("AutoMonitor updater");
            return STI::Utils::MixedValue();
        }
    };

    std::shared_ptr<AutoMonitor> monitor;
    device->addAutoMonitor(id, updateInterval_s, gil_updater, monitor);
    return monitor;
}

void LocalDevicePy::addTask(const std::shared_ptr<STI::Utils::Task>& task)
{
    if (task == 0) return;

    std::thread addThread([this, task]() {
        pybind11::gil_scoped_acquire acquire;
        device->addTask(task);
    });
    addThread.detach();
}

void LocalDevicePy::addTask(const std::shared_ptr<STI::Python::TaskPy>& task)
{
    if (task == 0) return;

    std::thread addThread([this, task]() {
        pybind11::gil_scoped_acquire acquire;
        device->addTask(task);
    });
    addThread.detach();
}

void LocalDevicePy::addTask(const std::shared_ptr<STI::Python::TaskPy>& task, const pybind11::object& taskObj)
{
    if (task == 0) return;

    pybind11::gil_scoped_acquire acquire;
    task->task_object = taskObj;
    addTask(task);
}

std::shared_ptr<STI::Device::Logger> LocalDevicePy::log()
{
    return log("");
}

std::shared_ptr<STI::Device::Logger> LocalDevicePy::log(const std::string& name)
{
    device->log(name);    // Ensure log exists (auto creates if needed)

    std::shared_ptr<STI::Device::LocalLogManager> manager;
    std::shared_ptr<STI::Device::Logger> logger;

    device->getLogManager(manager);
    manager->getLogger(name, logger);
    return logger;
}


STI::Python::PartnerDevicePy LocalDevicePy::partner(const STI::Device::DeviceID& id)
{
    PartnerDevicePy partner(device->partner(id));
    return partner;
}

STI::Python::PartnerDevicePy LocalDevicePy::partner(const std::string& alias)
{
    PartnerDevicePy partner(device->partner(alias));
    return partner;
}

STI::Engine::EngineParsingMessage& LocalDevicePy::addInfo(unsigned id, const std::string& name)
{
    return device->addInfo(id, name);
}
STI::Engine::EngineParsingMessage& LocalDevicePy::addWarning(unsigned id, const std::string& name)
{
    return device->addWarning(id, name);
}

void LocalDevicePy::throwConflictException(const STI::Engine::RawEvent& evt, const std::string& message)
{
    STI::Engine::EventConflictException exception(evt, message);
    throw exception;
}

void LocalDevicePy::throwConflictException(const STI::Engine::RawEvent& event1, const STI::Engine::RawEvent& event2, const std::string& message)
{
    cachedExceptions.addConflictException(event1, event2, message);
}

void LocalDevicePy::throwParsingException(const STI::Engine::RawEvent& evt, const std::string& message)
{
    cachedExceptions.addParsingException(evt, message);
}

void LocalDevicePy::throwPythonException(const std::string& message)
{
    cachedExceptions.addPythonException(message);
}

std::shared_ptr<STI::Python::DeviceMessageReceiverPy> LocalDevicePy::getMessageReceiver()
{
    if (messageReceiverPy == 0) {
        std::shared_ptr<STI::Device::DeviceMessageReceiver> receiver;
        device->getMessageReceiver(receiver);

        messageReceiverPy = std::make_shared<STI::Python::DeviceMessageReceiverPy>(receiver);
    }

    return messageReceiverPy;
}


///////////////////// CachedExceptions ///////////////////////////


LocalDevicePy::CachedExceptions::CachedExceptions()
{
    clear();
}

void LocalDevicePy::CachedExceptions::clear()
{
    conflictCount = 0;
    parseCount = 0;
    conflictException = 0;
    parseException = 0;
    pyException = 0;
    pyExceptCount = 0;
}

void LocalDevicePy::CachedExceptions::throwException()
{
    if (pyExceptCount > 0 && pyException != 0) {
        throw *pyException;
    }
    else if (parseCount > 0 && parseException != 0) {
        throw *parseException;
    }
    else if (conflictCount > 0 && conflictException != 0) {
        throw *conflictException;
    }
}

void LocalDevicePy::CachedExceptions::addConflictException(const STI::Engine::RawEvent& event1, const STI::Engine::RawEvent& event2, const std::string& message)
{
    conflictException = std::make_shared<STI::Engine::EventConflictException>(event1, event2, message);
    conflictCount++;
}

void LocalDevicePy::CachedExceptions::addParsingException(const STI::Engine::RawEvent& evt, const std::string& message)
{
    parseException = std::make_shared<STI::Engine::EventParsingException>(evt, message);
    parseCount++;
}

void LocalDevicePy::CachedExceptions::addPythonException(const std::string& message)
{
    pyException = std::make_shared<STI::Engine::STI_Exception>("Python Exception", message);
    pyExceptCount++;
}



////////////////// LocalDevicePy::LocalDeviceDelegate /////////////////////


LocalDevicePy::LocalDeviceDelegate::LocalDeviceDelegate(LocalDevicePy* localDevicePy, const std::map<std::string, std::string>& config)
: STI::Device::LocalDevice(config), localDevicePy(localDevicePy)
{
}

LocalDevicePy::LocalDeviceDelegate::LocalDeviceDelegate(LocalDevicePy* localDevicePy, const STI::Utils::Configuration& config, 
                                                        const std::string& section)
: STI::Device::LocalDevice(config, section), localDevicePy(localDevicePy)
{
}

LocalDevicePy::LocalDeviceDelegate::LocalDeviceDelegate(
                    LocalDevicePy* localDevicePy, 
                    const std::string& name, const std::string& address, unsigned short module,
                    const std::string& targetServer, 
                    const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(name, address, module, targetServer, config), localDevicePy(localDevicePy) 
{
}

bool LocalDevicePy::LocalDeviceDelegate::writeChannel(short channel, const STI::Utils::MixedValue& value)
{
    pybind11::gil_scoped_acquire acquire;

    STI::Python::MixedValuePy valuePy(value);
    pybind11::object valuePyObj = valuePy.getValue_py();    //Must create python object before releasing GIL

    bool success = false;
    {
        pybind11::gil_scoped_release release;
        success = localDevicePy->writeChannel(channel, valuePyObj);
    }
    
    return success;
}

bool LocalDevicePy::LocalDeviceDelegate::readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data) 
{
    pybind11::gil_scoped_acquire acquire;

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

void LocalDevicePy::LocalDeviceDelegate::parseEvents(const STI::Engine::RawEventMap& events, STI::Engine::SynchronousEventVector& synchedEvents)
{
    std::unique_lock<std::mutex> parseLock(STI::Python::SynchronousEventPy::pyEventManagerMutex);

    bool error = false;

    pybind11::gil_scoped_acquire acquire;
   
    auto manager = std::make_shared<STI::Python::SynchronousEventPyManager>();
    STI::Python::SynchronousEventPy::pyEventManager = manager;      //temporarily store manager in static member        

    if (localDevicePy != 0) {
        localDevicePy->cachedExceptions.clear();

        try {
            localDevicePy->parseEventsWrapper(events, synchedEvents);
        }
        catch (py::error_already_set& e) {
            error = true;   //cleanup first, then throw exception below
        }
    }

    double holderEventTime = 100;
    if (synchedEvents.size() > 0) {
        std::sort(synchedEvents.begin(), synchedEvents.end(), STI::Utils::compare_shared_ptr<STI::Engine::SynchronousEvent>);
        holderEventTime = synchedEvents.back()->getTime() + 100;
    }

    auto managerHolderEvent = std::make_shared<STI::Python::SynchronousEventPyManagerHolder>(holderEventTime, manager);
    synchedEvents.push_back(managerHolderEvent);   

    STI::Python::SynchronousEventPy::pyEventManager = 0;    //clear static member reference

    if (error && localDevicePy != 0) {
        localDevicePy->cachedExceptions.throwException();
    }
}
