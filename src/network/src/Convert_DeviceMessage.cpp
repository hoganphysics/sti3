
//#include "NetworkConvert.h"
#include "DeviceMessage.h"
#include "DeviceID.h"
#include "RemoteEventEngine.h"
#include "Convert_EventEngine.h"
#include "Convert_DeviceMessage.h"
#include "Convert_DeviceTrace.h"
#include "NetworkEventEngine.h"
#include "MixedValue.h"

#include "orbTypes.h"

#include <memory>


using STI::Network::convert;
using STI::Device::DeviceMessage;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;
using STI::Device::DeviceMessageType;
using STI::TNetwork::TDeviceMessageType;
using STI::TNetwork::TDeviceMessage;
using STI::TNetwork::TAnyMessage;

using STI::Utils::MixedValue;
using STI::TNetwork::TMixedValue;

using STI::Engine::EventEngineJob;
using STI::TNetwork::TEventEngineJob;

using STI::Device::RefreshDeviceMessage;
using STI::TNetwork::TRefreshDeviceMessage;

using STI::TNetwork::TEngineSchedulerMessage;
using STI::Device::EngineSchedulerMessage;

using STI::Device::EngineSchedulerMessage;
using STI::TNetwork::TSchedulerMessageType;

using STI::Device::EngineParserDeviceMessage;
using STI::TNetwork::TEngineParserDeviceMessage;

using STI::Engine::ParseID;
using STI::TNetwork::TParseID;

using STI::TNetwork::TChannelUpdateMessage;
using STI::Device::ChannelUpdateMessage;
using STI::TNetwork::TChannelUpdateMessageType;

using STI::TNetwork::TAttributeUpdateMessage;
using STI::Device::AttributeUpdateMessage;

using STI::Device::CollectionUpdateMessage;
using STI::TNetwork::TCollectionUpdateMessage;
using STI::TNetwork::TCollectionMessageType;

using STI::Device::DeviceTrace;
using STI::TNetwork::TDeviceTrace;

using STI::TNetwork::TEngineStateMessage;
using STI::Device::EngineStateMessage;

using STI::Engine::EngineID;
using STI::TNetwork::TEngineID;

using STI::Engine::EngineState;
using STI::TNetwork::TEngineState;

using STI::TNetwork::TEngineJobUpdateDeviceMessage;
using STI::Device::EngineJobUpdateDeviceMessage;
using STI::TNetwork::TEngineJobUpdateTarget;
using STI::Device::EngineJobUpdateTarget;

using STI::Engine::EventEngine;
using STI::TNetwork::TEventEngine_var;


template<>
TDeviceMessageType STI::Network::convert<DeviceMessageType, TDeviceMessageType>(const DeviceMessageType& type)
{
	TDeviceMessageType tType;

	switch (type)
	{
		case DeviceMessageType::Refresh:
			tType = TDeviceMessageType::MessageRefresh;
			break;
		case DeviceMessageType::CollectionUpdate:
			tType = TDeviceMessageType::MessageCollectionUpdate;
			break;
		case DeviceMessageType::ChannelUpdate:
			tType = TDeviceMessageType::MessageChannelUpdate;
			break;
		case DeviceMessageType::ChannelsRefresh:
			tType = TDeviceMessageType::MessageChannelsRefresh;
			break;
		case DeviceMessageType::AttributeUpdate:
			tType = TDeviceMessageType::MessageAttributeUpdate;
			break;
		case DeviceMessageType::AttributesRefresh:
			tType = TDeviceMessageType::MessageAttributesRefresh;
			break;
		case DeviceMessageType::MonitorUpdate:
			tType = TDeviceMessageType::MessageMonitorUpdate;
			break;
		case DeviceMessageType::EngineScheduler:
			tType = TDeviceMessageType::MessageEngineScheduler;
			break;
		case DeviceMessageType::EngineParser:
			tType = TDeviceMessageType::MessageEngineParser;
			break;
		case DeviceMessageType::EngineStatus:
			tType = TDeviceMessageType::MessageEngineStatus;
			break;
		case DeviceMessageType::EngineJobUpdate:
			tType = TDeviceMessageType::MessageEngineJobUpdate;
			break;
		default:
			tType = TDeviceMessageType::MessageUnknown;
			break;
	}

	return tType;
}

template<>
DeviceMessageType STI::Network::convert<TDeviceMessageType, DeviceMessageType>(const TDeviceMessageType& tType)
{
	DeviceMessageType type;

	switch (tType)
	{
		case TDeviceMessageType::MessageRefresh:
			type = DeviceMessageType::Refresh;
			break;
		case TDeviceMessageType::MessageCollectionUpdate:
			type = DeviceMessageType::CollectionUpdate;
			break;
		case TDeviceMessageType::MessageChannelUpdate:
			type = DeviceMessageType::ChannelUpdate;
			break;
		case TDeviceMessageType::MessageChannelsRefresh:
			type = DeviceMessageType::ChannelsRefresh;
			break;
		case TDeviceMessageType::MessageAttributeUpdate:
			type = DeviceMessageType::AttributeUpdate;
			break;
		case TDeviceMessageType::MessageAttributesRefresh:
			type = DeviceMessageType::AttributesRefresh;
			break;
		case TDeviceMessageType::MessageMonitorUpdate:
			type = DeviceMessageType::MonitorUpdate;
			break;
		case TDeviceMessageType::MessageEngineScheduler:
			type = DeviceMessageType::EngineScheduler;
			break;
		case TDeviceMessageType::MessageEngineParser:
			type = DeviceMessageType::EngineParser;
			break;
		case TDeviceMessageType::MessageEngineStatus:
			type = DeviceMessageType::EngineStatus;
			break;
		case TDeviceMessageType::MessageEngineJobUpdate:
			type = DeviceMessageType::EngineJobUpdate;
			break;		
		default:
			type = DeviceMessageType::Unknown;
			break;
	}

	return type;
}

template<>
bool STI::Network::convert<DeviceMessageType, TDeviceMessageType>(const DeviceMessageType& type, TDeviceMessageType& tType)
{
	tType = convert<DeviceMessageType, TDeviceMessageType>(type);
	return true;
}

template<>
bool STI::Network::convert<TDeviceMessageType, DeviceMessageType>(const TDeviceMessageType& tType, DeviceMessageType& type)
{
	type = convert<TDeviceMessageType, DeviceMessageType>(tType);
	return true;
}



template<>
bool STI::Network::convert<std::shared_ptr<STI::Device::DeviceMessage>, TDeviceMessage>(const std::shared_ptr<STI::Device::DeviceMessage>& deviceMessage, TDeviceMessage& tMessage)
{
	if (deviceMessage == 0) {
		return false;
	}

	tMessage.type = convert<DeviceMessageType, TDeviceMessageType>(deviceMessage->getType());
	tMessage.sourceTrace = convert<DeviceTrace, TDeviceTrace>(deviceMessage->getDeviceTrace());

	return true;
}

template<>
bool STI::Network::convert<TDeviceMessage, std::shared_ptr<STI::Device::DeviceMessage>>(const TDeviceMessage& tMessage, std::shared_ptr<STI::Device::DeviceMessage>& deviceMessage)
{
	deviceMessage = std::make_shared<DeviceMessage>(
		convert<TDeviceTrace, DeviceTrace>(tMessage.sourceTrace),
		convert<TDeviceMessageType, DeviceMessageType>(tMessage.type)
		);

	return deviceMessage != 0;
}


//T = Ttype (e.g. TRefreshDeviceMessage), D = derived message type (e.g. RefreshDeviceMessage)
template<typename D, typename T>
bool convertMessage(const std::shared_ptr<STI::Device::DeviceMessage>& deviceMessage, TAnyMessage& tAnyMessage)
{
	bool success = false;

	T tMessage;
	std::shared_ptr<D> dMessage = std::dynamic_pointer_cast<D>(deviceMessage);	//Derved type

	if (dMessage != 0 
		&& convert<std::shared_ptr<DeviceMessage>, TDeviceMessage>(deviceMessage, tMessage.base) //convert base
		&& convert<std::shared_ptr<D>, T>(dMessage, tMessage)) //convert derived
	{
		tAnyMessage.mess <<= tMessage;
		tAnyMessage.type = convert<DeviceMessageType, TDeviceMessageType>(deviceMessage->getType());
		success = true;
	}

	return success;
}



template<>
bool STI::Network::convert<std::shared_ptr<DeviceMessage>, TAnyMessage>(
					const std::shared_ptr<DeviceMessage>& deviceMessage, TAnyMessage& tAnyMessage)
{
	if (deviceMessage == 0) {
		return false;
	}

	bool success = false;

	switch (deviceMessage->getType())
	{
	case DeviceMessageType::Refresh:
		success = convertMessage<RefreshDeviceMessage, TRefreshDeviceMessage>(deviceMessage, tAnyMessage);
		break;
	case DeviceMessageType::EngineScheduler:
		success = convertMessage<EngineSchedulerMessage, TEngineSchedulerMessage>(deviceMessage, tAnyMessage);
		break;
	case DeviceMessageType::EngineParser:
		success = convertMessage<EngineParserDeviceMessage, TEngineParserDeviceMessage>(deviceMessage, tAnyMessage);
		break;
	case DeviceMessageType::AttributeUpdate:
		success = convertMessage<AttributeUpdateMessage, TAttributeUpdateMessage>(deviceMessage, tAnyMessage);
		break;
	case DeviceMessageType::ChannelUpdate:
		success = convertMessage<ChannelUpdateMessage, TChannelUpdateMessage>(deviceMessage, tAnyMessage);
		break;
	case DeviceMessageType::CollectionUpdate:
		success = convertMessage<CollectionUpdateMessage, TCollectionUpdateMessage>(deviceMessage, tAnyMessage);
		break;
	case DeviceMessageType::EngineStatus:
		success = convertMessage<EngineStateMessage, TEngineStateMessage>(deviceMessage, tAnyMessage);
		break;
	case DeviceMessageType::EngineJobUpdate:
		success = convertMessage<EngineJobUpdateDeviceMessage, TEngineJobUpdateDeviceMessage>(deviceMessage, tAnyMessage);
		break;
	}

	return success;
}


//T = Ttype (e.g. TRefreshDeviceMessage), D = Message type (e.g. RefreshDeviceMessage)
template<typename T, typename D>
bool extractMessage(const CORBA::Any& anyMessage, std::shared_ptr<STI::Device::DeviceMessage>& deviceMessage)
{
	bool success = false;

	T* tMess;								//e.g., TRefreshDeviceMessage
	std::shared_ptr<D> mess;					//e.g., RefreshDeviceMessage

	if ((anyMessage >>= tMess) &&	//memory managed by CORBA::Any
		tMess != 0 &&
		convert<T, std::shared_ptr<D>>(*tMess, mess))
	{	
		deviceMessage = mess;
		success = true;
	}
	return success;
}

template<>
bool STI::Network::convert<TAnyMessage, std::shared_ptr<STI::Device::DeviceMessage>>(const TAnyMessage& tAnyMessage, std::shared_ptr<STI::Device::DeviceMessage>& deviceMessage)
{
	bool success = false;

	switch (tAnyMessage.type) {
	case TDeviceMessageType::MessageRefresh:
		success = extractMessage<STI::TNetwork::TRefreshDeviceMessage, STI::Device::RefreshDeviceMessage>(tAnyMessage.mess, deviceMessage);
		break;
	case TDeviceMessageType::MessageEngineScheduler:
		success = extractMessage<TEngineSchedulerMessage, EngineSchedulerMessage>(tAnyMessage.mess, deviceMessage);
		break;
	case TDeviceMessageType::MessageEngineParser:
		success = extractMessage<TEngineParserDeviceMessage, EngineParserDeviceMessage>(tAnyMessage.mess, deviceMessage);
		break;
	case TDeviceMessageType::MessageAttributeUpdate:
		success = extractMessage<TAttributeUpdateMessage, AttributeUpdateMessage>(tAnyMessage.mess, deviceMessage);
		break;
	case TDeviceMessageType::MessageChannelUpdate:
		success = extractMessage<TChannelUpdateMessage, ChannelUpdateMessage>(tAnyMessage.mess, deviceMessage);
		break;
	case TDeviceMessageType::MessageCollectionUpdate:
		success = extractMessage<TCollectionUpdateMessage, CollectionUpdateMessage>(tAnyMessage.mess, deviceMessage);
		break;
	case TDeviceMessageType::MessageEngineStatus:
		success = extractMessage<TEngineStateMessage, EngineStateMessage>(tAnyMessage.mess, deviceMessage);
		break;
	case TDeviceMessageType::MessageEngineJobUpdate:
		success = extractMessage<TEngineJobUpdateDeviceMessage, EngineJobUpdateDeviceMessage>(tAnyMessage.mess, deviceMessage);
		break;
	}

	return success;
}


template<>
bool STI::Network::convert<STI::TNetwork::TRefreshDeviceMessage, std::shared_ptr<STI::Device::RefreshDeviceMessage>>(const STI::TNetwork::TRefreshDeviceMessage& tMessage, std::shared_ptr<STI::Device::RefreshDeviceMessage>& deviceMessage)
{
	deviceMessage = std::make_shared<STI::Device::RefreshDeviceMessage>(
		//convert<TDeviceID, DeviceID>(tMessage.base.sourceID)
		convert<TDeviceTrace, DeviceTrace>(tMessage.base.sourceTrace)
		);

	return deviceMessage != 0;
}

template<>
bool STI::Network::convert<std::shared_ptr<STI::Device::RefreshDeviceMessage>, STI::TNetwork::TRefreshDeviceMessage>(
	const std::shared_ptr<STI::Device::RefreshDeviceMessage>& deviceMessage, STI::TNetwork::TRefreshDeviceMessage& tMessage)
{
	//no other fields
	return (deviceMessage != 0);
}



//CollectionUpdateMessage
template<>
bool STI::Network::convert<TCollectionUpdateMessage, std::shared_ptr<CollectionUpdateMessage>>(
	const TCollectionUpdateMessage& tMessage, std::shared_ptr<CollectionUpdateMessage>& deviceMessage)
{
	deviceMessage = std::make_shared<STI::Device::CollectionUpdateMessage>(
		//convert<TDeviceID, DeviceID>(tMessage.base.sourceID)
		convert<TDeviceTrace, DeviceTrace>(tMessage.base.sourceTrace)
		);

	return deviceMessage != 0;
}


template<>
bool STI::Network::convert<std::shared_ptr<CollectionUpdateMessage>, TCollectionUpdateMessage>(
	const std::shared_ptr<CollectionUpdateMessage>& deviceMessage, TCollectionUpdateMessage& tMessage)
{
	tMessage.collectionUpdateType = convert<CollectionUpdateMessage::CollectionMessageType, TCollectionMessageType>(deviceMessage->updateType);
	return (deviceMessage != 0);
}



//CollectionMessageType
template<>
TCollectionMessageType STI::Network::convert<CollectionUpdateMessage::CollectionMessageType, TCollectionMessageType>(
	const CollectionUpdateMessage::CollectionMessageType& type)
{
	TCollectionMessageType tType;

	switch (type)
	{
	case CollectionUpdateMessage::CollectionMessageType::Add:
		tType = TCollectionMessageType::CollectionMessageAdd;
		break;
	case CollectionUpdateMessage::CollectionMessageType::Remove:
		tType = TCollectionMessageType::CollectionMessageRemove;
		break;
	case CollectionUpdateMessage::CollectionMessageType::Refresh:
		tType = TCollectionMessageType::CollectionMessageRefresh;
		break;
	default:
		tType = TCollectionMessageType::CollectionMessageRefresh;
		break;
	}

	return tType;
}

template<>
CollectionUpdateMessage::CollectionMessageType STI::Network::convert<TCollectionMessageType, CollectionUpdateMessage::CollectionMessageType>(
	const TCollectionMessageType& tType)
{
	CollectionUpdateMessage::CollectionMessageType type;

	switch (tType)
	{
	case TCollectionMessageType::CollectionMessageAdd:
		type = CollectionUpdateMessage::CollectionMessageType::Add;
		break;
	case TCollectionMessageType::CollectionMessageRemove:
		type = CollectionUpdateMessage::CollectionMessageType::Remove;
		break;
	case TCollectionMessageType::CollectionMessageRefresh:
		type = CollectionUpdateMessage::CollectionMessageType::Refresh;
		break;
	default:
		type = CollectionUpdateMessage::CollectionMessageType::Refresh;
		break;
	}

	return type;
}




template<>
bool STI::Network::convert<TEngineSchedulerMessage, std::shared_ptr<EngineSchedulerMessage>>(
			const TEngineSchedulerMessage& tMessage, std::shared_ptr<EngineSchedulerMessage>& deviceMessage)
{

//EngineSchedulerMessage(const STI::Device::DeviceID& source, STI::Device::DeviceID originalSource, const SchedulerMessageType& type)

	deviceMessage = std::make_shared<STI::Device::EngineSchedulerMessage>(
		convert<TDeviceTrace, DeviceTrace>(tMessage.base.sourceTrace),
//		convert<TDeviceID, DeviceID>(tMessage.base.sourceID),
//		convert<TDeviceID, DeviceID>(tMessage.originalSource),
		convert<STI::TNetwork::TSchedulerMessageType, STI::Device::EngineSchedulerMessage::SchedulerMessageType>(tMessage.type)
		);

	if (deviceMessage != 0) {

		convert<STI::TNetwork::TEngineJobID, STI::Engine::EngineJobID>(tMessage.jobID, deviceMessage->jobID);

		std::shared_ptr<STI::Engine::EventEngine> engine;
		if (convert<TEventEngine_var, std::shared_ptr<EventEngine>>(tMessage.engine, engine)) {
			deviceMessage->setEngine(engine);
		}

		// if (!CORBA::is_nil(tMessage.engine)) {
		// 	auto engine = std::make_shared<STI::Network::RemoteEventEngine>(tMessage.engine);
		// 	deviceMessage->setEngine(engine);
		// }
		
		convert<STI::TNetwork::TRawEvent, STI::Engine::RawEvent>(tMessage.handledEvents, deviceMessage->handledEvents);
		convert<STI::TNetwork::TRawEvent, STI::Engine::RawEvent>(tMessage.unhandledEvents, deviceMessage->unhandledEvents);
		convert<STI::TNetwork::TEngineParsingMessage, STI::Engine::EngineParsingMessage>(tMessage.messages, deviceMessage->messages);
		convert<STI::TNetwork::TEngineState, STI::Engine::EngineState>(tMessage.engineState, deviceMessage->engineState);
	}

	return deviceMessage != 0;
}



template<>
bool STI::Network::convert<std::shared_ptr<EngineSchedulerMessage>, TEngineSchedulerMessage>(
			const std::shared_ptr<EngineSchedulerMessage>& deviceMessage, TEngineSchedulerMessage& tMessage)
{
	// STI::Device::DeviceID originalSource;
	// STI::Engine::EngineJobID jobID;
	// std::shared_ptr<STI::Engine::EventEngine> engine;
	// std::vector<STI::Engine::RawEvent> handledEvents;
	// std::vector<STI::Engine::RawEvent> unhandledEvents;

	if (deviceMessage == 0) {
		return false;
	}
	
	//tMessage.originalSource = convert<DeviceID, TDeviceID>(deviceMessage->originalSource);
	tMessage.jobID = convert<STI::Engine::EngineJobID, STI::TNetwork::TEngineJobID>(deviceMessage->jobID);
	convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(deviceMessage->handledEvents, tMessage.handledEvents);
	convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(deviceMessage->unhandledEvents, tMessage.unhandledEvents);
	tMessage.type = convert<STI::Device::EngineSchedulerMessage::SchedulerMessageType, STI::TNetwork::TSchedulerMessageType>(deviceMessage->schedulerMessageType);
	convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(deviceMessage->messages, tMessage.messages);
	convert<STI::Engine::EngineState, STI::TNetwork::TEngineState>(deviceMessage->engineState, tMessage.engineState);

	STI::TNetwork::TEventEngine_var tEngine;
	convert<std::shared_ptr<EventEngine>, TEventEngine_var>(deviceMessage->getEngine(), tEngine);
	tMessage.engine = tEngine;

	// STI::TNetwork::TEventEngine_ptr tEngine;
	// if (STI::Network::NetworkEventEngine::getTEventEngineReference(deviceMessage->getEngine(), tEngine)) {
	// 	tMessage.engine = tEngine;
	// }

	return true;
}


//SchedulerMessageType
template<>
TSchedulerMessageType STI::Network::convert<EngineSchedulerMessage::SchedulerMessageType, TSchedulerMessageType>(const EngineSchedulerMessage::SchedulerMessageType& type)
{
	
	TSchedulerMessageType tType;

	switch (type)
	{
	case EngineSchedulerMessage::SchedulerMessageType::ParseComplete:
		tType = TSchedulerMessageType::SchedulerParseComplete;
		break;
	case EngineSchedulerMessage::SchedulerMessageType::YieldParse:
		tType = TSchedulerMessageType::SchedulerYieldParse;
		break;
	case EngineSchedulerMessage::SchedulerMessageType::PartialParse:
		tType = TSchedulerMessageType::SchedulerPartialParse;
		break;
	case EngineSchedulerMessage::SchedulerMessageType::PlayReady:
		tType = TSchedulerMessageType::SchedulerPlayReady;
		break;
	case EngineSchedulerMessage::SchedulerMessageType::PlayComplete:
		tType = TSchedulerMessageType::SchedulerPlayComplete;
		break;
	case EngineSchedulerMessage::SchedulerMessageType::YieldPlay:
		tType = TSchedulerMessageType::SchedulerYieldPlay;
		break;
	default:
		tType = TSchedulerMessageType::SchedulerYieldParse;
		break;
	}

	return tType;
}

template<>
EngineSchedulerMessage::SchedulerMessageType STI::Network::convert<TSchedulerMessageType, EngineSchedulerMessage::SchedulerMessageType>(const TSchedulerMessageType& tType)
{
	EngineSchedulerMessage::SchedulerMessageType type;

	// SchedulerParseComplete, SchedulerYieldParse, SchedulerPartialParse, SchedulerPlayReady, SchedulerYieldPlay
	switch (tType)
	{
	case TSchedulerMessageType::SchedulerParseComplete:
		type = EngineSchedulerMessage::SchedulerMessageType::ParseComplete;
		break;
	case TSchedulerMessageType::SchedulerYieldParse:
		type = EngineSchedulerMessage::SchedulerMessageType::YieldParse;
		break;
	case TSchedulerMessageType::SchedulerPartialParse:
		type = EngineSchedulerMessage::SchedulerMessageType::PartialParse;
		break;
	case TSchedulerMessageType::SchedulerPlayReady:
		type = EngineSchedulerMessage::SchedulerMessageType::PlayReady;
		break;
	case TSchedulerMessageType::SchedulerPlayComplete:
		type = EngineSchedulerMessage::SchedulerMessageType::PlayComplete;
		break;
	case TSchedulerMessageType::SchedulerYieldPlay:
		type = EngineSchedulerMessage::SchedulerMessageType::YieldPlay;
		break;
	default:
		type = EngineSchedulerMessage::SchedulerMessageType::YieldParse;
		break;
	}

	return type;
}


//EngineParserDeviceMessage
template<>
bool STI::Network::convert<TEngineParserDeviceMessage, std::shared_ptr<EngineParserDeviceMessage>>(
	const TEngineParserDeviceMessage& tMessage, std::shared_ptr<EngineParserDeviceMessage>& deviceMessage)
{
	// STI::Engine::ParseID pid;
	// std::vector<STI::Engine::EngineParsingMessage> messages;

	deviceMessage = std::make_shared<EngineParserDeviceMessage>(
		convert<TDeviceTrace, DeviceTrace>(tMessage.base.sourceTrace),
//		convert<TDeviceID, DeviceID>(tMessage.base.sourceID),
		convert<TParseID, ParseID>(tMessage.pid)
		);

	// if (deviceMessage == 0) {
	// 	return false;
	// }

	// deviceMessage->pid = convert<TParseID, ParseID>(tMessage.pid);

	bool success = convert<STI::TNetwork::TEngineParsingMessage, STI::Engine::EngineParsingMessage>(tMessage.messages, deviceMessage->messages);
	return success;
}

template<>
bool STI::Network::convert<std::shared_ptr<EngineParserDeviceMessage>, TEngineParserDeviceMessage>(
	const std::shared_ptr<EngineParserDeviceMessage>& deviceMessage, TEngineParserDeviceMessage& tMessage)
{
	if (deviceMessage == 0) {
		return false;
	}
	
	tMessage.pid = convert<ParseID, TParseID>(deviceMessage->pid);
	
	return convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(deviceMessage->messages, tMessage.messages);
}



//ChannelUpdateMessage
template<>
bool STI::Network::convert<TChannelUpdateMessage, std::shared_ptr<ChannelUpdateMessage>>(
	const TChannelUpdateMessage& tMessage, std::shared_ptr<ChannelUpdateMessage>& deviceMessage)
{
	deviceMessage = std::make_shared<ChannelUpdateMessage>(
		convert<TDeviceTrace, DeviceTrace>(tMessage.base.sourceTrace)
//		convert<TDeviceID, DeviceID>(tMessage.base.sourceID)		
		);

	deviceMessage->channelUpdateType = convert<TChannelUpdateMessageType, ChannelUpdateMessage::ChannelUpdateMessageType>(tMessage.channelUpdateType);

	if (deviceMessage->channelUpdateType == ChannelUpdateMessage::ChannelUpdateMessageType::ChannelValue) {
		//channel value message
		for(unsigned i = 0; i < tMessage.channelValues.length(); ++i) {

			deviceMessage->channelValues.insert(
				std::pair<short, MixedValue>(
					static_cast<short>(tMessage.channelValues[i].channelNumber),
					convert<TMixedValue, MixedValue>(tMessage.channelValues[i].value)
				));
		}		
	}
	else {
		//channel name message
		deviceMessage->channelNumber = static_cast<short>(tMessage.channelNumber);
		deviceMessage->channelName = convert<CORBA::String_member, std::string>(tMessage.channelName);
	}

	return (deviceMessage != 0);
}

template<>
bool STI::Network::convert<std::shared_ptr<ChannelUpdateMessage>, TChannelUpdateMessage>(
	const std::shared_ptr<ChannelUpdateMessage>& deviceMessage, TChannelUpdateMessage& tMessage)
{
	if (deviceMessage == 0) {
		return false;
	}

	tMessage.channelUpdateType = convert<ChannelUpdateMessage::ChannelUpdateMessageType, TChannelUpdateMessageType>(deviceMessage->channelUpdateType);

	tMessage.channelValues.length( static_cast<unsigned>(deviceMessage->channelValues.size()) );

	unsigned i = 0;
	for (auto& ch : deviceMessage->channelValues) {
		if (i < tMessage.channelValues.length()) {
			tMessage.channelValues[i].channelNumber = static_cast<CORBA::Short>(ch.first);
			tMessage.channelValues[i].value = convert<MixedValue, TMixedValue>(ch.second);
		}
		++i;
	}

	tMessage.channelNumber = static_cast<CORBA::Short>(deviceMessage->channelNumber);
	tMessage.channelName = convert<std::string, CORBA::String_member>(deviceMessage->channelName);

	return true;
}

//ChannelUpdateMessageType
template<>
TChannelUpdateMessageType STI::Network::convert<ChannelUpdateMessage::ChannelUpdateMessageType, TChannelUpdateMessageType>(const ChannelUpdateMessage::ChannelUpdateMessageType& type)
{
	TChannelUpdateMessageType tType;

	switch (type)
	{
	case ChannelUpdateMessage::ChannelUpdateMessageType::ChannelValue:
		tType = TChannelUpdateMessageType::ChannelUpdataValue;
		break;
	case ChannelUpdateMessage::ChannelUpdateMessageType::ChannelName:
		tType = TChannelUpdateMessageType::ChannelUpdateName;
		break;
	default:
		tType = TChannelUpdateMessageType::ChannelUpdataValue;
		break;
	}

	return tType;
}

template<>
ChannelUpdateMessage::ChannelUpdateMessageType STI::Network::convert<TChannelUpdateMessageType, ChannelUpdateMessage::ChannelUpdateMessageType>(const TChannelUpdateMessageType& tType)
{
	ChannelUpdateMessage::ChannelUpdateMessageType type;

	switch (tType)
	{
	case TChannelUpdateMessageType::ChannelUpdataValue:
		type = ChannelUpdateMessage::ChannelUpdateMessageType::ChannelValue;
		break;
	case TChannelUpdateMessageType::ChannelUpdateName:
		type = ChannelUpdateMessage::ChannelUpdateMessageType::ChannelName;
		break;
	default:
		type = ChannelUpdateMessage::ChannelUpdateMessageType::ChannelValue;
		break;
	}

	return type;
}


//AttributeUpdateMessage
template<>
bool STI::Network::convert<TAttributeUpdateMessage, std::shared_ptr<AttributeUpdateMessage>>(
	const TAttributeUpdateMessage& tMessage, std::shared_ptr<AttributeUpdateMessage>& deviceMessage)
{
	deviceMessage = std::make_shared<AttributeUpdateMessage>(
		convert<TDeviceTrace, DeviceTrace>(tMessage.base.sourceTrace)
//		convert<TDeviceID, DeviceID>(tMessage.base.sourceID)		
		);

	for(unsigned i = 0; i < tMessage.attributes.length(); ++i) {

		deviceMessage->attributes.insert(
			std::pair<std::string, std::string>(
				convert<CORBA::String_member, std::string>(tMessage.attributes[i].key),
				convert<CORBA::String_member, std::string>(tMessage.attributes[i].value)
			));
	}

	return (deviceMessage != 0);
}

template<>
bool STI::Network::convert<std::shared_ptr<AttributeUpdateMessage>, TAttributeUpdateMessage>(
	const std::shared_ptr<AttributeUpdateMessage>& deviceMessage, TAttributeUpdateMessage& tMessage)
{
	if (deviceMessage == 0) {
		return false;
	}

	tMessage.attributes.length( static_cast<unsigned>(deviceMessage->attributes.size()) );

	unsigned i = 0;
	for (auto& attrib : deviceMessage->attributes) {
		if (i < tMessage.attributes.length()) {
			tMessage.attributes[i].key = convert<std::string, CORBA::String_member>(attrib.first);
			tMessage.attributes[i].key = convert<std::string, CORBA::String_member>(attrib.second);
		}
		++i;
	}

	return true;
}



//EngineStateMessage
template<>
bool STI::Network::convert<TEngineStateMessage, std::shared_ptr<EngineStateMessage>>(
	const TEngineStateMessage& tMessage, std::shared_ptr<EngineStateMessage>& deviceMessage)
{

	deviceMessage = std::make_shared<EngineStateMessage>(
		convert<TDeviceTrace, DeviceTrace>(tMessage.base.sourceTrace)
		);

	for(unsigned i = 0; i < tMessage.engineStates.length(); ++i) {

		deviceMessage->engineStates.insert(
			std::pair<EngineID, EngineState>(
				convert<TEngineID, EngineID>(tMessage.engineStates[i].engineID),
				convert<TEngineState, EngineState>(tMessage.engineStates[i].state)
			));
	}

	return (deviceMessage != 0);
}

template<>
bool STI::Network::convert<std::shared_ptr<EngineStateMessage>, TEngineStateMessage>(
	const std::shared_ptr<EngineStateMessage>& deviceMessage, TEngineStateMessage& tMessage)
{
	if (deviceMessage == 0) {
		return false;
	}

	tMessage.engineStates.length( static_cast<unsigned>(deviceMessage->engineStates.size()) );

	unsigned i = 0;
	for (auto& tuple : deviceMessage->engineStates) {
		if (i < tMessage.engineStates.length()) {
			tMessage.engineStates[i].engineID = convert<EngineID, TEngineID>(tuple.first);
			tMessage.engineStates[i].state = convert<EngineState, TEngineState>(tuple.second);
		}
		++i;
	}

	return true;
}



//EngineJobUpdateDeviceMessage
template<>
bool STI::Network::convert<TEngineJobUpdateDeviceMessage, std::shared_ptr<EngineJobUpdateDeviceMessage>>(
	const TEngineJobUpdateDeviceMessage& tMessage, std::shared_ptr<EngineJobUpdateDeviceMessage>& deviceMessage)
{
	std::shared_ptr<EventEngineJob> engineJob;

	convert<TEventEngineJob, std::shared_ptr<EventEngineJob>>(tMessage.engineJob, engineJob);

	deviceMessage = std::make_shared<EngineJobUpdateDeviceMessage>(
		convert<TDeviceTrace, DeviceTrace>(tMessage.base.sourceTrace),
		engineJob,
		convert<TEngineJobUpdateTarget, EngineJobUpdateTarget>(tMessage.targetList)
		);

	// deviceMessage->targetList = convert<TEngineJobUpdateTarget, EngineJobUpdateTarget>(
	// 							tMessage.targetList);

	// return convert<TEventEngineJob, std::shared_ptr<EventEngineJob>>(tMessage.engineJob, deviceMessage->engineJob)
	// 		&&  (deviceMessage != 0);

	return (deviceMessage != 0);
}

template<>
bool STI::Network::convert<std::shared_ptr<EngineJobUpdateDeviceMessage>, TEngineJobUpdateDeviceMessage>(
	const std::shared_ptr<EngineJobUpdateDeviceMessage>& deviceMessage, TEngineJobUpdateDeviceMessage& tMessage)
{
	if (deviceMessage == 0) {
		return false;
	}

	tMessage.targetList = convert<EngineJobUpdateTarget, TEngineJobUpdateTarget>(
								deviceMessage->getTargetList());

	return convert<std::shared_ptr<EventEngineJob>, TEventEngineJob>(deviceMessage->getEngineJob(), tMessage.engineJob);
}


//EngineJobUpdateTarget
template<>
TEngineJobUpdateTarget STI::Network::convert
	<EngineJobUpdateTarget, TEngineJobUpdateTarget>(
		const EngineJobUpdateTarget& type)
{
	TEngineJobUpdateTarget tType;

	switch (type)
	{
	case EngineJobUpdateTarget::Queued:
		tType = TEngineJobUpdateTarget::JobUpdateTargetQueued;
		break;
	case EngineJobUpdateTarget::Running:
		tType = TEngineJobUpdateTarget::JobUpdateTargetRunning;
		break;
	case EngineJobUpdateTarget::Completed:
		tType = TEngineJobUpdateTarget::JobUpdateTargetCompleted;
		break;
	default:
		tType = TEngineJobUpdateTarget::JobUpdateTargetCompleted;
		break;
	}

	return tType;
}

template<>
EngineJobUpdateTarget STI::Network::convert
	<TEngineJobUpdateTarget, EngineJobUpdateTarget>(
		const TEngineJobUpdateTarget& tType)
{
	EngineJobUpdateTarget type;

	switch (tType)
	{
	case TEngineJobUpdateTarget::JobUpdateTargetQueued:
		type = EngineJobUpdateTarget::Queued;
		break;
	case TEngineJobUpdateTarget::JobUpdateTargetRunning:
		type = EngineJobUpdateTarget::Running;
		break;
	case TEngineJobUpdateTarget::JobUpdateTargetCompleted:
		type = EngineJobUpdateTarget::Completed;
		break;
	default:
		type = EngineJobUpdateTarget::Completed;
		break;
	}

	return type;
}

