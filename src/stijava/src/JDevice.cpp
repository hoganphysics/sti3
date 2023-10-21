
#include "JDevice.h"
#include <sti/device/Device.h>
#include <sti/device/DeviceID.h>
#include <sti/device/TaskManager.h>
#include <sti/device/LogManager.h>
#include <sti/device/ProfileManager.h>
#include <sti/device/Attribute.h>
#include <sti/LocalDevice.h>

#include "JDeviceCollection.h"
#include "JDeviceMessageDispatcher.h"
#include "JEventEngineScheduler.h"
#include <sti/device/ChannelManager.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include "JChannelManager.h"
#include <sti/device/AttributeManager.h>
#include "JAttributeManager.h"
#include "JEventEngineScheduler.h"
#include "JPersistenceManager.h"
#include "JLogManager.h"
#include "JTaskManager.h"
#include "JProfileManager.h"

#include <memory>

using STI::Device::JDevice;
using STI::Device::LocalDevice;
using STI::Device::DeviceID;
using STI::Device::DeviceCollection;
using STI::Device::JDeviceCollection;
using STI::Device::DeviceMessageDispatcher;
using STI::Device::JDeviceMessageDispatcher;
using STI::Engine::JEventEngineScheduler;
using STI::Device::ChannelManager;
using STI::Device::JChannelManager;
using STI::Device::AttributeManager;
using STI::Device::JAttributeManager;
using STI::Device::PersistenceManager;
using STI::Device::JPersistenceManager;
using STI::Device::ProfileManager;
using STI::Device::TaskManager;
using STI::Device::LogManager;
using STI::Device::Attribute;


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

void JDevice::kill()
{
    if(wrappedDevice != 0) {
        wrappedDevice->kill();
    }
}

void JDevice::activate()
{
    if(wrappedDevice != 0) {
        wrappedDevice->activate();
    } 
}

void JDevice::disable()
{
    if(wrappedDevice != 0) {
        wrappedDevice->disable();
    }
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

std::shared_ptr<STI::Device::JDeviceMessageDispatcher> JDevice::getMessageDispatcher()
{
    std::shared_ptr<STI::Device::JDeviceMessageDispatcher> jDispatcher;

    if(wrappedDevice != 0) {
        std::shared_ptr<STI::Device::DeviceMessageDispatcher> dispatcher;
        wrappedDevice->getMessageDispatcher(dispatcher);

        jDispatcher = std::make_shared<STI::Device::JDeviceMessageDispatcher>(dispatcher);
    }

    return jDispatcher;
}

std::shared_ptr<STI::Engine::JEventEngineScheduler> JDevice::getEngineScheduler()
{
    std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;
    std::shared_ptr<STI::Engine::JEventEngineScheduler> jScheduler;

    if (wrappedDevice != 0) {

        wrappedDevice->getEngineScheduler(scheduler);
    }
    if (scheduler != 0) {
        jScheduler = std::make_shared<STI::Engine::JEventEngineScheduler>(scheduler);
    }

    return jScheduler;
}

std::shared_ptr<STI::Device::JChannelManager> JDevice::getChannelManager()
{
    std::shared_ptr<STI::Device::ChannelManager> manager;
    std::shared_ptr<STI::Device::JChannelManager> jmanager;

    if (wrappedDevice != 0) {
        wrappedDevice->getChannelManager(manager);
    }
    if (manager != 0) {
        jmanager = std::make_shared<STI::Device::JChannelManager>(manager);
    }

    return jmanager;
}


std::shared_ptr<STI::Device::JAttributeManager> JDevice::getAttributeManager()
{
    std::shared_ptr<STI::Device::AttributeManager> manager;
    std::shared_ptr<STI::Device::JAttributeManager> jmanager;

    if (wrappedDevice != 0) {
        wrappedDevice->getAttributeManager(manager);
    }
    if (manager != 0) {
        jmanager = std::make_shared<STI::Device::JAttributeManager>(manager);
    }

    return jmanager;
}

std::shared_ptr<STI::Device::JPersistenceManager> JDevice::getPersistenceManager()
{
    std::shared_ptr<STI::Device::PersistenceManager> manager;
    std::shared_ptr<STI::Device::JPersistenceManager> jmanager;

    if (wrappedDevice != 0) {
        wrappedDevice->getPersistenceManager(manager);
    }
    if (manager != 0) {
        jmanager = std::make_shared<STI::Device::JPersistenceManager>(manager);
    }

    return jmanager;
}



std::shared_ptr<STI::Device::JProfileManager> JDevice::getProfileManager()
{
    std::shared_ptr<STI::Device::ProfileManager> manager;
    std::shared_ptr<STI::Device::JProfileManager> jmanager;

    if (wrappedDevice != 0) {
        wrappedDevice->getProfileManager(manager);
    }
    if (manager != 0) {
        jmanager = std::make_shared<STI::Device::JProfileManager>(manager);
    }

    return jmanager;
}

std::shared_ptr<STI::Device::JTaskManager> JDevice::getTaskManager()
{
    std::shared_ptr<STI::Device::TaskManager> manager;
    std::shared_ptr<STI::Device::JTaskManager> jmanager;

    if (wrappedDevice != 0) {
        wrappedDevice->getTaskManager(manager);
    }
    if (manager != 0) {
        jmanager = std::make_shared<STI::Device::JTaskManager>(manager);
    }

    return jmanager;
}

std::shared_ptr<STI::Device::JLogManager> JDevice::getLogManager()
{
    std::shared_ptr<STI::Device::LogManager> manager;
    std::shared_ptr<STI::Device::JLogManager> jmanager;

    if (wrappedDevice != 0) {
        wrappedDevice->getLogManager(manager);
    }
    if (manager != 0) {
        jmanager = std::make_shared<STI::Device::JLogManager>(manager);
    }

    return jmanager;
}


bool JDevice::write(short channel, const STI::Utils::MixedValue& value)
{
    if(wrappedDevice != 0) {
        return wrappedDevice->write(channel, value);
    }
    return false;
}

bool JDevice::read(short channel, STI::Utils::MixedValue& data)
{
    return read(channel, STI::Utils::MixedValueType::Empty, data);
}

bool JDevice::read(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
    if(wrappedDevice != 0) {
        return wrappedDevice->read(channel, value, data);
    }
    return false;
}

void JDevice::stopRW()
{
    if(wrappedDevice != 0) {
        wrappedDevice->stopRW();
    }
}

std::string JDevice::getAttribute(const std::string& key)
{
    if(wrappedDevice != 0) {
        return wrappedDevice->getAttribute(key);
    }
    return "";
}

bool JDevice::setAttribute(const std::string& key, const std::string& value)
{
    if(wrappedDevice != 0) {
        return wrappedDevice->setAttribute(key, value);
    }
    return false;
}

bool JDevice::getAttribute(const std::string& key, std::shared_ptr<Attribute>& attribute)
{
    if(wrappedDevice != 0) {
        return wrappedDevice->getAttribute(key, attribute);
    }
    return false;
}


void JDevice::getCollection(std::shared_ptr<STI::Device::DeviceCollection>& collection)
{
    if(wrappedDevice != 0) {
        wrappedDevice->getCollection(collection);
    }
}

void JDevice::getMessageDispatcher(std::shared_ptr<DeviceMessageDispatcher>& dispatcher)
{
    if(wrappedDevice != 0) {
        wrappedDevice->getMessageDispatcher(dispatcher);
    }
}

bool JDevice::getEngineScheduler(std::shared_ptr<STI::Engine::EventEngineScheduler>& scheduler)
{
    if(wrappedDevice != 0) {
        return wrappedDevice->getEngineScheduler(scheduler);
    }
    return false;
}

void JDevice::getChannelManager(std::shared_ptr<ChannelManager>& manager)
{
    if(wrappedDevice != 0) {
        wrappedDevice->getChannelManager(manager);
    }
}

void JDevice::getAttributeManager(std::shared_ptr<AttributeManager>& manager)
{
    if(wrappedDevice != 0) {
        wrappedDevice->getAttributeManager(manager);
    }
}

bool JDevice::getPersistenceManager(std::shared_ptr<PersistenceManager>& manager)
{
    bool success = false;

    if(wrappedDevice != 0) {
        success = wrappedDevice->getPersistenceManager(manager);
    }
    return success;
}

bool JDevice::getProfileManager(std::shared_ptr<ProfileManager>& manager)
{
    if(wrappedDevice != 0) {
        return wrappedDevice->getProfileManager(manager);
    }
    return false;
}

bool JDevice::getTaskManager(std::shared_ptr<TaskManager>& manager)
{
    if(wrappedDevice != 0) {
        return wrappedDevice->getTaskManager(manager);
    }
    return false;
}

bool JDevice::getLogManager(std::shared_ptr<LogManager>& manager)
{
    if(wrappedDevice != 0) {
        return wrappedDevice->getLogManager(manager);
    }
    return false;
}

bool JDevice::refresh()
{
    if(wrappedDevice != 0) {
        return wrappedDevice->refresh();
    }
    return false;
}

