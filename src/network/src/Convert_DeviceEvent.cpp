
#include "NetworkConvert.h"
#include "DeviceEvent.h"
#include "DeviceID.h"

#include "orbTypes.h"
#include <memory>

using STI::Network::convert;
using STI::Device::DeviceEvent;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;
using STI::Device::DeviceEventType;
using STI::TNetwork::TDeviceEventType;
using STI::TNetwork::TDeviceEvent;
using STI::TNetwork::TAnyEvent;



template<>
TDeviceEventType convert<DeviceEventType, TDeviceEventType>(const DeviceEventType& type)
{
	TDeviceEventType tType;

	switch (type)
	{
	case DeviceEventType::AttributesRefresh:
		tType = TDeviceEventType::AttributesRefresh;
		break;
	case DeviceEventType::AttributeUpdate:
		tType = TDeviceEventType::AttributeUpdate;
		break;
	case DeviceEventType::ChannelsRefresh:
		tType = TDeviceEventType::ChannelsRefresh;
		break;
	case DeviceEventType::ChannelUpdate:
		tType = TDeviceEventType::ChannelUpdate;
		break;
	case DeviceEventType::CollectionUpdate:
		tType = TDeviceEventType::CollectionUpdate;
		break;		
	case DeviceEventType::MonitorUpdate:
		tType = TDeviceEventType::MonitorUpdate;
		break;
	case DeviceEventType::Refresh:
		tType = TDeviceEventType::Refresh;
		break;
	default:
		tType = TDeviceEventType::Unknown;
		break;
	}

	return tType;
}

template<>
DeviceEventType convert<TDeviceEventType, DeviceEventType>(const TDeviceEventType& tType)
{
	DeviceEventType type;

	switch (tType)
	{
	case TDeviceEventType::AttributesRefresh:
		type = DeviceEventType::AttributesRefresh;
		break;
	case TDeviceEventType::AttributeUpdate:
		type = DeviceEventType::AttributeUpdate;
		break;
	case TDeviceEventType::ChannelsRefresh:
		type = DeviceEventType::ChannelsRefresh;
		break;
	case TDeviceEventType::ChannelUpdate:
		type = DeviceEventType::ChannelUpdate;
		break;
	case TDeviceEventType::CollectionUpdate:
		type = DeviceEventType::CollectionUpdate;
		break;
	case TDeviceEventType::MonitorUpdate:
		type = DeviceEventType::MonitorUpdate;
		break;
	case TDeviceEventType::Refresh:
		type = DeviceEventType::Refresh;
		break;
	default:
		type = DeviceEventType::Unknown;
		break;
	}

	return type;
}

template<>
bool convert<DeviceEventType, TDeviceEventType>(const DeviceEventType& type, TDeviceEventType& tType)
{
	tType = convert<DeviceEventType, TDeviceEventType>(type);
	return true;
}

template<>
bool convert<TDeviceEventType, DeviceEventType>(const TDeviceEventType& tType, DeviceEventType& type)
{
	type = convert<TDeviceEventType, DeviceEventType>(tType);
	return true;
}


template<>
bool convert<std::shared_ptr<STI::Device::DeviceEvent>, TDeviceEvent>(const std::shared_ptr<STI::Device::DeviceEvent>& deviceEvent, TDeviceEvent& tEvent)
{
	if (deviceEvent == 0) {
		return false;
	}

	tEvent.type = convert<DeviceEventType, TDeviceEventType>(deviceEvent->getType());
	tEvent.sourceID = convert<DeviceID, TDeviceID>(deviceEvent->sourceID());

	return true;
}

template<>
bool convert<TDeviceEvent, std::shared_ptr<STI::Device::DeviceEvent>>(const TDeviceEvent& tEvent, std::shared_ptr<STI::Device::DeviceEvent>& deviceEvent)
{
	deviceEvent = std::make_shared<DeviceEvent>(
		convert<TDeviceID, DeviceID>(tEvent.sourceID),
		convert<TDeviceEventType, DeviceEventType>(tEvent.type)
		);

	return deviceEvent != 0;
}


template<>
bool convert<std::shared_ptr<DeviceEvent>, TAnyEvent>(const std::shared_ptr<DeviceEvent>& deviceEvent, TAnyEvent& tAnyEvent)
{
	if (deviceEvent == 0) {
		return false;
	}

	bool success = false;

	switch (deviceEvent->getType())
	{
	case DeviceEventType::Refresh:
		STI::TNetwork::TRefreshDeviceEvent tRefreshEvt;
		auto rde = std::dynamic_pointer_cast<STI::Device::RefreshDeviceEvent>(deviceEvent);
		if (rde != 0 &&
			convert<std::shared_ptr<DeviceEvent>, TDeviceEvent>(deviceEvent, tRefreshEvt.base)) 
		{
//			tRefreshEvt.base.type = convert<DeviceEventType, TDeviceEventType>(deviceEvent->getType());
	//		tRefreshEvt.base.sourceID = convert<DeviceID, TDeviceID>(deviceEvent->sourceID());
			tAnyEvent.evt <<= tRefreshEvt;
			tAnyEvent.type = convert<DeviceEventType, TDeviceEventType>(deviceEvent->getType());
			success = true;
			//convert<std::shared_ptr<STI::Device::RefreshDeviceEvent>, STI::TNetwork::TRefreshDeviceEvent>(rde, tRefreshEvt);
		}
	}

	return success;
}


template<typename T, typename D>
bool extractEvent(const CORBA::Any& anyEvent, std::shared_ptr<STI::Device::DeviceEvent>& deviceEvent)
{
	bool success = false;

	T* tEvt;								//e.g., TRefreshDeviceEvent
	std::shared_ptr<D> evt;					//e.g., RefreshDeviceEvent

	if ((anyEvent >>= tEvt) &&	//memory managed by CORBA::Any
		tEvt != 0 &&
		convert<T, std::shared_ptr<D>>(*tEvt, evt))
	{	
		deviceEvent = evt;
		success = true;
	}
	return success;
}

template<>
bool convert<TAnyEvent, std::shared_ptr<STI::Device::DeviceEvent>>(const TAnyEvent& tAnyEvent, std::shared_ptr<STI::Device::DeviceEvent>& deviceEvent)
{
	bool success = false;

	switch (tAnyEvent.type) {
	case TDeviceEventType::Refresh:

		success = extractEvent<STI::TNetwork::TRefreshDeviceEvent, STI::Device::RefreshDeviceEvent>(tAnyEvent.evt, deviceEvent);

		//STI::TNetwork::TRefreshDeviceEvent* evt;
		////convert<CORBA::Any, TRefreshDeviceEvent>(tAnyEvent.evt, evt);			//extract from any
		////deviceEvent = convert<TRefreshDeviceEvent, RefreshDeviceEvent>(evt)

		//if (tAnyEvent.evt >>= evt) {	//memory managed by CORBA::Any

		//}
		break;
	}

	return success;
}


template<>
bool convert<STI::TNetwork::TRefreshDeviceEvent, std::shared_ptr<STI::Device::RefreshDeviceEvent>>(const STI::TNetwork::TRefreshDeviceEvent& tEvent, std::shared_ptr<STI::Device::RefreshDeviceEvent>& deviceEvent)
{
	deviceEvent = std::make_shared<STI::Device::RefreshDeviceEvent>(
		convert<TDeviceID, DeviceID>(tEvent.base.sourceID)
		);

	return deviceEvent != 0;
}
