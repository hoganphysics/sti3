
#include "JDevice.h"
#include "Device.h"
#include "DeviceID.h"
#include "LocalDevice.h"
#include "JDeviceCollection.h"
#include "JDeviceEventDispatcher.h"
#include "JEventEngineScheduler.h"
#include "ChannelManager.h"

#include "DeviceEventDispatcher.h"

#include <memory>

using STI::Device::JDevice;
using STI::Device::LocalDevice;
using STI::Device::DeviceID;
using STI::Device::DeviceCollection;
using STI::Device::JDeviceCollection;
using STI::Device::DeviceEventDispatcher;
using STI::Device::JDeviceEventDispatcher;
using STI::Device::JEventEngineScheduler;
using STI::Device::ChannelManager;


JDevice::JDevice(const std::shared_ptr<STI::Device::Device>& device)
{
    wrappedDevice = device;     //could be network or local
}

JDevice::JDevice(const std::string& name, const std::string& address, unsigned short module, const std::string& targetServer)
{
    wrappedDevice = std::make_shared<LocalDevice>(name, address, module, targetServer);
}

JDevice::~JDevice()
{
}

const DeviceID JDevice::getID() const
{
    if(wrappedDevice != 0) {
        return wrappedDevice->getID();
    }
    return DeviceID("","",0,"");
}

std::shared_ptr<STI::Device::JDeviceCollection> JDevice::getCollection()
{
    std::shared_ptr<STI::Device::JDeviceCollection> jCollection;

    if(wrappedDevice != 0) {
        std::shared_ptr<STI::Device::DeviceCollection> collection;
        wrappedDevice->getCollection(collection);

        jCollection = std::make_shared<STI::Device::JDeviceCollection>(collection);
    }

    return jCollection;
}

std::shared_ptr<STI::Device::JDeviceEventDispatcher> JDevice::getEventDispatcher()
{
    std::shared_ptr<STI::Device::JDeviceEventDispatcher> jDispatcher;

    if(wrappedDevice != 0) {
        std::shared_ptr<STI::Device::DeviceEventDispatcher> dispatcher;
        wrappedDevice->getEventDispatcher(dispatcher);

        jDispatcher = std::make_shared<STI::Device::JDeviceEventDispatcher>(dispatcher);
    }

    return jDispatcher;
}

std::shared_ptr<STI::Device::JEventEngineScheduler> JDevice::getEngineScheduler()
{
    std::shared_ptr<STI::Device::JEventEngineScheduler> jScheduler;

    if(wrappedDevice != 0) {
        std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
        wrappedDevice->getEngineScheduler(scheduler);

        jScheduler = std::make_shared<STI::Device::JEventEngineScheduler>(scheduler);
    }

    return jScheduler;
}

std::shared_ptr<STI::Device::ChannelManager> JDevice::getChannelManager()
{
    std::shared_ptr<STI::Device::ChannelManager> manager;
    
    if(wrappedDevice != 0) {
        wrappedDevice->getChannelManager(manager);
    }

    return manager;
}

void JDevice::getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection)
{
    if(wrappedDevice != 0) {
        wrappedDevice->getCollection(collection);
    }
}

void JDevice::getEventDispatcher(std::shared_ptr<DeviceEventDispatcher>& dispatcher)
{
    if(wrappedDevice != 0) {
        wrappedDevice->getEventDispatcher(dispatcher);
    }
}

bool JDevice::getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
{
    if(wrappedDevice != 0) {
        wrappedDevice->getEngineScheduler(scheduler);
    }
}

void JDevice::getChannelManager(std::shared_ptr<ChannelManager>& manager)
{
    if(wrappedDevice != 0) {
        wrappedDevice->getChannelManager(manager);
    }
}



bool JDevice::refresh()
{
    if(wrappedDevice != 0) {
        return wrappedDevice->refresh();
    }
    return false;
}

