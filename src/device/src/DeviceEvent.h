#ifndef STI_DEVICE_DEVICEEVENT_H
#define STI_DEVICE_DEVICEEVENT_H

#include "DeviceID.h"

namespace STI
{
namespace Device
{


enum class DeviceEventType { Refresh, CollectionUpdate, ChannelUpdate, ChannelsRefresh, AttributeUpdate, AttributesRefresh, MonitorUpdate, Unknown };

class DeviceEvent
{
public:
	DeviceEvent() : _type(DeviceEventType::Unknown) {}
	//DeviceEvent(const DeviceEventType& type) : _type(type) {}
	DeviceEvent(const STI::Device::DeviceID& source, DeviceEventType type);
	virtual ~DeviceEvent();

	const STI::Device::DeviceID& sourceID() const;

	DeviceEventType getType() const;

private:

//	virtual DeviceEventType _getType() const { return DeviceEventType::Unknown; }	//should never happen

	DeviceEventType _type;
	STI::Device::DeviceID _source;

};


class RefreshDeviceEvent : public DeviceEvent
{
public:

	RefreshDeviceEvent(const STI::Device::DeviceID& source) : DeviceEvent(source, DeviceEventType::Refresh) {}

private:

//	DeviceEventType _getType() const { return DeviceEventType::Refresh; }

};

//
//class ChannelUpdateDeviceEvent : public DeviceEvent
//{
//public:
//
//	ChannelUpdateDeviceEvent(const STI::Device::DeviceID& source) : DeviceEvent(source) {}
//
//	short channel();
//
//	STI::Utils::MixedValue value();
//
//private:
//
//	const DeviceEventType& _getType() const { return DeviceEventType::ChannelUpdate; }
//};
//



} //Device
} //STI


#endif

