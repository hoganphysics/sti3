
#include <sti/engine/DeviceEventParser.h>

#include <sti/device/DeviceID.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/RawEventTargetDevice.h>

#include <sti/engine/RawEventGroup.h>

#include <memory>

using STI::Engine::DeviceEventParser;
using STI::Engine::DeviceEventMap;
using STI::Engine::RawEventMap;
using STI::Engine::RawEvent;
using STI::Engine::SynchronousEventVector;
using STI::Engine::EngineParsingMessage;

DeviceEventParser::DeviceEventParser()
{
	parsing = false;
	eventNumber = 0;
	partnerEventTarget = 0;
}

void DeviceEventParser::parseEvents(const RawEventMap& events, 
					SynchronousEventVector& synchedEvents, STI::Device::DeviceID deviceID, const STI::Engine::EngineID& engineID, DeviceEventMap* target)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);		//ensure only one engine can parse at a time

	parsing = true;

	clearEventNumber();
	parsingMessages.clear();

	setPartnerEventTarget(target);

	currentEngineID = engineID;
	localDeviceID = deviceID;

	parseEvents(events, synchedEvents);		//call pure virtual

	parsing = false;
}


void DeviceEventParser::setPartnerEventTarget(DeviceEventMap* target)
{
	partnerEventTarget = target;
}

void DeviceEventParser::addEvent(const RawEvent& evt, const RawEvent& referenceEvent)
{
	if (!parsing) return;
	if (partnerEventTarget == 0) return;

	std::shared_ptr<RawEventGroup> partnerGroup;
	auto it = partnerEventTarget->find(evt.target().device().deviceID());

	if (it == partnerEventTarget->end()) {
		//Not found (first event for this partner). Make the group for this partner's events.
		partnerGroup = std::make_shared<RawEventGroup>();
		(*partnerEventTarget)[evt.target().device().deviceID()] = partnerGroup;
	}
	else {
		partnerGroup = it->second;
	}

	if (partnerGroup != 0) {

		partnerGroup->addEvent(RawEvent(evt, referenceEvent, eventNumber));
		eventNumber++;
	}

	//if (parsing && partnerEventTarget != nullptr) {
	//	
	//	(*partnerEventTarget)[evt.target().device().deviceID()]->addEvent(RawEvent(evt, referenceEvent, eventNumber));

	//	eventNumber++;
	//}
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
