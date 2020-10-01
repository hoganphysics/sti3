
#include "JDevice.h"
#include "Device.h"
#include "DeviceID.h"
#include "LocalDevice.h"
#include "JDeviceCollection.h"

#include <memory>

using STI::Device::JDevice;
using STI::Device::LocalDevice;
using STI::Device::DeviceID;
using STI::Device::DeviceCollection;
using STI::Device::JDeviceCollection;
using STI::Device::DeviceEventDispatcher;
//using STI::Device::DeviceEventReceiver;


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

DeviceID JDevice::getID()
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

// void JDevice::getEventReceiver(std::shared_ptr<DeviceEventReceiver>& receiver)
// {
//     if(localDevice != 0) {
//         localDevice->getEventReceiver(receiver);
//     }
// }

bool JDevice::refresh()
{
    if(wrappedDevice != 0) {
        return wrappedDevice->refresh();
    }
    return false;
}

void JDevice::write(unsigned input)
{
    if(wrappedDevice != 0) {
        return wrappedDevice->write(input);
    }
}
