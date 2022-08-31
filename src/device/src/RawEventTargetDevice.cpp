
#include <sti/engine/RawEventTargetDevice.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>


using STI::Engine::RawEventTargetDevice;

RawEventTargetDevice::RawEventTargetDevice()
: RawEventTargetDevice("")
{
}

RawEventTargetDevice::RawEventTargetDevice(const std::string& name)
: _name(name), _isAbstract(true)
{
}

RawEventTargetDevice::RawEventTargetDevice(const STI::Device::DeviceID& targetDevice)
: targetDeviceID(targetDevice), _isAbstract(false)
{
}

RawEventTargetDevice::RawEventTargetDevice(const std::string& name, const std::string& address, unsigned short module)
: RawEventTargetDevice( STI::Device::DeviceID(name, address, module) )
{
}

bool RawEventTargetDevice::isAbstract() const
{
    return _isAbstract;
}

std::string RawEventTargetDevice::name() const
{
    return _name;
}

STI::Device::DeviceID RawEventTargetDevice::deviceID() const
{
    return targetDeviceID;
}

void RawEventTargetDevice::setTargetDeviceID(const STI::Device::DeviceID& targetDevice)
{
    _isAbstract = false;
    targetDeviceID = targetDevice;
}


bool RawEventTargetDevice::operator<(const RawEventTargetDevice& rhs) const
{
    if (isAbstract()) {
        if (rhs.isAbstract()) {
            return name() < rhs.name();
        }
        return true;
    }

    //neither abstract
    return deviceID() < rhs.deviceID();
}

bool RawEventTargetDevice::operator==(const RawEventTargetDevice& rhs) const
{
    if (isAbstract()) {
        if (rhs.isAbstract()) {
            return name() == rhs.name();
        }
        return false;     
    }

    //neither abstract
    return deviceID() == rhs.deviceID();

}

bool RawEventTargetDevice::operator!=(const RawEventTargetDevice& rhs) const
{
    return !((*this) == rhs);
}


template<class Archive>
void RawEventTargetDevice::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("name", _name), 
		cereal::make_nvp("targetDeviceID", targetDeviceID),
        cereal::make_nvp("isAbstract", _isAbstract)
		);
}

template void RawEventTargetDevice::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void RawEventTargetDevice::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
