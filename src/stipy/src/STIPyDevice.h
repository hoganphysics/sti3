#ifndef STI_PYTHON_STIPYDEVICE_H
#define STI_PYTHON_STIPYDEVICE_H

#include "DeviceID.h"

#include <string>

namespace STI
{
namespace Python
{

class STIPyDevice
{
public:

    STIPyDevice(const std::string& name);   //abstract

    STIPyDevice(const std::string& name, const std::string& address, unsigned module);
    STIPyDevice(const STI::Device::DeviceID& id);

    bool isAbstract() const;
    const std::string& abstractName() const;

    STI::Device::DeviceID id() const;

private:

    STI::Device::DeviceID deviceID;
    bool abstract_;
    std::string abstractName_;

};


} //Python
} //STI

#endif

