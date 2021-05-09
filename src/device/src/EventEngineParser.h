#ifndef STI_ENGINE_EVENTENGINEPARSER_H
#define STI_ENGINE_EVENTENGINEPARSER_H

#include "fwd/RawEvent_fwd.h"
#include "fwd/Measurement_fwd.h"
#include "fwd/SynchronousEvent_fwd.h"
#include "DeviceID.h"
#include "fwd/ChannelManager_fwd.h"
#include "DeviceEventParser.h"
#include "utils/GraphPathLabel.h"
//#include "EngineParsingError.h"
#include "EngineID.h"

#include <string>
#include <sstream>
#include <set>

namespace STI
{
namespace Engine
{

//class EngineParsingError;
class LocalEventEngine;
class EngineParsingMessage;

class EventEngineParser
{
public:


	EventEngineParser(const EngineID& engineID, const STI::Device::DeviceID& localDeviceID, 
					  const std::shared_ptr<STI::Device::ChannelManager>& channelManager, 
					  DeviceEventParser* deviceParser);

	// EventEngineParser(LocalEventEngine* engine, DeviceEventParser* deviceParser);
	~EventEngineParser();

	bool parse(const STI::Engine::RawEventVector& events, SynchronousEventVector& synchedEvents);
	void getEventTargets(std::set<STI::Device::DeviceID>& targetIDs);
	void clear();

	RawEventMap rawEvents;
	DeviceEventMap partnerEvents;

	EngineParsingMessage& addParsingError(const std::string& name);

	const std::vector<EngineParsingMessage>& getParsingMessages() const;

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

	void defineErrorIDs();

//	std::vector<EngineParsingError> errors;
	std::vector<EngineParsingMessage> messages;
	bool hasErrors;

//	LocalEventEngine* engine;
	DeviceEventParser* deviceParser;

	EngineID engineID;
	STI::Device::DeviceID localDeviceID;
	std::shared_ptr<STI::Device::ChannelManager> channelManager;

	std::map<std::string, unsigned> errorIDs;
};


} //Engine
} //STI

#endif
