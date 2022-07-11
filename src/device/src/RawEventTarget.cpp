
#include <sti/engine/RawEventTarget.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>


using STI::Engine::RawEventTarget;
using STI::Engine::RawEventTargetDevice;
using STI::Engine::RawEventTargetChannel;


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

