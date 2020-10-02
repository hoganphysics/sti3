
#include "JLocalDevice.h"
#include "LocalDevice.h"
#include "DeviceEventReceiver.h"
#include "JDeviceEventReceiver.h"

#include <memory>

using STI::Device::JLocalDevice;
using STI::Device::JDeviceEventReceiver;


JLocalDevice::JLocalDevice(const std::string& name, const std::string& address, unsigned short module,
		const std::string& targetServer)
        : STI::Device::JDevice(name, address, module, targetServer)
{
    //Slight hack here. We want JLocalDevice to inherit from JDevice AND to delegate to the same
    //wrapped pointer. Using the local constructor, the pointer is created as a LocalDevice but 
    //stored in JDevice as a Device so the JDevice class can also wrap RemoteDevices. So here we 
    //dynamic_cast back...
    //Note JDevoce::wrappedDevice was just created as a LocalDevice, so this is guaranteed to work.

    wrappedLocalDevice = std::dynamic_pointer_cast<STI::Device::LocalDevice>(wrappedDevice);

    //Get and store DeviceEventReceiver reference
    std::shared_ptr<DeviceEventReceiver> receiver;
    wrappedLocalDevice->getEventReceiver(receiver);

    jReceiver = std::make_shared<JDeviceEventReceiver>(receiver);
}

JLocalDevice::~JLocalDevice()
{
}

std::shared_ptr<STI::Device::JDeviceEventReceiver> JLocalDevice::getEventReceiver()
{
    return jReceiver;
}
