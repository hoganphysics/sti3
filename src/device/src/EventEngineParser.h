#ifndef STI_ENGINE_EVENTENGINEPARSER_H
#define STI_ENGINE_EVENTENGINEPARSER_H

#include <sti/fwd/RawEvent_fwd.h>
#include <sti/fwd/Measurement_fwd.h>
#include <sti/fwd/SynchronousEvent_fwd.h>
#include <sti/device/DeviceID.h>
#include "fwd/ChannelManager_fwd.h"
#include <sti/engine/DeviceEventParser.h>
#include <sti/utils/GraphPathLabel.h>
#include <sti/engine/EngineID.h>

#include <string>
#include <sstream>
#include <set>


namespace STI
{
namespace Engine
{

class LocalEventEngine;
class EngineParsingMessage;


class EventEngineParser
{
public:


	EventEngineParser(const EngineID& engineID, const STI::Device::DeviceID& localDeviceID, 
					  const std::shared_ptr<STI::Device::ChannelManager>& channelManager, 
					  DeviceEventParser* deviceParser);
	~EventEngineParser();

	bool parse(const STI::Engine::RawEventGroup& eventGroup, SynchronousEventVector& synchedEvents);
	void getEventTargets(std::set<STI::Device::DeviceID>& targetIDs);
	void clear();

	RawEventMap rawEvents;
	DeviceEventMap partnerEvents;

	EngineParsingMessage& addParsingError(const std::string& name);

	const std::vector<EngineParsingMessage>& getParsingMessages() const;

private:

	bool addRawEvent(const RawEvent& rawEvent, unsigned& errorCount, unsigned maxErrors);
	bool addEventGroup(const RawEventGroup& eventGroup, unsigned& errorCount, unsigned maxErrors, bool& success);
	bool groupEventsByTime(const RawEventGroup& eventGroup);
	bool parseEvents(SynchronousEventVector& synchedEvents);
	bool checkMeasurements(SynchronousEventVector& synchedEvents);

	struct MeasurementCounter
	{
		MeasurementCounter(const RawEvent* rawEvent) 
			: rawEvent(rawEvent), count(0) {}

		const RawEvent* rawEvent;
		unsigned count;		//number of SynchronousEvents that reference this rawEvent
	};

	// std::map<STI::Utils::GraphPathLabel, MeasurementCounter> measurementEventGraph;
	std::map<RawEventID, MeasurementCounter> measurementEventGraph;

	bool countMeasurementRefs(const std::vector<std::shared_ptr<Measurement>>& measurements);
	bool maxErrorCheck(unsigned errorCount, unsigned maxErrors);

	void defineErrorIDs();

	std::vector<EngineParsingMessage> messages;
	bool hasErrors;

	DeviceEventParser* deviceParser;

	EngineID engineID;
	STI::Device::DeviceID localDeviceID;
	std::shared_ptr<STI::Device::ChannelManager> channelManager;

	std::map<std::string, unsigned> errorIDs;
};


} //Engine
} //STI

#endif
