#ifndef STI_DEVICE_DEVICEMESSAGE_H
#define STI_DEVICE_DEVICEMESSAGE_H

#include "DeviceID.h"
#include "fwd/EventEngine_fwd.h"
#include "RawEvent.h"
#include "EngineJobID.h"

namespace STI
{
namespace Device
{


enum class DeviceMessageType { 
	Refresh, CollectionUpdate, 
	ChannelUpdate, ChannelsRefresh, 
	AttributeUpdate, AttributesRefresh, 
	MonitorUpdate, 
//	EngineJobUpdate,
	EngineScheduler, 
	EngineParser,
	EngineStatus,
	Unknown };
//DeviceMessage, 
//DeviceMessageReceiver::addListener, ::removeListener, ::refreshListenerGroups,  and add a dedicated ListenerGroupMap instance

class DeviceMessage
{
public:
	DeviceMessage() : _type(DeviceMessageType::Unknown) {}
	DeviceMessage(const STI::Device::DeviceID& source, DeviceMessageType type);
	virtual ~DeviceMessage();

	const STI::Device::DeviceID& sourceID() const;

	DeviceMessageType getType() const;

	template <typename T>
	static bool convert(const std::shared_ptr<DeviceMessage>& evt, std::shared_ptr<T>& outEvt)
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
	
	static DeviceMessageType getMessageClassType() { return DeviceMessageType::Unknown; }

private:

	DeviceMessageType _type;
	STI::Device::DeviceID _source;

};


class RefreshDeviceMessage : public DeviceMessage
{
public:

	RefreshDeviceMessage(const STI::Device::DeviceID& source) : DeviceMessage(source, DeviceMessageType::Refresh) {}

	static DeviceMessageType getMessageClassType() { return DeviceMessageType::Refresh; }

private:

};


class ChannelUpdateDeviceMessage : public DeviceMessage
{
public:

	ChannelUpdateDeviceMessage(const STI::Device::DeviceID& source) : DeviceMessage(source, DeviceMessageType::ChannelUpdate) {}

	//MixedValue channelValue();

	static DeviceMessageType getMessageClassType() { return DeviceMessageType::ChannelUpdate; }

};


// class EngineJobUpdateDeviceMessage : public DeviceMessage
// {
// public:

// 	EngineJobUpdateDeviceMessage(const STI::Device::DeviceID& source) : DeviceMessage(source, DeviceMessageType::EngineJobUpdate) {}

// 	static DeviceMessageType getMessageClassType() { return DeviceMessageType::EngineJobUpdate; }



// private:

// };

/*

ParseReserve:  Ready, Not ready

*/

class EngineSchedulerMessage : public DeviceMessage
{
public:

	//enum class ReserveStatus { Success, Yield };
	enum class SchedulerMessageType { ParseComplete, YieldParse, PartialParse, PlayReady, YieldPlay };

	EngineSchedulerMessage(const STI::Device::DeviceID& source, STI::Device::DeviceID originalSource, const SchedulerMessageType& type) 
	: DeviceMessage(source, DeviceMessageType::EngineScheduler), originalSource(originalSource), schedulerMessageType(type) 
	{
	}
	
	static DeviceMessageType getMessageClassType() { return DeviceMessageType::EngineScheduler; }

	SchedulerMessageType schedulerMessageType;

	STI::Device::DeviceID originalSource;	//device that generated the original message
	STI::Engine::EngineJobID jobID;
	std::shared_ptr<STI::Engine::EventEngine> engine;
	// std::vector<STI::Engine::RawEvent> parsedEvents; //device generated events that are already parsed; want a complete record to make it up the chain
	// std::vector<STI::Engine::RawEvent> upstreamEvents; //to be handled upstream

	std::vector<STI::Engine::RawEvent> handledEvents;	//:device generated events that are being sent upstream for documentation, but they have already been parsed
	std::vector<STI::Engine::RawEvent> unhandledEvents;	//:device generated events that have not been parsed and are being sent upstream so their target can be found. 

};

class STIParsingMessage
{
public:

	//errors, warnings
	//status

	enum class ParserMessageType { Error, Warning, Information };

	unsigned id_code;
	std::string name;
	std::string message;
	std::vector<STI::Engine::RawEvent> events;
};

class EngineParserMessage : public DeviceMessage
{
public:

	//errors, warnings
	//status
	STI::Engine::ParseID pid;
	std::vector<STIParsingMessage> messages;

};

class EventEngineMessage : public DeviceMessage
{
public:
	//engine status

};

} //Device
} //STI


#endif

