
#include "DeviceEventParser.h"
#include "RawEvent.h"
#include "DeviceID.h"

using STI::Engine::DeviceEventParser;
using STI::Engine::DeviceEventMap;
using STI::Engine::RawEventMap;
using STI::Engine::RawEvent;
using STI::Engine::SynchronousEventVector;



void DeviceEventParser::parseEvents(const RawEventMap& events, 
					SynchronousEventVector& synchedEvents, const STI::Engine::EngineID& engineID, DeviceEventMap* target)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);
	
	clearEventNumber();

	setPartnerEventTarget(target);

	currentEngineID = engineID;

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
		(*_target)[evt.targetDevice()].emplace_back(evt, referenceEvent, eventNumber);

		eventNumber++;
	}
}

void DeviceEventParser::clearEventNumber()
{
	eventNumber = 0;
}
