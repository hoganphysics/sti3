
#include "DeviceEventParser.h"
#include "RawEvent.h"
#include "DeviceID.h"

using STI::Engine::DeviceEventParser;
using STI::Engine::DeviceEventMap;
using STI::Engine::RawEvent;

void DeviceEventParser::setPartnerEventTarget(DeviceEventMap* target)
{
	_target = target;
}

void DeviceEventParser::addEvent(const RawEvent& evt, const RawEvent& referenceEvent)
{
	//RawEvent newEvent(evt, referenceEvent, eventNumber);
	//_target->insert({ evt.targetDevice(), newEvent });

	if (_target != nullptr) {
		//_target->emplace(std::piecewise_construct,
		//	std::make_tuple(evt.targetDevice()),
		//	std::make_tuple(evt, referenceEvent, eventNumber));
		
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