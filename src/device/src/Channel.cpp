
#include "Channel.h"

using STI::Device::Channel;


Channel::Channel(unsigned short channelNumber, STI::Device::TChannelType type,
	STI::Utils::MixedValueType inputType, STI::Utils::MixedValueType outputType, const std::string& defaultName)
	: channelNumber(channelNumber), type(type), inputType(inputType), outputType(outputType), channelName(defaultName)
{
}

Channel::~Channel()
{
}

void Channel::setChannelName(const std::string& name)
{
	channelName = name;
}

std::string Channel::getChannelName() const
{
	return channelName;
}
