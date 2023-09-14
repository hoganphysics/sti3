#include "EventEngineParser.h"

#include <sti/device/Channel.h>
#include <sti/device/DeviceID.h>

#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/SynchronousEvent.h>

#include <sti/utils/MixedValue.h>
#include <sti/utils/utils.h>

#include <sti/engine/EventConflictException.h>
#include <sti/engine/EventParsingException.h>
#include "LocalEventEngine.h"
#include <sti/engine/RawEventGroup.h>

#include <sti/utils/VirtualFileServer.h>

#include <set>

using STI::Device::Channel;
using STI::Engine::DeviceEventParser;
using STI::Engine::EventEngineParser;
using STI::Engine::LocalEventEngine;
using STI::Engine::RawEvent;
using STI::Engine::RawEventType;
using STI::Engine::RawEventGroup;
using STI::Engine::Measurement;
using STI::Engine::SynchronousEventVector;
using STI::Engine::EngineParsingMessage;
using STI::Utils::MixedValueType;
using STI::Utils::MixedValue;



EventEngineParser::EventEngineParser(const EngineID& engineID, const STI::Device::DeviceID& localDeviceID, 
					const std::shared_ptr<STI::Device::ChannelManager>& channelManager, 
					DeviceEventParser* deviceParser)
: engineID(engineID), localDeviceID(localDeviceID), channelManager(channelManager), deviceParser(deviceParser)
{
	defineErrorIDs();
	hasErrors = false;
	clear();
}

EventEngineParser::~EventEngineParser()
{
}

void EventEngineParser::clear()
{
	hasErrors = false;

	messages.clear();
	rawEvents.clear();
	partnerEvents.clear();
	measurementEventGraph.clear();

	fileServer = std::make_shared<STI::Utils::VirtualFileServer>();	//make new instead of clear, in case Measurement from previous shot have a reference.
}

const std::vector<EngineParsingMessage>& EventEngineParser::getParsingMessages() const
{
	return messages;
}

bool EventEngineParser::parse(const RawEventGroup& eventGroup, SynchronousEventVector& synchedEvents)
{
	bool success = true;

	clear();

	if (channelManager == 0) return false;

	success = groupEventsByTime(eventGroup);

	if (success) {
		//All events were added successfully.  
		//Now check for device-specific conflicts and errors while parsing.

		success = parseEvents(synchedEvents);
	}

	if (success) {
		//sort in time ascending order
		std::sort(synchedEvents.begin(), synchedEvents.end(), STI::Utils::compare_shared_ptr<SynchronousEvent>);
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

bool EventEngineParser::groupEventsByTime(const RawEventGroup& eventGroup)
{
	// //reset
	// rawEvents.clear();
	// partnerEvents.clear();
	// measurementEventGraph.clear();

	bool success = true;

	unsigned errorCount = 0;	//limit the number of errors that are reported back during a single parse attempt
	unsigned maxErrors = 10;

	addEventGroup(eventGroup, errorCount, maxErrors, success);

	return success;
}

bool EventEngineParser::addEventGroup(const RawEventGroup& eventGroup, unsigned& errorCount, unsigned maxErrors, bool& success)
{
	std::shared_ptr<RawEventVector> events;
	events = eventGroup.getEvents();

	// auto groupName = parentGroupName + "/" + eventGroup.getName();

	if (events != 0) {
		for (auto& evt : *events) {

			evt.setParentGroup(&eventGroup);
			success &= addRawEvent(evt, errorCount, maxErrors);

			if (maxErrorCheck(errorCount, maxErrors)) {
				success = false;
				return false;
			}
		}
	}

	for (auto& g : eventGroup.getSubgroups()) {
		if (g != 0) {
			
			if(!addEventGroup(*g, errorCount, maxErrors, success)) {
				return false;
			}
		}
	}

	return true;
}

EngineParsingMessage& EventEngineParser::addParsingError(const std::string& name)
{
	unsigned id = 0;	//default ID
	auto it = errorIDs.find(name);

	if (it != errorIDs.end()) {
		id = it->second;
	}

    messages.emplace_back(localDeviceID, ParsingMessageType::Error, id, name);
	
	hasErrors = true;
	
	return messages.back();
}

bool EventEngineParser::addRawEvent(RawEvent& rawEvent, unsigned& errorCount, unsigned maxErrors)
{
	bool success = true;

	//check that newest event's channel is defined
	std::shared_ptr<Channel> channel;

	//check that newest event's channel is defined and that the value type is correct
	if (!channelManager->getChannel(rawEvent.channel(), channel)) {
		//Missing channel
		success = false;
		errorCount++;

		//Error: Channel #24 is not defined on this device. Event trace:
		addParsingError("Missing Channel").addEvent(rawEvent)
			<< "Channel #" << rawEvent.channel()
			<< " is not defined on this device.";

	}
	else if (rawEvent.value().getType() != channel->getOutputType()) {
		//Wrong output type
		success = false;
		errorCount++;

		//Error: Incorrect type found for event on channel #5. Expected type 'Number'. Event trace:
		addParsingError("Incorrect Output Type").addEvent(rawEvent)
			<< "Incorrect output type found for event on channel #" << rawEvent.channel()
			<< ". Expected type '"
			<< MixedValue::TypeToString(channel->getOutputType()) << "' but received type ' " 
			<< MixedValue::TypeToString(rawEvent.value().getType()) << "'.";
	}

	if (!success || channel == 0)
		return false;

	switch (rawEvent.type())
	{
	case RawEventType::Play:
		if(channel->getType() != STI::Device::ChannelType::Output) {
			//Play called on a non-output channel
			success = false;
			errorCount++;

			//Error: Incorrect type found for event on channel #5. Expected type 'Number'. Event trace:
			addParsingError("Illegal Output Event").addEvent(rawEvent)
				<< "Output event requested on channel #" << rawEvent.channel()
				<< ", but this is an input channel.";
		}
		break;
	case RawEventType::Measurement:
		if(channel->getType() != STI::Device::ChannelType::Input) {
			//Measurement called on a non-input channel
			success = false;
			errorCount++;

			//Error: Incorrect type found for event on channel #5. Expected type 'Number'. Event trace:
			addParsingError("Illegal Input Event").addEvent(rawEvent)
				<< "Input measurement requested on channel #" << rawEvent.channel()
				<< ", but this is an output channel.";
		}
		break;
	case RawEventType::Waveform:
		break;
	case RawEventType::Pause:
		break;
	case RawEventType::Jump:
		break;
	default:
		break;
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

		//Entries in map: {GraphPathLabel, MeasurementCounter}
		measurementEventGraph.insert(
		{ rawEvent.getEventID(), MeasurementCounter( &(rawEvents[eventTime].back()) ) }
		);

		rawEvent.attachFileServer(fileServer);
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
//	deviceParser->clearEventNumber();						//Each device generated event gets a unique number appended to the graph label
//	deviceParser->setPartnerEventTarget(&partnerEvents);	//Set partner event target to point to this engine.

	do {
		success = true;	//Each time through the loop any offending events 
						//are removed before trying again. This way all events
						//can generate errors messages before returning.
		try {
			deviceParser->parseEvents(rawEvents, synchedEvents, localDeviceID, engineID, &partnerEvents);	//delegates to parseDeviceEvents (user code)
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

	auto& deviceParsingMessages = deviceParser->getParsingMessages();
	messages.insert(messages.begin(), deviceParsingMessages.begin(), deviceParsingMessages.end());

	return success;
}

bool EventEngineParser::countMeasurementRefs(const std::vector<std::shared_ptr<Measurement>>& measurements)
{
	//Look for each Measurement's source RawEvent in measurementEventGraph (via common graph path identifier).
	//If found, remove from measurementEventGraph (remaining events are not scheduled).
	for (auto& m : measurements) {

		if (m == 0) {
			addParsingError("Null Measurement")
				<< "Found null Measurement in the list of measurements! "
				<< "This occured in countMeasurementRefs(...). "
				<< "This is likely an error in the STI library.";
			return false;
		}

		auto it = measurementEventGraph.find(m->getEventID());

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
	errorIDs["Incorrect Output Type"]  				= 31;

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

	errorIDs["Illegal Output Event"] 				= 42;
	errorIDs["Illegal Input Event"] 				= 43;

	errorIDs["Null Measurement"] 					= 44;	

}

