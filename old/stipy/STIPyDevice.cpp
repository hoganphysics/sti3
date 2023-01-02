
#include "STIPyDevice.h"

#include <sstream>

using STI::Python::STIPyDevice;

STIPyDevice::STIPyDevice(const std::string& name) 
: abstract_(true), deviceID(), abstractName_(name)
{
}

STIPyDevice::STIPyDevice(const std::string& name, const std::string& address, unsigned module, const std::string& targetServerID)
: abstract_(false), deviceID(name, address, module, targetServerID)
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

std::string STIPyDevice::abstractName() const
{
    return abstractName_;
}

STI::Device::DeviceID STIPyDevice::id() const
{
    return deviceID;
}

std::string STIPyDevice::print() const
{
    std::stringstream s;
 
    s << "dev(";    
    if (isAbstract()) {
        s << abstractName();
    }
    else {
        s << id().getID();
    }
    s << ")";

    return s.str();
}
