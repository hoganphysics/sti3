
#include "NetworkConvert.h"
#include "DeviceEvent.h"
#include "DeviceID.h"
#include "RemoteEventEngine.h"
#include "Convert_EventEngine.h"
#include "Convert_DeviceMessage.h"
#include "NetworkEventEngine.h"

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

using STI::Engine::EventEngineJob;
using STI::TNetwork::TEventEngineJob;

using STI::TNetwork::TEngineSchedulerMessage;
using STI::Device::EngineSchedulerMessage;

using STI::Device::EngineSchedulerMessage;
using STI::TNetwork::TSchedulerMessageType;


template<>
TDeviceEventType STI::Network::convert<DeviceEventType, TDeviceEventType>(const DeviceEventType& type)
{
	TDeviceEventType tType;

	switch (type)
	{
		case DeviceEventType::Refresh:
			tType = TDeviceEventType::MessageRefresh;
			break;
		case DeviceEventType::CollectionUpdate:
			tType = TDeviceEventType::MessageCollectionUpdate;
			break;
		case DeviceEventType::ChannelUpdate:
			tType = TDeviceEventType::MessageChannelUpdate;
			break;
		case DeviceEventType::ChannelsRefresh:
			tType = TDeviceEventType::MessageChannelsRefresh;
			break;
		case DeviceEventType::AttributeUpdate:
			tType = TDeviceEventType::MessageAttributeUpdate;
			break;
		case DeviceEventType::AttributesRefresh:
			tType = TDeviceEventType::MessageAttributesRefresh;
			break;
		case DeviceEventType::MonitorUpdate:
			tType = TDeviceEventType::MessageMonitorUpdate;
			break;
		case DeviceEventType::EngineScheduler:
			tType = TDeviceEventType::MessageEngineScheduler;
			break;
		case DeviceEventType::EngineParser:
			tType = TDeviceEventType::MessageEngineParser;
			break;
		case DeviceEventType::EngineStatus:
			tType = TDeviceEventType::MessageEngineStatus;
			break;
		default:
			tType = TDeviceEventType::MessageUnknown;
			break;
	}

	return tType;
}

template<>
DeviceEventType STI::Network::convert<TDeviceEventType, DeviceEventType>(const TDeviceEventType& tType)
{
	DeviceEventType type;

	switch (tType)
	{
		case TDeviceEventType::MessageRefresh:
			type = DeviceEventType::Refresh;
			break;
		case TDeviceEventType::MessageCollectionUpdate:
			type = DeviceEventType::CollectionUpdate;
			break;
		case TDeviceEventType::MessageChannelUpdate:
			type = DeviceEventType::ChannelUpdate;
			break;
		case TDeviceEventType::MessageChannelsRefresh:
			type = DeviceEventType::ChannelsRefresh;
			break;
		case TDeviceEventType::MessageAttributeUpdate:
			type = DeviceEventType::AttributeUpdate;
			break;
		case TDeviceEventType::MessageAttributesRefresh:
			type = DeviceEventType::AttributesRefresh;
			break;
		case TDeviceEventType::MessageMonitorUpdate:
			type = DeviceEventType::MonitorUpdate;
			break;
		case TDeviceEventType::MessageEngineScheduler:
			type = DeviceEventType::EngineScheduler;
			break;
		case TDeviceEventType::MessageEngineParser:
			type = DeviceEventType::EngineParser;
			break;
		case TDeviceEventType::MessageEngineStatus:
			type = DeviceEventType::EngineStatus;
			break;
		default:
			type = DeviceEventType::Unknown;
			break;
	}

	return type;
}

template<>
bool STI::Network::convert<DeviceEventType, TDeviceEventType>(const DeviceEventType& type, TDeviceEventType& tType)
{
	tType = convert<DeviceEventType, TDeviceEventType>(type);
	return true;
}

template<>
bool STI::Network::convert<TDeviceEventType, DeviceEventType>(const TDeviceEventType& tType, DeviceEventType& type)
{
	type = convert<TDeviceEventType, DeviceEventType>(tType);
	return true;
}


template<>
bool STI::Network::convert<std::shared_ptr<STI::Device::DeviceEvent>, TDeviceEvent>(const std::shared_ptr<STI::Device::DeviceEvent>& deviceEvent, TDeviceEvent& tEvent)
{
	if (deviceEvent == 0) {
		return false;
	}

	tEvent.type = convert<DeviceEventType, TDeviceEventType>(deviceEvent->getType());
	tEvent.sourceID = convert<DeviceID, TDeviceID>(deviceEvent->sourceID());

	return true;
}

template<>
bool STI::Network::convert<TDeviceEvent, std::shared_ptr<STI::Device::DeviceEvent>>(const TDeviceEvent& tEvent, std::shared_ptr<STI::Device::DeviceEvent>& deviceEvent)
{
	deviceEvent = std::make_shared<DeviceEvent>(
		convert<TDeviceID, DeviceID>(tEvent.sourceID),
		convert<TDeviceEventType, DeviceEventType>(tEvent.type)
		);

	return deviceEvent != 0;
}


//T = Ttype (e.g. TRefreshDeviceEvent), D = derived message type (e.g. RefreshDeviceEvent)
template<typename D, typename T>
bool convertMessage(const std::shared_ptr<STI::Device::DeviceEvent>& deviceMessage, TAnyEvent& tAnyEvent)
{
	bool success = false;

	T tMessage;
	std::shared_ptr<D> dMessage = std::dynamic_pointer_cast<D>(deviceMessage);	//Derved type

	if (dMessage != 0 
		&& convert<std::shared_ptr<DeviceEvent>, TDeviceEvent>(deviceMessage, tMessage.base) //convert base
		&& convert<std::shared_ptr<D>, T>(dMessage, tMessage)) //convert derived
	{
		tAnyEvent.evt <<= tMessage;
		tAnyEvent.type = convert<DeviceEventType, TDeviceEventType>(deviceMessage->getType());
		success = true;
	}

	return success;
}



template<>
bool STI::Network::convert<std::shared_ptr<DeviceEvent>, TAnyEvent>(const std::shared_ptr<DeviceEvent>& deviceEvent, TAnyEvent& tAnyEvent)
{
	if (deviceEvent == 0) {
		return false;
	}

	bool success = false;

	switch (deviceEvent->getType())
	{
	case DeviceEventType::Refresh:
		{
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
		break;
	case DeviceEventType::EngineScheduler:
		success = convertMessage<EngineSchedulerMessage, TEngineSchedulerMessage>(deviceEvent, tAnyEvent);
		break;
	}


	return success;
}


//T = Ttype (e.g. TRefreshDeviceEvent), D = Message type (e.g. RefreshDeviceEvent)
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
bool STI::Network::convert<TAnyEvent, std::shared_ptr<STI::Device::DeviceEvent>>(const TAnyEvent& tAnyEvent, std::shared_ptr<STI::Device::DeviceEvent>& deviceEvent)
{
	bool success = false;

	switch (tAnyEvent.type) {
	case TDeviceEventType::MessageRefresh:

		success = extractEvent<STI::TNetwork::TRefreshDeviceEvent, STI::Device::RefreshDeviceEvent>(tAnyEvent.evt, deviceEvent);

		//STI::TNetwork::TRefreshDeviceEvent* evt;
		////convert<CORBA::Any, TRefreshDeviceEvent>(tAnyEvent.evt, evt);			//extract from any
		////deviceEvent = convert<TRefreshDeviceEvent, RefreshDeviceEvent>(evt)

		//if (tAnyEvent.evt >>= evt) {	//memory managed by CORBA::Any

		//}
		break;
	case TDeviceEventType::MessageEngineScheduler:

		success = extractEvent<TEngineSchedulerMessage, EngineSchedulerMessage>(tAnyEvent.evt, deviceEvent);

		break;
	}

	return success;
}


template<>
bool STI::Network::convert<STI::TNetwork::TRefreshDeviceEvent, std::shared_ptr<STI::Device::RefreshDeviceEvent>>(const STI::TNetwork::TRefreshDeviceEvent& tEvent, std::shared_ptr<STI::Device::RefreshDeviceEvent>& deviceEvent)
{
	deviceEvent = std::make_shared<STI::Device::RefreshDeviceEvent>(
		convert<TDeviceID, DeviceID>(tEvent.base.sourceID)
		);

	return deviceEvent != 0;
}

template<>
bool STI::Network::convert<std::shared_ptr<STI::Device::RefreshDeviceEvent>, STI::TNetwork::TRefreshDeviceEvent>(
	const std::shared_ptr<STI::Device::RefreshDeviceEvent>& deviceMessage, STI::TNetwork::TRefreshDeviceEvent& tMessage)
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



