

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

std::string DeviceMessage::typeToString(const DeviceMessageType& type)
{
	std::string name;

	switch (type)
	{
	case DeviceMessageType::Refresh:
		name = "Refresh";
		break;
	case DeviceMessageType::CollectionUpdate:
		name = "CollectionUpdate";
		break;
	case DeviceMessageType::ChannelUpdate:
		name = "ChannelUpdate";
		break;
	case DeviceMessageType::ChannelsRefresh:
		name = "ChannelsRefresh";
		break;
	case DeviceMessageType::AttributeUpdate:
		name = "AttributeUpdate";
		break;
	case DeviceMessageType::AttributesRefresh:
		name = "AttributesRefresh";
		break;
	case DeviceMessageType::MonitorUpdate:
		name = "MonitorUpdate";
		break;
	case DeviceMessageType::EngineScheduler:
		name = "EngineScheduler";
		break;
	case DeviceMessageType::EngineParser:
		name = "EngineParser";
		break;
	case DeviceMessageType::EngineStatus:
		name = "EngineStatus";
		break;
	case DeviceMessageType::Unknown:
		name = "Unknown";
		break;
	default:
		name = "Unknown";
		break;
	}

	return name;
}