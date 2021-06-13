

#include "DeviceMessage.h"
#include "DeviceID.h"

using STI::Device::DeviceMessage;
using STI::Device::DeviceMessageType;
using STI::Device::RefreshDeviceMessage;

DeviceMessage::DeviceMessage(const STI::Device::DeviceID& source, DeviceMessageType type)
: _trace(source), _type(type)
{
}

DeviceMessage::DeviceMessage(const STI::Device::DeviceTrace& trace, DeviceMessageType type)
: _trace(trace), _type(type)
{
}

// DeviceMessage::DeviceMessage(const STI::Device::DeviceID& relayingID, const DeviceMessage& message)
// : _trace(message._trace), _type(message._type)
// {
// 	_trace.addID(relayingID);
// }

DeviceMessage::~DeviceMessage()
{
}

void DeviceMessage::addRelayingID(const STI::Device::DeviceID& relayingID)
{
	_trace.addID(relayingID);
}

const STI::Device::DeviceID DeviceMessage::sourceID() const
{ 
	return _trace.last();
}

const STI::Device::DeviceID DeviceMessage::originalSourceID() const
{
	return _trace.first();
}

DeviceMessageType DeviceMessage::getType() const 
{ 
	return _type;
}

const STI::Device::DeviceTrace& DeviceMessage::getDeviceTrace() const
{
	return _trace;
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