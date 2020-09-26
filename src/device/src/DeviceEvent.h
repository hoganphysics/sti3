#ifndef STI_DEVICE_DEVICEEVENT_H
#define STI_DEVICE_DEVICEEVENT_H

#include "DeviceID.h"

namespace STI
{
namespace Device
{


enum class DeviceEventType { Refresh, CollectionUpdate, ChannelUpdate, ChannelsRefresh, AttributeUpdate, AttributesRefresh, MonitorUpdate, Unknown };
//DeviceEvent, 
//DeviceEventReceiver::addListener, ::removeListener, ::refreshListenerGroups

class DeviceEvent
{
public:
	DeviceEvent() : _type(DeviceEventType::Unknown) {}
	DeviceEvent(const STI::Device::DeviceID& source, DeviceEventType type);
	virtual ~DeviceEvent();

	const STI::Device::DeviceID& sourceID() const;

	DeviceEventType getType() const;

	template <typename T>
	static bool convert(const std::shared_ptr<DeviceEvent>& evt, std::shared_ptr<T>& outEvt)
	{
		if (evt == 0) {
			return false;
		}

		bool success = false;
		auto rde = std::dynamic_pointer_cast<T>(evt);
		if (rde != 0) {
			outEvt = rde;
			success = true;
		}
		return success;
	}
	
	static DeviceEventType getEventClassType() { return DeviceEventType::Unknown; }

private:

	DeviceEventType _type;
	STI::Device::DeviceID _source;

};


class RefreshDeviceEvent : public DeviceEvent
{
public:

	RefreshDeviceEvent(const STI::Device::DeviceID& source) : DeviceEvent(source, DeviceEventType::Refresh) {}

	static DeviceEventType getEventClassType() { return DeviceEventType::Refresh; }

private:

};


class ChannelUpdateDeviceEvent : public DeviceEvent
{
public:

	ChannelUpdateDeviceEvent(const STI::Device::DeviceID& source) : DeviceEvent(source, DeviceEventType::ChannelUpdate) {}

	//MixedValue channelValue();

	static DeviceEventType getEventClassType() { return DeviceEventType::ChannelUpdate; }

};


} //Device
} //STI


#endif

