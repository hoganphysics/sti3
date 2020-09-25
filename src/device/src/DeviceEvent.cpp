

#include "DeviceEvent.h"
#include "DeviceID.h"

using STI::Device::DeviceEvent;
using STI::Device::DeviceEventType;
using STI::Device::RefreshDeviceEvent;

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

//
//template<>
//bool DeviceEvent::convert<RefreshDeviceEvent>(const std::shared_ptr<DeviceEvent>& evt, std::shared_ptr<RefreshDeviceEvent>& outEvt)
//{
//	if (evt == 0) {
//		return false;
//	}
//
//	bool success = false;
//	auto rde = std::dynamic_pointer_cast<RefreshDeviceEvent>(evt);
//	if (rde != 0) {
//		outEvt = rde;
//		success = true;
//	}
//	return success;
//}

