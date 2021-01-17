
#include "DevicePy.h"
#include "DeviceID.h"
#include "DeviceMessageDispatcher.h"
#include "ChannelManagerPy.h"
#include "ChannelManager.h"
#include "DeviceCollection.h"
#include "EventEngineSchedulerPy.h"


#include <iostream>

using STI::Python::DevicePy2;
using STI::Python::ChannelManagerPy;
using STI::Device::ChannelManager;

using STI::Python::DeviceCollectionPy;
using STI::Python::EventEngineSchedulerPy;

DevicePy2::DevicePy2(const std::shared_ptr<STI::Device::Device>& device)
: device_(device)
{
}

DevicePy2::~DevicePy2()
{
}

void DevicePy2::setDevice(const std::shared_ptr<STI::Device::Device>& device)
{
    device_ = device;
}

std::shared_ptr<STI::Device::Device> DevicePy2::getDevice()
{
//    std::cout << "getDevice() device_ == " << (device_==0 ? "0" : "1") << std::endl;
    return device_;
}

const STI::Device::DeviceID DevicePy2::getID() const
{
    if (device_ != 0) {
        return device_->getID();
    }

    STI::Device::DeviceID dummy;
    return dummy;
}

std::shared_ptr<STI::Python::DeviceCollectionPy> DevicePy2::getDeviceCollection()
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

std::shared_ptr<EventEngineSchedulerPy> DevicePy2::getEngineScheduler()
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

std::shared_ptr<STI::Device::DeviceMessageDispatcher> DevicePy2::getMessageDispatcher()
{
    std::shared_ptr<STI::Device::DeviceMessageDispatcher> dispatcher;
    
    if (device_ != 0) {
        device_->getMessageDispatcher(dispatcher);
    }

    return dispatcher;
}

std::shared_ptr<ChannelManagerPy> DevicePy2::getChannelManager()
{
    std::shared_ptr<ChannelManager> manager;
    std::shared_ptr<ChannelManagerPy> wrapper;

    if (device_ != 0) {
//        std::cout << "device_ != 0" << std::endl;
        device_->getChannelManager(manager);
//        std::cout << "manager == " << (manager==0 ? "0" : "1") << std::endl;
    }

    if (manager != 0) {
//        std::cout << "manager != 0" << std::endl;
        wrapper = std::make_shared<ChannelManagerPy>(manager);
//        std::cout << "manager != 0 and wrapper == " << (wrapper==0 ? "0" : "1") << std::endl;
    }

    return wrapper;
}

