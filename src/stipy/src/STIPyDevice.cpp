
#include "STIPyDevice.h"


using STI::Python::STIPyDevice;

STIPyDevice::STIPyDevice(const std::string& name) 
: abstract_(true), deviceID(), abstractName_(name)
{
}

STIPyDevice::STIPyDevice(const std::string& name, const std::string& address, unsigned module)
: abstract_(false), deviceID(name, address, module)
{
}


STIPyDevice::STIPyDevice(const STI::Device::DeviceID& id)
: abstract_(false), deviceID(id)
{
}

bool STIPyDevice::isAbstract() const
{
    return abstract_;
}

const std::string& STIPyDevice::abstractName() const
{
    return abstractName_;
}

STI::Device::DeviceID STIPyDevice::id() const
{
    return deviceID;
}

