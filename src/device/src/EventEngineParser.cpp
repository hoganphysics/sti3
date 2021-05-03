
#include "EventEngineParser.h"

#include "Channel.h"
#include "EventConflictException.h"
#include "EventParsingException.h"
#include "LocalEventEngine.h"
#include "RawEvent.h"
#include "SynchronousEvent.h"
#include "Measurement.h"
#include "utils.h"
#include "MixedValue.h"

// #include "EngineParsingError.h"
#include "EngineParsingMessage.h"

#include "DeviceID.h"

#include <set>


using STI::Engine::DeviceEventParser;
using STI::Engine::EventEngineParser;
using STI::Engine::LocalEventEngine;
using STI::Engine::RawEvent;
using STI::Engine::Measurement;
using STI::Engine::SynchronousEventVector;
using STI::Utils::MixedValueType;
using STI::Utils::MixedValue;
using std::endl;
using STI::Device::Channel;
using STI::Engine::EngineParsingMessage;

EventEngineParser::EventEngineParser(LocalEventEngine* engine, DeviceEventParser* deviceParser) 
	: engine(engine), deviceParser(deviceParser)
{
	defineErrorIDs();
	hasErrors = false;
}

EventEngineParser::~EventEngineParser()
{
}

const std::vector<EngineParsingMessage>& EventEngineParser::getParsingMessages() const
{
	return messages;
}

bool EventEngineParser::parse(const STI::Engine::RawEventVector& events, SynchronousEventVector& synchedEvents)
{
	bool success = true;

//	errors.clear();
	messages.clear();
	hasErrors = false;

	success = groupEventsByTime(events);

	if (success) {
		//All events were added successfully.  
		//Now check for device-specific conflicts and errors while parsing.

		success = parseEvents(synchedEvents);
	}
	
	if (success) {
		//sort in time ascending order
		std::sort(synchedEvents.begin(), synchedEvents.end(), STI::Utils::compare_unique_ptr<SynchronousEvent>);
	}
	
	if (success) {
		success = checkMeasurements(synchedEvents);		//make sure all meas() events have been registered
	}

	//Check for error messages
	for (auto& m : messages) {
		if (m.getType() == ParsingMessageType::Error) {
			hasErrors = true;
			break;
		}
	}

	return success && !hasErrors;
}

void EventEngineParser::getEventTargets(std::set<STI::Device::DeviceID>& targetIDs)
{
	deviceParser->getEventTargets(targetIDs);
}

bool EventEngineParser::groupEventsByTime(const STI::Engine::RawEventVector& events)
{
	//reset
	rawEvents.clear();
	partnerEvents.clear();
	measurementEventGraph.clear();

	bool success = true;

	unsigned errorCount = 0;	//limit the number of errors that are reported back during a single parse attempt
	unsigned maxErrors = 10;

	//This is only zero after resetting this device's parsed events.
	//	unsigned initialEventNumber = rawEvents.size();
	unsigned initialEventNumber = 0;

	//Move the events from TDeviceEventSeq 'events' (provided by server) to
	//the raw event list 'rawEvents'.  Check for general event errors.
	for(auto& evt : events)
	{
		success &= addRawEvent(evt, errorCount, maxErrors);

		if (maxErrorCheck(errorCount, maxErrors)) {
			return false;
		}
	}

	return success;
}

EngineParsingMessage& EventEngineParser::addParsingError(const std::string& name)
{
	unsigned id = 0;	//default ID
	auto it = errorIDs.find(name);

	if (it != errorIDs.end()) {
		id = it->second;
	}

    messages.emplace_back(engine->getDeviceID(), ParsingMessageType::Error, id, name);
	
	hasErrors = true;
	
	return messages.back();
}

bool EventEngineParser::addRawEvent(const RawEvent& rawEvent, unsigned& errorCount, unsigned maxErrors)
{
	bool success = true;

	//check that newest event's channel is defined
	auto channels = engine->getLocalChannels();
	
	std::shared_ptr<Channel> channel;

	//auto channel = engine->localChannels.find(rawEvent.channel());

	//check that newest event's channel is defined and that the value type is correct
//	if (channel == engine->localChannels.end()) {
	if (!channels->getChannel(rawEvent.channel(), channel)) {
		//Missing channel
		success = false;
		errorCount++;

		// struct EEParserError
		// {
		// 	EEParserError(unsigned id, const std::string& name) : id(id), name(name) {}
		// 	unsigned id;
		// 	std::string name;
		// };
		// enum class ErrorType { ErrorMissingChannel };
		// std::map<ErrorType,EEParserError> errorList;
		// errorList[ErrorType::ErrorMissingChannel] = EEParserError(30, "Missing Channel");

		//Error: Channel #24 is not defined on this device. Event trace:
		addParsingError("Missing Channel").addEvent(rawEvent)
			<< "Channel #" << rawEvent.channel()
			<< " is not defined on this device.";

	}
	else if (rawEvent.value().getType() != channel->getOutputType()) {
		//Wrong type
		success = false;
		errorCount++;

		//Error: Incorrect type found for event on channel #5. Expected type 'Number'. Event trace:
		addParsingError("Incorrect Type").addEvent(rawEvent)
			<< "Incorrect type found for event on channel #" << rawEvent.channel()
			<< ". Expected type '"
			<< MixedValue::TypeToString(channel->getOutputType()) << "' but received type ' " 
			<< MixedValue::TypeToString(rawEvent.value().getType()) << "'.";
	}

	if (!success)
		return false;

	//To do: Try std::move from input vector to map?
	//add event
	double eventTime = rawEvent.time();
	rawEvents[eventTime].push_back(rawEvent);		//consider storing events by int time, or Time class
	
	//Store pointers to all measurement RawEvents, indexed by their event graph identifier.  
	//This is for fast reverse lookup in checkMeasurements(...)
	if (rawEvent.isMeasurementEvent()) {
		//Entries in map: {GraphPathLabel, *RawEvent}
		//measurementEventGraph.insert({ rawEvent.getEventGraphPath(), &(rawEvents[eventTime].back()) });

		//Entries in map: {GraphPathLabel, MeasurementCounter}
		measurementEventGraph.insert(
		{ rawEvent.getEventGraphPath(), MeasurementCounter( &(rawEvents[eventTime].back()) ) }
		);
	}

	auto& eventVec = rawEvents[eventTime];

	//check for multiple events on the same channel at the same time
	for (unsigned j = 0; j < eventVec.size() - 1; ++j)
	{
		//Has the current event's channel already being set?
		if (rawEvent.channel() == eventVec.at(j).channel())
		{
			success = false;
			errorCount++;

			//Error: Multiple events scheduled on channel #24 ('Laser power') at time 2.56:
			addParsingError("Event Conflict").addEvent(rawEvent).addEvent(rawEvents[eventTime].at(j))
				<< "Multiple events scheduled on channel #" << rawEvent.channel()
				<< " ('" << channel->getChannelName() << "') "
				<< " at time " << STI::Utils::printTimeFormated(eventTime) << ".";
		}
		if (errorCount > maxErrors)
			break;
	}

	return success;
}

bool EventEngineParser::parseEvents(SynchronousEventVector& synchedEvents)
{
	RawEventMap::iterator badEvent = rawEvents.end();

	bool success;

	//limit the number of errors that are reported back during a single parse attempt
	unsigned errorCount = 0;
	unsigned maxErrors = 10;

	//Device generated event setup (events created by user code in parseDeviceEvents)
	deviceParser->clearEventNumber();						//Each device generated event gets a unique number appended to the graph label
	deviceParser->setPartnerEventTarget(&partnerEvents);	//Set partner event target to point to this engine.

	do {
		success = true;	//Each time through the loop any offending events 
						//are removed before trying again. This way all events
						//can generate errors messages before returning.
		try {
			deviceParser->parseEvents(rawEvents, synchedEvents);	//delegates to parseDeviceEvents (user code)
		}
		catch (EventConflictException& eventConflict)
		{
			errorCount++;
			success = false;
			//Error: Event conflict. <Device Specific Message>
			//       Event trace:
			addParsingError("Event Conflict Exception")
				.addEvent(eventConflict.getEvent1())
				.addEvent(eventConflict.getEvent2())
				<< "Event conflict. " << eventConflict.printMessage();

			//find the latest event associated with this exception
			badEvent = rawEvents.find(eventConflict.lastTime());
		}
		catch (EventParsingException& eventParsing)
		{
			errorCount++;
			success = false;
			//Error: Event parsing error. <Device Specific Message>
			//       Event trace:
			addParsingError("Event Parsing Exception")
				.addEvent(eventParsing.getEvent())
				<< eventParsing.printMessage();

			//find the event associated with this exception
			badEvent = rawEvents.find(eventParsing.getEvent().time());
		}
		catch (STI_Exception& exception)
		{
			errorCount++;
			success = false;

			addParsingError("Generic Parsing Exception")
				 << "Caught generic STI_Exception while parsing events in device code. "
				 << "Message: '"
				 << exception.printMessage() << "'.";

			return false;		//break the error loop immediately
		}
		catch (...)	//generic conflict or error
		{
			errorCount++;
			success = false;
			//Error: Event error or conflict detected. Debug info not available.
			addParsingError("Unhandled Parsing Exception")
				<< "Unhandled exception while parsing events in device. "
				<< "Debug info not available.";

			return false;		//break the error loop immediately
		}

		if (maxErrorCheck(errorCount, maxErrors)) {
			return false;
		}

		//Try to continue parsing the rest of the rawEvents list
		if (!success && badEvent != rawEvents.end()) {
			//remove all previous events from the map
			badEvent++;		//erase removes [first, last)
			rawEvents.erase(rawEvents.begin(), badEvent);
		}

	} while (!success && rawEvents.size() != 0);

	return success;
}

bool EventEngineParser::countMeasurementRefs(const std::vector<std::shared_ptr<Measurement>>& measurements)
{
	//Look for each Measurement's source RawEvent in measurementEventGraph (via common graph path identifier).
	//If found, remove from measurementEventGraph (remaining events are not scheduled).
	for (auto& m : measurements) {

		auto it = measurementEventGraph.find(m->getMeasurementGraphPath());

		if (it != measurementEventGraph.end()) {
			it->second.count++;		//Expect each RawEvent to be found once and only once
		}
		else {
			//Error: A Measurement points to an event that doesn't exist in the graph.
			//Check to see if it's a known RawEvent, but not a measurement event.
			addParsingError("Invalid Measurement Registration")
				<< "A scheduled Measurement points to a RawEvent that doesn't exist in the measurement event list. "
				<< "This could mean a RawEvent that is not a measurement was registered with a SynchronousEvent. "
				<< "This is an error in the device's implemented parse events function.";

			return false;
		}
	}
	return true;
}

bool EventEngineParser::checkMeasurements(SynchronousEventVector& synchedEvents)
{
	unsigned errorCount = 0;
	unsigned maxErrors = 10;

	for (auto& synchedEvt : synchedEvents) {

		//Each SynchronousEvent can have a vector of scheduled events associated with it
		auto& measurements = synchedEvt->getMeasurements();

		if (!countMeasurementRefs(measurements)) {
			errorCount++;
		}
		
		if (maxErrorCheck(errorCount, maxErrors)) {
			return false;
		}
	}

	//We want to verify a one-to-one relationship between measurement RawEvents and Measurements 
	//created when the user registers a RawEvent with a SynchronousEvent in parseDeviceEvents.
	//Associated RawEvents and Measurements share the same GraphPathLabel.
	//To solve the reverse lookup problem efficiently, we have previously constructed a map of 
	//GraphPathLabels to RawEvents pointers and an included a counter.  Constructing this just 
	//takes memory and a constant scale factor over the normal RawEventMap construction.
	//Next we can iterate O(n) through all measurements in the synchedEvents (loop below), and
	//increment the counters of all the GraphPathLabels we find.  Each search through the map costs
	//O(log(n)), so the total complexity to compute the counts is O(n log(n)).
	//Finally we iterate through the map and check for counts that are not one.

	//Check that all measurements are associated with one SynchronousEvent.
	//Each measurement RawEvent should be registered once and only once.
	for (auto& me : measurementEventGraph) {
		if (me.second.count == 0) {
			errorCount++;

			//Error: RawEvent was not registered by any SynchronousEvent
			addParsingError("Unregistered Measurement").addEvent( *(me.second.rawEvent) )
				<< "The following measurement event is not associated with a SynchronousEvent. "
				<< "This is an error in the device's implemented parse events function.";
		}
		else if (me.second.count > 1) {
			errorCount++;

			//Error: Multiple SynchronousEvents registered the same RawEvent
			addParsingError("Multiple Measurement Registrations").addEvent( *(me.second.rawEvent) )
				<< "The same measurement event has been registered with multiple SynchronousEvents "
				<< "(registration count = " << me.second.count << "). "
				<< "Each measurement RawEvent must be added to exactly one SynchronousEvent. "
				<< "This is an error in the device's implemented parse events function.";
		}

		if (maxErrorCheck(errorCount, maxErrors)) {
			return false;
		}
	}


	//Check that synchedEvents has only one entry for each time.  The following
	//assumes the synchedEvents vector is sorted by time.
	for (unsigned i = 0; i < synchedEvents.size(); i++) {

		if (i < synchedEvents.size() - 1
			&& synchedEvents.at(i)->getTime() == synchedEvents.at(i + 1)->getTime()) {
			errorCount++;

			addParsingError("Multiple SynchonousEvents")
				<< "Multiple SynchonousEvent are scheduled at time " 
				<< STI::Utils::printTimeFormated(synchedEvents.at(i)->getTime()) << ". "
				<< "Events that occur on multiple channels at the same time must be grouped "
				<< "into a single SynchonousEvent. "
				<< "Only one SynchonousEvent is allowed at any time. "
				<< "This is an error in the device's implemented parse events function.";
		}

		if (maxErrorCheck(errorCount, maxErrors)) {
			return false;
		}
	}

	return (errorCount == 0);
}

bool EventEngineParser::maxErrorCheck(unsigned errorCount, unsigned maxErrors)
{
	if (errorCount > maxErrors) {

		//Too many errors; stop parsing and tell the user that there may be more
		addParsingError("Max Error Count Reached")
			<< "Too many errors. Parsing aborted after " 
			<< errorCount << " errors.";

		return true;		//break the error loop immediately
	}
	return false;
}

void EventEngineParser::defineErrorIDs()
{
	errorIDs["Missing Channel"] 					= 30;
	errorIDs["Incorrect Type"]  					= 31;

	errorIDs["Event Conflict"]  					= 32;
	errorIDs["Event Conflict Exception"]  			= 33;

	errorIDs["Event Parsing Exception"]   			= 34;
	errorIDs["Generic Parsing Exception"] 			= 35;

	errorIDs["Unhandled Parsing Exception"] 		= 36;
	errorIDs["Invalid Measurement Registration"] 	= 37;

	errorIDs["Unregistered Measurement"] 			= 38;
	errorIDs["Multiple Measurement Registrations"] 	= 39;

	errorIDs["Multiple SynchonousEvents"] 			= 40;
	errorIDs["Max Error Count Reached"] 			= 41;

}