
#include <sti/engine/DeviceEventParser.h>
#include <sti/engine/RawEvent.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/EngineParsingMessage.h>
#include "RawEventGroup.h"
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/RawEventTargetDevice.h>

using STI::Engine::DeviceEventParser;
using STI::Engine::DeviceEventMap;
using STI::Engine::RawEventMap;
using STI::Engine::RawEvent;
using STI::Engine::SynchronousEventVector;
using STI::Engine::EngineParsingMessage;


void DeviceEventParser::parseEvents(const RawEventMap& events, 
					SynchronousEventVector& synchedEvents, STI::Device::DeviceID deviceID, const STI::Engine::EngineID& engineID, DeviceEventMap* target)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);
	
	clearEventNumber();

	setPartnerEventTarget(target);

	currentEngineID = engineID;
	localDeviceID = deviceID;

	parseEvents(events, synchedEvents);		//call pure virtual
}


void DeviceEventParser::setPartnerEventTarget(DeviceEventMap* target)
{
	_target = target;
}

void DeviceEventParser::addEvent(const RawEvent& evt, const RawEvent& referenceEvent)
{
	if (_target != nullptr) {
		
		//(*_target) is map<DeviceID, vector<RawEvent>>

		//emplace_back appends to the raw event vector by calling the following constuctor:
		//RawEvent(evt, referenceEvent, eventNumber).
		//This avoids constructing a temp RawEvent and then deep copying to the vector.


		
		// std::string subgroupName = referenceEvent.getGroupName();	//Todo: need to remove top group name...
		(*_target)[evt.target().device().deviceID()]->addEvent(RawEvent(evt, referenceEvent, eventNumber));

		eventNumber++;
	}
}

EngineParsingMessage& DeviceEventParser::addInfo(unsigned id, const std::string& name)
{
	parsingMessages.emplace_back(localDeviceID, ParsingMessageType::Information, id, name);
	return parsingMessages.back();
}

EngineParsingMessage& DeviceEventParser::addWarning(unsigned id, const std::string& name)
{
	parsingMessages.emplace_back(localDeviceID, ParsingMessageType::Warning, id, name);
	return parsingMessages.back();
}

void DeviceEventParser::clearEventNumber()
{
	eventNumber = 0;
}
