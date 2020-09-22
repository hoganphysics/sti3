

#include "DeviceEvent.h"
#include "DeviceID.h"

using STI::Device::DeviceEvent;
using STI::Device::DeviceEventType;


DeviceEvent::DeviceEvent(const STI::Device::DeviceID& source, DeviceEventType type)
	: _source(source), _type(type)
{
}

DeviceEvent::~DeviceEvent()
{
}

const STI::Device::DeviceID& DeviceEvent::sourceID() const
{ 
	return _source; 
}

DeviceEventType DeviceEvent::getType() const 
{ 
//	return _getType();
	return _type;
}

