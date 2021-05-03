
//#include "NetworkConvert.h"
#include "DeviceMessage.h"
#include "DeviceID.h"
#include "RemoteEventEngine.h"
#include "Convert_EventEngine.h"
#include "Convert_DeviceMessage.h"
#include "NetworkEventEngine.h"

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

using STI::Engine::EventEngineJob;
using STI::TNetwork::TEventEngineJob;

using STI::TNetwork::TEngineSchedulerMessage;
using STI::Device::EngineSchedulerMessage;

using STI::Device::EngineSchedulerMessage;
using STI::TNetwork::TSchedulerMessageType;

using STI::Device::EngineParserDeviceMessage;
using STI::TNetwork::TEngineParserDeviceMessage;

using STI::Engine::ParseID;
using STI::TNetwork::TParseID;

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
	tMessage.sourceID = convert<DeviceID, TDeviceID>(deviceMessage->sourceID());

	return true;
}

template<>
bool STI::Network::convert<TDeviceMessage, std::shared_ptr<STI::Device::DeviceMessage>>(const TDeviceMessage& tMessage, std::shared_ptr<STI::Device::DeviceMessage>& deviceMessage)
{
	deviceMessage = std::make_shared<DeviceMessage>(
		convert<TDeviceID, DeviceID>(tMessage.sourceID),
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
		{
			STI::TNetwork::TRefreshDeviceMessage tRefreshMess;
			auto rde = std::dynamic_pointer_cast<STI::Device::RefreshDeviceMessage>(deviceMessage);
			if (rde != 0 &&
				convert<std::shared_ptr<DeviceMessage>, TDeviceMessage>(deviceMessage, tRefreshMess.base)) 
			{
	//			tRefreshEvt.base.type = convert<DeviceMessageType, TDeviceMessageType>(deviceMessage->getType());
		//		tRefreshEvt.base.sourceID = convert<DeviceID, TDeviceID>(deviceMessage->sourceID());
				tAnyMessage.mess <<= tRefreshMess;
				tAnyMessage.type = convert<DeviceMessageType, TDeviceMessageType>(deviceMessage->getType());
				success = true;
				//convert<std::shared_ptr<STI::Device::RefreshDeviceMessage>, STI::TNetwork::TRefreshDeviceMessage>(rde, tRefreshEvt);
			}
		}
		break;
	case DeviceMessageType::EngineScheduler:
		success = convertMessage<EngineSchedulerMessage, TEngineSchedulerMessage>(deviceMessage, tAnyMessage);
		break;
	case DeviceMessageType::EngineParser:
		success = convertMessage<EngineParserDeviceMessage, TEngineParserDeviceMessage>(deviceMessage, tAnyMessage);
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

		//STI::TNetwork::TRefreshDeviceMessage* evt;
		////convert<CORBA::Any, TRefreshDeviceMessage>(tAnyMessage.evt, evt);			//extract from any
		////deviceMessage = convert<TRefreshDeviceMessage, RefreshDeviceMessage>(evt)

		//if (tAnyMessage.evt >>= evt) {	//memory managed by CORBA::Any

		//}
		break;
	case TDeviceMessageType::MessageEngineScheduler:
		success = extractMessage<TEngineSchedulerMessage, EngineSchedulerMessage>(tAnyMessage.mess, deviceMessage);
		break;
	case TDeviceMessageType::MessageEngineParser:
		success = extractMessage<TEngineParserDeviceMessage, EngineParserDeviceMessage>(tAnyMessage.mess, deviceMessage);
		break;
	}

	return success;
}


template<>
bool STI::Network::convert<STI::TNetwork::TRefreshDeviceMessage, std::shared_ptr<STI::Device::RefreshDeviceMessage>>(const STI::TNetwork::TRefreshDeviceMessage& tMessage, std::shared_ptr<STI::Device::RefreshDeviceMessage>& deviceMessage)
{
	deviceMessage = std::make_shared<STI::Device::RefreshDeviceMessage>(
		convert<TDeviceID, DeviceID>(tMessage.base.sourceID)
		);

	return deviceMessage != 0;
}

template<>
bool STI::Network::convert<std::shared_ptr<STI::Device::RefreshDeviceMessage>, STI::TNetwork::TRefreshDeviceMessage>(
	const std::shared_ptr<STI::Device::RefreshDeviceMessage>& deviceMessage, STI::TNetwork::TRefreshDeviceMessage& tMessage)
{
	//no other fields
	return true;
}



template<>
bool STI::Network::convert<TEngineSchedulerMessage, std::shared_ptr<EngineSchedulerMessage>>(
			const TEngineSchedulerMessage& tMessage, std::shared_ptr<EngineSchedulerMessage>& deviceMessage)
{

//EngineSchedulerMessage(const STI::Device::DeviceID& source, STI::Device::DeviceID originalSource, const SchedulerMessageType& type)

	deviceMessage = std::make_shared<STI::Device::EngineSchedulerMessage>(
		convert<TDeviceID, DeviceID>(tMessage.base.sourceID),
		convert<TDeviceID, DeviceID>(tMessage.originalSource),
		convert<STI::TNetwork::TSchedulerMessageType, STI::Device::EngineSchedulerMessage::SchedulerMessageType>(tMessage.type)
		);

	if (deviceMessage != 0) {

		convert<STI::TNetwork::TEngineJobID, STI::Engine::EngineJobID>(tMessage.jobID, deviceMessage->jobID);

		if (!CORBA::is_nil(tMessage.engine)) {
			deviceMessage->engine = std::make_shared<STI::Network::RemoteEventEngine>(tMessage.engine);
		}
		
		convert<STI::TNetwork::TRawEvent, STI::Engine::RawEvent>(tMessage.handledEvents, deviceMessage->handledEvents);
		convert<STI::TNetwork::TRawEvent, STI::Engine::RawEvent>(tMessage.unhandledEvents, deviceMessage->unhandledEvents);
		convert<STI::TNetwork::TEngineParsingMessage, STI::Engine::EngineParsingMessage>(tMessage.messages, deviceMessage->messages);
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
	
	tMessage.originalSource = convert<DeviceID, TDeviceID>(deviceMessage->originalSource);
	tMessage.jobID = convert<STI::Engine::EngineJobID, STI::TNetwork::TEngineJobID>(deviceMessage->jobID);
	convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(deviceMessage->handledEvents, tMessage.handledEvents);
	convert<STI::Engine::RawEvent, STI::TNetwork::TRawEvent>(deviceMessage->unhandledEvents, tMessage.unhandledEvents);
	tMessage.type = convert<STI::Device::EngineSchedulerMessage::SchedulerMessageType, STI::TNetwork::TSchedulerMessageType>(deviceMessage->schedulerMessageType);
	convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(deviceMessage->messages, tMessage.messages);

	STI::TNetwork::TEventEngine_ptr tEngine;
	if (STI::Network::NetworkEventEngine::getTEventEngineReference(deviceMessage->engine, tEngine)) {
		tMessage.engine = tEngine;
	}

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

	deviceMessage->pid = convert<TParseID, ParseID>(tMessage.pid);

	return convert<STI::TNetwork::TEngineParsingMessage, STI::Engine::EngineParsingMessage>(tMessage.messages, deviceMessage->messages);
}

template<>
bool STI::Network::convert<std::shared_ptr<EngineParserDeviceMessage>, TEngineParserDeviceMessage>(
	const std::shared_ptr<EngineParserDeviceMessage>& deviceMessage, TEngineParserDeviceMessage& tMessage)
{
	tMessage.pid = convert<ParseID, TParseID>(deviceMessage->pid);
	
	return convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(deviceMessage->messages, tMessage.messages);
}


