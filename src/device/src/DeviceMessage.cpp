

#include "DeviceMessage.h"
#include "DeviceID.h"

using STI::Device::DeviceMessage;
using STI::Device::DeviceMessageType;
using STI::Device::RefreshDeviceMessage;

DeviceMessage::DeviceMessage(const STI::Device::DeviceID& source, DeviceMessageType type)
	: _source(source), _type(type)
{
}

DeviceMessage::~DeviceMessage()
{
}

const STI::Device::DeviceID& DeviceMessage::sourceID() const
{ 
	return _source; 
}

DeviceMessageType DeviceMessage::getType() const 
{ 
	return _type;
}

