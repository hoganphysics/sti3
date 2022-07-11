#ifndef STI_ENGINE_RAWEVENTTARGETDEVICE_H
#define STI_ENGINE_RAWEVENTTARGETDEVICE_H


#include <sti/device/DeviceID.h>

#include <string>


namespace STI
{
namespace Engine
{


class RawEventTargetDevice
{
public:

    RawEventTargetDevice(const std::string& name);
    RawEventTargetDevice(const STI::Device::DeviceID& targetDevice);
    RawEventTargetDevice(const std::string& name, const std::string& address, unsigned short module);

    bool isAbstract() const;
    std::string name() const;
    STI::Device::DeviceID deviceID() const;

    void setTargetDeviceID(const STI::Device::DeviceID& targetDevice);

    template<class Archive>
    void serialize(Archive& archive);
    
private:
    
    bool _isAbstract;
    std::string _name;
    STI::Device::DeviceID targetDeviceID;
};


} //Engine
} //STI

#endif
