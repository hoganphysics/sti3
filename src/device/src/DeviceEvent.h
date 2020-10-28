#ifndef STI_DEVICE_DEVICEEVENT_H
#define STI_DEVICE_DEVICEEVENT_H

#include "DeviceID.h"
#include "fwd/EventEngine_fwd.h"
#include "RawEvent.h"
#include "EngineJobID.h"

namespace STI
{
namespace Device
{


enum class DeviceEventType { 
	Refresh, CollectionUpdate, 
	ChannelUpdate, ChannelsRefresh, 
	AttributeUpdate, AttributesRefresh, 
	MonitorUpdate, 
//	EngineJobUpdate,
	EngineScheduler, 
	EngineParser,
	EngineStatus,
	Unknown };
//DeviceEvent, 
//DeviceEventReceiver::addListener, ::removeListener, ::refreshListenerGroups,  and add a dedicated ListenerGroupMap instance

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


// class EngineJobUpdateDeviceEvent : public DeviceEvent
// {
// public:

// 	EngineJobUpdateDeviceEvent(const STI::Device::DeviceID& source) : DeviceEvent(source, DeviceEventType::EngineJobUpdate) {}

// 	static DeviceEventType getEventClassType() { return DeviceEventType::EngineJobUpdate; }



// private:

// };

/*

ParseReserve:  Ready, Not ready

*/

class EngineSchedulerMessage : public DeviceEvent
{
public:

	//enum class ReserveStatus { Success, Yield };
	enum class SchedulerMessageType { ParseComplete, YieldParse, PartialParse, PlayReady, YieldPlay };

	EngineSchedulerMessage(const STI::Device::DeviceID& source, STI::Device::DeviceID originalSource, const SchedulerMessageType& type) 
	: DeviceEvent(source, DeviceEventType::EngineScheduler), originalSource(originalSource), type(type) 
	{
	}
	
	static DeviceEventType getEventClassType() { return DeviceEventType::EngineScheduler; }

	SchedulerMessageType type;

	STI::Device::DeviceID originalSource;	//device that generated the original message
	STI::Engine::EngineJobID jobID;
	std::shared_ptr<STI::Engine::EventEngine> engine;
	std::vector<STI::Engine::RawEvent> parsedEvents; //device generated events that are already parsed; want a complete record to make it up the chain
	std::vector<STI::Engine::RawEvent> upstreamEvents; //to be handled upstream

	//handledEvents		:device generated events that are being sent upstream for documentation, but they have already been parsed
	//unhandledEvents	:device generated events that have not been parsed and are being sent upstream so their target can be found. 

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

class EngineParserMessage : public DeviceEvent
{
public:

	//errors, warnings
	//status
	STI::Engine::ParseID pid;
	std::vector<STIParsingMessage> messages;

};

class EventEngineMessage : public DeviceEvent
{
public:
	//engine status

};

} //Device
} //STI


#endif

