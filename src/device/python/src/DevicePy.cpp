
#include "DevicePy.h"
#include "DeviceID.h"
#include "DeviceMessageDispatcher.h"
#include "ChannelManagerPy.h"
#include "ChannelManager.h"
#include "DeviceCollection.h"
#include "EventEngineSchedulerPy.h"
#include "AttributeManagerPy.h"
#include "PersistenceManagerPy.h"

#include <iostream>

using STI::Python::DevicePy;
using STI::Python::ChannelManagerPy;
using STI::Device::ChannelManager;
using STI::Python::DeviceCollectionPy;
using STI::Python::EventEngineSchedulerPy;
using STI::Python::AttributeManagerPy;
using STI::Device::AttributeManager;
using STI::Python::PersistenceManagerPy;
using STI::Device::PersistenceManager;


DevicePy::DevicePy(const std::shared_ptr<STI::Device::Device>& device)
: device_(device)
{
}

DevicePy::~DevicePy()
{
}

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

void DevicePy::kill()
{
    if (device_ != 0) {
        return device_->kill();
    }
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

std::shared_ptr<EventEngineSchedulerPy> DevicePy::getEngineScheduler()
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    std::shared_ptr<STI::Python::EventEngineSchedulerPy> wrapper;

    if (device_ != 0) {
        device_->getEngineScheduler(scheduler);
    }

    if (scheduler != 0) {
        wrapper = std::make_shared<EventEngineSchedulerPy>(scheduler);
    }

    return wrapper;
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

