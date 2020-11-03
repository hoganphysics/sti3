#ifndef STI_ENGINE_EVENTENGINEPARSER_H
#define STI_ENGINE_EVENTENGINEPARSER_H

#include "fwd/RawEvent_fwd.h"
#include "fwd/Measurement_fwd.h"
#include "fwd/SynchronousEvent_fwd.h"
#include "fwd/DeviceID_fwd.h"

#include "DeviceEventParser.h"
#include "utils/GraphPathLabel.h"
#include "EngineParsingError.h"

#include <string>
#include <sstream>
#include <set>

namespace STI
{
namespace Engine
{

class EngineParsingError;
class LocalEventEngine;

class EventEngineParser
{
public:

	EventEngineParser(LocalEventEngine* engine, DeviceEventParser* deviceParser);
	~EventEngineParser();

	bool parse(const STI::Engine::RawEventVector& events, SynchronousEventVector& synchedEvents);
	void getEventTargets(std::set<STI::Device::DeviceID>& targetIDs);

	RawEventMap rawEvents;
	DeviceEventMap partnerEvents;

private:

	bool addRawEvent(const RawEvent& rawEvent, unsigned& errorCount, unsigned maxErrors);

	bool groupEventsByTime(const STI::Engine::RawEventVector& events);
	bool parseEvents(SynchronousEventVector& synchedEvents);
	bool checkMeasurements(SynchronousEventVector& synchedEvents);

	struct MeasurementCounter
	{
		MeasurementCounter(const RawEvent* rawEvent) 
			: rawEvent(rawEvent), count(0) {}

		const RawEvent* rawEvent;
		unsigned count;		//number of SynchronousEvents that reference this rawEvent
	};

	std::map<STI::Utils::GraphPathLabel, MeasurementCounter> measurementEventGraph;
//	std::map<STI::Utils::GraphPathLabel, const RawEvent*> measurementEventGraph;

	bool countMeasurementRefs(const std::vector<std::shared_ptr<Measurement>>& measurements);
	bool maxErrorCheck(unsigned errorCount, unsigned maxErrors);

	std::vector<EngineParsingError> errors;

	LocalEventEngine* engine;
	DeviceEventParser* deviceParser;

};


} //Engine
} //STI

#endif
