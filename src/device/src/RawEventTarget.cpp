
#include <sti/engine/RawEventTarget.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>


using STI::Engine::RawEventTarget;
using STI::Engine::RawEventTargetDevice;
using STI::Engine::RawEventTargetChannel;

RawEventTarget::RawEventTarget()
: _device(""), _channel("")
{
}

RawEventTarget::RawEventTarget(const RawEventTargetDevice& dev, const RawEventTargetChannel& ch)
: _device(dev), _channel(ch)
{
}

RawEventTarget::RawEventTarget(const STI::Device::DeviceID& targetDevice, unsigned short channel)
: _device(targetDevice), _channel(channel)
{
}

RawEventTarget::RawEventTarget(const STI::Device::DeviceID& targetDevice, const std::string& channelName)
: _device(targetDevice), _channel(channelName)
{   
}

RawEventTarget::RawEventTarget(const std::string& deviceName, unsigned short channel)
: _device(deviceName), _channel(channel)
{  
}

RawEventTarget::RawEventTarget(const std::string& deviceName, const std::string& channelName)
: _device(deviceName), _channel(channelName)
{  
}

RawEventTarget::RawEventTarget(const std::string& channelName)
: _device(""), _channel(channelName)
{   
}

bool RawEventTarget::isAbstract() const
{
    return (_device.isAbstract() || _channel.isAbstract());
}

RawEventTargetDevice& RawEventTarget::getDevice()
{
    return _device;
}
const RawEventTargetDevice& RawEventTarget::device() const
{
    return _device;
}

RawEventTargetChannel& RawEventTarget::getChannel()
{
    return _channel;
}

const RawEventTargetChannel& RawEventTarget::channel() const
{
    return _channel;
}

bool RawEventTarget::operator<(const RawEventTarget& rhs) const
{
    if (device() == rhs.device()) {
        return channel() < rhs.channel();
    }
    return device() < rhs.device();
}

bool RawEventTarget::operator==(const RawEventTarget& rhs) const
{
    return device() == rhs.device() && channel() == rhs.channel();
}

bool RawEventTarget::operator!=(const RawEventTarget& rhs) const
{
    return !((*this) == rhs);
}

template<class Archive>
void RawEventTarget::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("device", _device), 
		cereal::make_nvp("channel", _channel)
		);
}


template void RawEventTarget::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void RawEventTarget::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

