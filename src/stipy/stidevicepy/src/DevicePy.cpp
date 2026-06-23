
#include "DevicePy.h"

#include <sti/device/Attribute.h>
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include "ChannelManagerPy.h"
#include <sti/device/ChannelManager.h>
#include <sti/device/DeviceCollection.h>
#include <sti/device/ProfileManager.h>
#include <sti/device/TaskManager.h>
#include <sti/device/VersionManager.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/utils/MixedValue.h>
// #include "EventEngineSchedulerPy.h"
#include "AttributeManagerPy.h"
#include "MonitorManagerPy.h"
#include "PersistenceManagerPy.h"

#include <pybind11/pybind11.h>
namespace py = pybind11;

using STI::Python::DevicePy;
using STI::Python::ChannelManagerPy;
using STI::Device::ChannelManager;
using STI::Python::DeviceCollectionPy;
// using STI::Python::EventEngineSchedulerPy;
using STI::Python::AttributeManagerPy;
using STI::Device::AttributeManager;
using STI::Python::MonitorManagerPy;
using STI::Device::MonitorManager;
using STI::Python::PersistenceManagerPy;
using STI::Device::PersistenceManager;
using STI::Engine::EventEngineScheduler;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;
using STI::Device::ProfileManager;
using STI::Device::LogManager;
using STI::Device::VersionManager;


DevicePy::DevicePy(const std::shared_ptr<STI::Device::Device>& device)
: device_(device)
{
}

//DevicePy::~DevicePy()
//{
//}

void DevicePy::setDevice(const std::shared_ptr<STI::Device::Device>& device)
{
    device_ = device;
}

std::shared_ptr<STI::Device::Device> DevicePy::getDevice()
{
    return device_;
}

const STI::Device::DeviceID DevicePy::getID() const
{
    if (device_ != 0) {
        return device_->getID();
    }

    STI::Device::DeviceID dummy;
    return dummy;
}

std::vector<STI::Device::PartnerDeviceInfo> DevicePy::getPartnerDevices() const
{
    std::vector<STI::Device::PartnerDeviceInfo> partners;

    if (device_ != 0) {
        device_->getPartnerDevices(partners);
    }

    return partners;
}

void DevicePy::kill()
{
    if (device_ != 0) {
        return device_->kill();
    }
}

bool DevicePy::refresh()
{
    if (device_ != 0) {
        return device_->refresh();
    }
    return false;
}

std::shared_ptr<STI::Python::DeviceCollectionPy> DevicePy::getDeviceCollection()
{
    std::shared_ptr<STI::Device::DeviceCollection> collection;
    std::shared_ptr<STI::Python::DeviceCollectionPy> wrapper;

    if (device_ != 0) {
        device_->getCollection(collection);
    }

    if (collection != 0) {
        wrapper = std::make_shared<DeviceCollectionPy>(collection);
    }

    return wrapper;
}

std::shared_ptr<STI::Device::DeviceMessageDispatcher> DevicePy::getMessageDispatcher()
{
    std::shared_ptr<STI::Device::DeviceMessageDispatcher> dispatcher;
    
    if (device_ != 0) {
        device_->getMessageDispatcher(dispatcher);
    }

    return dispatcher;
}

std::shared_ptr<EventEngineScheduler> DevicePy::getEngineScheduler()
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

    if (device_ != 0) {
        device_->getEngineScheduler(scheduler);
    }
    
    return scheduler;
}

std::shared_ptr<ChannelManagerPy> DevicePy::getChannelManager()
{
    std::shared_ptr<ChannelManager> manager;
    std::shared_ptr<ChannelManagerPy> wrapper;

    if (device_ != 0) {
        device_->getChannelManager(manager);
    }

    if (manager != 0) {
        wrapper = std::make_shared<ChannelManagerPy>(manager);
    }

    return wrapper;
}


std::shared_ptr<AttributeManagerPy> DevicePy::getAttributeManager()
{
    std::shared_ptr<AttributeManager> manager;
    std::shared_ptr<AttributeManagerPy> wrapper;

    if (device_ != 0) {
        device_->getAttributeManager(manager);
    }

    if (manager != 0) {
        wrapper = std::make_shared<AttributeManagerPy>(manager);
    }

    return wrapper;
}

std::shared_ptr<MonitorManagerPy> DevicePy::getMonitorManager()
{
    std::shared_ptr<MonitorManager> manager;

    if (device_ != 0) {
        device_->getMonitorManager(manager);
    }

    if (manager != 0) {
        return std::make_shared<MonitorManagerPy>(manager);
    }

    return nullptr;
}

std::shared_ptr<PersistenceManagerPy> DevicePy::getPersistenceManager()
{
    std::shared_ptr<STI::Device::PersistenceManager> manager;
    std::shared_ptr<STI::Python::PersistenceManagerPy> wrapper;

    if (device_ != 0) {
        device_->getPersistenceManager(manager);
    }

    if (manager != 0) {
        wrapper = std::make_shared<PersistenceManagerPy>(manager);
    }

    return wrapper;
}

std::shared_ptr<ProfileManager> DevicePy::getProfileManager()
{
    std::shared_ptr<ProfileManager> manager;

    if (device_ != 0) {
        device_->getProfileManager(manager);
    }

    return manager;
}

std::shared_ptr<STI::Device::TaskManager> DevicePy::getTaskManager()
{
    std::shared_ptr<STI::Device::TaskManager> manager;

    if (device_ != 0) {
        device_->getTaskManager(manager);
    }

    return manager;
}

std::shared_ptr<STI::Device::LogManager> DevicePy::getLogManager()
{
    std::shared_ptr<LogManager> manager;

    if (device_ != 0) {
        device_->getLogManager(manager);
    }

    return manager;
}

std::shared_ptr<STI::Device::VersionManager> DevicePy::getVersionManager()
{
    std::shared_ptr<VersionManager> manager;

    if (device_ != 0) {
        device_->getVersionManager(manager);
    }

    return manager;
}

bool DevicePy::write(short channel, const pybind11::object& value)
{
    MixedValuePy valuepy(value);
    return write(channel, valuepy);
}

bool DevicePy::write(short channel, const MixedValuePy& valuepy)
{
    return (device_ != 0) && device_->write(channel, valuepy.getMixedValue());
}

pybind11::object DevicePy::read(short channel)
{
    return read(channel, pybind11::none());
}

pybind11::object DevicePy::read(short channel, const pybind11::object& value)
{
    MixedValuePy valuepy(value);
    return read(channel, valuepy);
}

pybind11::object DevicePy::read(short channel, const MixedValuePy& valuepy)
{
    MixedValue data;

    bool success = (device_ != 0) && device_->read(channel, valuepy.getMixedValue(), data);

    if (success) {
        pybind11::gil_scoped_acquire acquire;
        return MixedValuePy::convertReadResult(data);
    }
    return py::none();
}

void DevicePy::stopRW()
{
    if (device_ != 0) {
        device_->stopRW();
    }
}

pybind11::dict DevicePy::metadata() const
{
    pybind11::dict values;

    if (device_ == 0) {
        return values;
    }

    const auto& metaData = device_->getMetaData();

    if (!metaData.isType(MixedValueType::Vector)) {
        return values;
    }

    for (const auto& tuple : metaData.getVector()) {
        if (!tuple.isType(MixedValueType::Vector)) {
            continue;
        }

        const auto& entry = tuple.getVector();

        if (entry.size() != 2 || !entry.at(0).isType(MixedValueType::String)) {
            continue;
        }

        MixedValuePy value(entry.at(1));
        values[entry.at(0).getString().c_str()] = value.getValue_py();
    }

    return values;
}

pybind11::object DevicePy::metadata(const std::string& key) const
{
    if (device_ != 0) {
        MixedValuePy value(device_->getMetaData(key));
        return value.getValue_py();
    }

    return pybind11::none();
}

std::string DevicePy::getAttribute(const std::string& key)
{
    if (device_ != 0) {
        return device_->getAttribute(key);
    }
    return "";
}

bool DevicePy::getAttribute(const std::string& key, std::shared_ptr<STI::Device::Attribute>& attribute)
{
    if (device_ != 0) {
        return device_->getAttribute(key, attribute);
    }
    return false;
}

bool DevicePy::setAttribute(const std::string& key, const std::string& value)
{
    if (device_ != 0) {
        return device_->setAttribute(key, value);
    }
    return false;
}

bool DevicePy::refreshAttribute(const std::string& key)
{
    std::shared_ptr<AttributeManager> manager;

    if (device_ != 0) {
        device_->getAttributeManager(manager);
    }

    if (manager != 0) {
        return manager->refreshValue(key);
    }
    return false;
}

void DevicePy::refreshAttributes()
{
    std::shared_ptr<AttributeManager> manager;

    if (device_ != 0) {
        device_->getAttributeManager(manager);
    }

    if (manager != 0) {
        manager->refreshValues();
    }
}
