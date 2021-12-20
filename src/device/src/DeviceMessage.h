#ifndef STI_DEVICE_DEVICEMESSAGE_H
#define STI_DEVICE_DEVICEMESSAGE_H

#include "DeviceID.h"
#include "fwd/EventEngine_fwd.h"
#include "RawEvent.h"
#include "EngineJobID.h"
#include "GroupableMessage.h"
#include "EngineParsingMessage.h"
#include "EngineState.h"
#include "DeviceTrace.h"
#include "EngineID.h"
#include "EngineParsingMessage.h"
//#include "EventEngineDependencyTree.h"
#include "EventEngineJob.h"
#include "EventEngineJobList.h"

#include <sstream>

namespace STI
{
namespace Device
{


enum class DeviceMessageType { 
	Refresh, CollectionUpdate, 
	ChannelUpdate, ChannelsRefresh, 
	AttributeUpdate, AttributesRefresh, 
	MonitorUpdate, 
	EngineJobUpdate,
	EngineScheduler, 
	EngineParser,
	EngineStatus,
	Unknown };
//DeviceMessage, 
//DeviceMessageReceiver::addListener, ::removeListener, ::refreshListenerGroups, ::clearAllListenerGroups, 
//and add a dedicated ListenerGroupMap instance

class DeviceMessage
{
public:
	DeviceMessage() : _type(DeviceMessageType::Unknown) {}
	DeviceMessage(const STI::Device::DeviceID& source, DeviceMessageType type);
	DeviceMessage(const STI::Device::DeviceTrace& trace, DeviceMessageType type);
//	DeviceMessage(const STI::Device::DeviceID& relayingID, const DeviceMessage& message);
	virtual ~DeviceMessage();

	const STI::Device::DeviceID sourceID() const;

	const STI::Device::DeviceID originalSourceID() const;

	DeviceMessageType getType() const;
	const DeviceTrace& getDeviceTrace() const;

	void addRelayingID(const STI::Device::DeviceID& relayingID);


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

	static std::string typeToString(const DeviceMessageType& type);

private:

	DeviceMessageType _type;
	//STI::Device::DeviceID _source;
	STI::Device::DeviceTrace _trace;
};


class RefreshDeviceMessage : public DeviceMessage
{
public:

	RefreshDeviceMessage(const STI::Device::DeviceTrace& trace) : DeviceMessage(trace, DeviceMessageType::Refresh) {}

	static DeviceMessageType getMessageClassType() { return DeviceMessageType::Refresh; }

private:

};



class CollectionUpdateMessage;

class CollectionUpdateMessage : public DeviceMessage, 
						  		public STI::Device::GroupableMessage<CollectionUpdateMessage>
{
public:

	enum class CollectionMessageType { Add, Remove, Refresh };

	CollectionUpdateMessage(const STI::Device::DeviceTrace& trace) 
	: DeviceMessage(trace, DeviceMessageType::CollectionUpdate), updateType(CollectionMessageType::Refresh)
	{
	}

	static std::shared_ptr<CollectionUpdateMessage> makeMessage(const STI::Device::DeviceID& source)
	{
		STI::Device::DeviceTrace trace(source);
		auto mess = std::make_shared<CollectionUpdateMessage>(trace);
		return mess;
	}

	// CollectionUpdateMessage(const STI::Device::DeviceID& source, const CollectionMessageType& type, const STI::Device::DeviceID& updatedID) 
	// : DeviceMessage(source, DeviceMessageType::CollectionUpdate), id(id), updateType(type)
	// {
	// 	channelValues[channel] = value;
	// }
	
	static DeviceMessageType getMessageClassType() { return DeviceMessageType::CollectionUpdate; }

    bool appendMessage(const CollectionUpdateMessage& mess)
	{
		return true;
	}
    
	bool groupable() const
	{
		return true;
	}

	CollectionUpdateMessage& get()
	{
		return *this;
	}

	CollectionMessageType updateType;

};



class ChannelUpdateMessage;

class ChannelUpdateMessage : public DeviceMessage, 
							 public STI::Device::GroupableMessage<ChannelUpdateMessage>
{
public:

	enum class ChannelUpdateMessageType { ChannelValue, ChannelName };

	ChannelUpdateMessage(const STI::Device::DeviceTrace& trace) 
	: DeviceMessage(trace, DeviceMessageType::ChannelUpdate) 
	{
		channelUpdateType = ChannelUpdateMessageType::ChannelValue;
	}

	ChannelUpdateMessage(const STI::Device::DeviceTrace& trace, short channel, const STI::Utils::MixedValue& value) 
	: DeviceMessage(trace, DeviceMessageType::ChannelUpdate) 
	{
		channelUpdateType = ChannelUpdateMessageType::ChannelValue;
		channelValues[channel] = value;
	}

	ChannelUpdateMessage(const STI::Device::DeviceTrace& trace, short channel, const std::string& name) 
	: DeviceMessage(trace, DeviceMessageType::ChannelUpdate) 
	{
		channelUpdateType = ChannelUpdateMessageType::ChannelName;
		channelNumber = channel;
		channelName = name;
	}
	
	static DeviceMessageType getMessageClassType() { return DeviceMessageType::ChannelUpdate; }

    bool appendMessage(const ChannelUpdateMessage& mess)
	{
        for (auto& pair : mess.channelValues) {
            channelValues[pair.first] = pair.second;  //overwrites
        }
		return true;
	}
    
	bool groupable() const
	{
		return channelUpdateType == ChannelUpdateMessageType::ChannelValue;
	}

	ChannelUpdateMessage& get()
	{
		return *this;
	}

	ChannelUpdateMessageType channelUpdateType;
	std::map<short, STI::Utils::MixedValue> channelValues;	//just {channel, value} pairs

	//only used for ChannelName messages
	short channelNumber;
	std::string channelName;

};


class AttributeUpdateMessage;

class AttributeUpdateMessage : public DeviceMessage,
							   public STI::Device::GroupableMessage<AttributeUpdateMessage>
{
public:

	AttributeUpdateMessage(const STI::Device::DeviceTrace& trace) 
	: DeviceMessage(trace, DeviceMessageType::AttributeUpdate) 
	{
	}

	AttributeUpdateMessage(const STI::Device::DeviceTrace& trace, const std::string& key, const std::string& value) 
	: DeviceMessage(trace, DeviceMessageType::AttributeUpdate) 
	{
		attributes[key] = value;
	}

	//MixedValue channelValue();

	static DeviceMessageType getMessageClassType() { return DeviceMessageType::AttributeUpdate; }

    bool appendMessage(const AttributeUpdateMessage& mess)
	{
        for (auto& pair : mess.attributes) {
            attributes[pair.first] = pair.second;  //overwrites
        }
		return true;
	}
	
	bool groupable() const
	{
		return true;
	}

    AttributeUpdateMessage& get()
	{
		return *this;
	}

	std::map<std::string, std::string> attributes;	//just {key, value} pairs

	std::string toString() const
	{
		std::stringstream mess;
		mess << "AttributeUpdate {\n";
		
		for (auto& pair : attributes) {
			mess << "\t" << pair.first << " -> " << pair.second << "\n";
		}
		mess << "}";
		return mess.str();
	}

};



// enum class EngineJobUpdateTarget { Queued, Running, Completed };

class EngineJobUpdateDeviceMessage : public DeviceMessage
{
public:

	EngineJobUpdateDeviceMessage(const STI::Device::DeviceTrace& trace) 
	: DeviceMessage(trace, DeviceMessageType::EngineJobUpdate) {}

	EngineJobUpdateDeviceMessage(const STI::Device::DeviceTrace& trace, 
		const std::shared_ptr<STI::Engine::EventEngineJob>& job, STI::Engine::EventEngineJobList targetList) 
	: DeviceMessage(trace, DeviceMessageType::EngineJobUpdate)
	{
		switch (targetList) {
			case STI::Engine::EventEngineJobList::Queued:
				toQueuedList(job);
				break;
			case STI::Engine::EventEngineJobList::Running:
				toRunningList(job);
				break;
			case STI::Engine::EventEngineJobList::Completed:
				toCompleteList(job);
				break;
			case STI::Engine::EventEngineJobList::Archived:
				toArchive(job);
				break;
			default:
				toCompleteList(job);
		}
	}

	static DeviceMessageType getMessageClassType() { return DeviceMessageType::EngineJobUpdate; }

	void toQueuedList(const std::shared_ptr<STI::Engine::EventEngineJob>& job)
	{
		targetList = STI::Engine::EventEngineJobList::Queued;
		engineJob = job;
	}
	void toRunningList(const std::shared_ptr<STI::Engine::EventEngineJob>& job)
	{
		targetList = STI::Engine::EventEngineJobList::Running;
		engineJob = job;
	}
	void toCompleteList(const std::shared_ptr<STI::Engine::EventEngineJob>& job)
	{
		targetList = STI::Engine::EventEngineJobList::Completed;
		engineJob = job;
	}
	void toArchive(const std::shared_ptr<STI::Engine::EventEngineJob>& job)
	{
		targetList = STI::Engine::EventEngineJobList::Archived;
		engineJob = job;
	}
	

	STI::Engine::EventEngineJobList getTargetList() const
	{
		return targetList;
	}

	std::shared_ptr<STI::Engine::EventEngineJob> getEngineJob() const
	{
		return engineJob;
	}

	static std::string jobTargetToString(STI::Engine::EventEngineJobList target) 
	{
		std::string result;

		switch (target)
		{
		case STI::Engine::EventEngineJobList::Queued:
			result = "Queued";
			break;
		case STI::Engine::EventEngineJobList::Running:
			result = "Running";
			break;
		case STI::Engine::EventEngineJobList::Completed:
			result = "Completed";
			break;
		case STI::Engine::EventEngineJobList::Archived:
			result = "Archived";
			break;			
		default:
			result = "Unknown";
			break;
		}

		return result;
	}

private:

	STI::Engine::EventEngineJobList targetList;	//the list the job belongs in
	std::shared_ptr<STI::Engine::EventEngineJob> engineJob;

};

/*

ParseReserve:  Ready, Not ready

*/

//should rename this EngineParserMessage
//EngineSchedulerMessage should deal with requesting parse/play across network
class EngineSchedulerMessage : public DeviceMessage
{
public:

	//enum class ReserveStatus { Success, Yield };
	enum class SchedulerMessageType { ParseComplete, YieldParse, PartialParse, PlayReady, PlayComplete, YieldPlay };

	EngineSchedulerMessage(const STI::Device::DeviceTrace& trace, const SchedulerMessageType& type) 	//STI::Device::DeviceID originalSource,
	: DeviceMessage(trace, DeviceMessageType::EngineScheduler), schedulerMessageType(type)	//, originalSource(originalSource) 
	{
	}
	
	static DeviceMessageType getMessageClassType() { return DeviceMessageType::EngineScheduler; }


	SchedulerMessageType schedulerMessageType;

	//STI::Device::DeviceID originalSource;	//device that generated the original message
	STI::Engine::EngineJobID jobID;

	std::shared_ptr<STI::Engine::EventEngine> getEngine() const
	{
		return _engine;
	}
	void setEngine(const std::shared_ptr<STI::Engine::EventEngine>& engine)
	{
		_engine = engine;
	}

	// std::vector<STI::Engine::RawEvent> parsedEvents; //device generated events that are already parsed; want a complete record to make it up the chain
	// std::vector<STI::Engine::RawEvent> upstreamEvents; //to be handled upstream

	std::vector<STI::Engine::RawEvent> handledEvents;	//:device generated events that are being sent upstream for documentation, but they have already been parsed
	std::vector<STI::Engine::RawEvent> unhandledEvents;	//:device generated events that have not been parsed and are being sent upstream so their target can be found. 

	std::vector<STI::Engine::EngineParsingMessage> messages;

	STI::Engine::EngineState engineState;

private:

	std::shared_ptr<STI::Engine::EventEngine> _engine;
};

// class STIParsingMessage
// {
// public:

// 	//errors, warnings
// 	//status

// 	enum class ParsingMessageType { Error, Warning, Information };

// 	ParsingMessageType type;

// 	unsigned id_code;
// 	std::string name;
// 	std::string message;
// 	std::vector<STI::Engine::RawEvent> events;
// };

class EngineParserDeviceMessage : public DeviceMessage
{
public:

	EngineParserDeviceMessage(const STI::Device::DeviceTrace& trace, const STI::Engine::ParseID& parseID) 
	: DeviceMessage(trace, DeviceMessageType::EngineParser), pid(parseID)
	{
	}

	static DeviceMessageType getMessageClassType() { return DeviceMessageType::EngineParser; }

	void addParseMessage(const STI::Engine::EngineParsingMessage& message)
	{
		messages.push_back(message);
	}

	//errors, warnings
	//status
	STI::Engine::ParseID pid;
	std::vector<STI::Engine::EngineParsingMessage> messages;
//	std::shared_ptr<STI::Engine::EventEngineDependencyTree> parsedTree;

};



class EngineStateMessage;

class EngineStateMessage : public DeviceMessage,
						   public STI::Device::GroupableMessage<EngineStateMessage>
{
public:
	//engine status

	EngineStateMessage(const STI::Device::DeviceTrace& trace)
	: DeviceMessage(trace, DeviceMessageType::EngineStatus)
	{
	}

	EngineStateMessage(const STI::Device::DeviceTrace& trace, const STI::Engine::EngineID& engineID, const STI::Engine::EngineState& state) 
	: DeviceMessage(trace, DeviceMessageType::EngineStatus) 
	{
		engineStates[engineID] = state;
	}

	static DeviceMessageType getMessageClassType() { return DeviceMessageType::EngineStatus; }

    bool appendMessage(const EngineStateMessage& mess)
	{
		for (auto& pair : mess.engineStates) {
			engineStates[pair.first] = pair.second;
		}
		return true;
	}
	
	bool groupable() const
	{
		return true;
	}

    EngineStateMessage& get()
	{
		return *this;
	}
//, STI::Engine::EngineID::EngineIDCompare
	std::map<STI::Engine::EngineID, STI::Engine::EngineState> engineStates;

};

} //Device
} //STI


#endif

