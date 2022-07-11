
#include <sti/engine/RawEventTargetChannel.h>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/string.hpp>


using STI::Engine::RawEventTargetChannel;



RawEventTargetChannel::RawEventTargetChannel(const std::string& name)
: _name(name), _isAbstract(true), _channel(0)
{
}

RawEventTargetChannel::RawEventTargetChannel(unsigned short channel)
: _channel(channel), _isAbstract(false), _name("")
{
}

bool RawEventTargetChannel::isAbstract() const
{
    return _isAbstract;
}

std::string RawEventTargetChannel::name() const
{
    return _name;
}

unsigned short RawEventTargetChannel::channel() const
{
    return _channel;
}

void RawEventTargetChannel::setChannel(unsigned short channel)
{
    _channel = channel;
}

template<class Archive>
void RawEventTargetChannel::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("name", _name), 
		cereal::make_nvp("channel", _channel),
        cereal::make_nvp("isAbstract", _isAbstract)
		);
}

template void RawEventTargetChannel::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void RawEventTargetChannel::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
