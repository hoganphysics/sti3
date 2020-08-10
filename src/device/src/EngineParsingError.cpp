
#include "EngineParsingError.h"
#include "RawEvent.h"
#include "EventConflictException.h"
#include "EventParsingException.h"
#include "DeviceID.h"

using STI::Engine::EventConflictException; 
using STI::Engine::EventParsingException;
using STI::Engine::EngineParsingError;
using STI::Engine::RawEvent;
using STI::Device::DeviceID;
using std::endl;

EngineParsingError::EngineParsingError(const DeviceID& deviceID)
	: deviceID(deviceID)
{
}

EngineParsingError::EngineParsingError(const DeviceID& deviceID, const EventConflictException& exception)
	: deviceID(deviceID)
{
	(*this)
		<< "Error: Event conflict. "
		<< exception.printMessage() << "\n";
	addEvent(exception.getEvent1());

	//Some EventConflictException can have only one event (?).  Conflict with some other constraint...
	if (exception.getEvent1() != exception.getEvent2()) {
		addEvent(exception.getEvent2());
	}
}

EngineParsingError::EngineParsingError(const DeviceID& deviceID, const EventParsingException& exception)
	: deviceID(deviceID)
{
	(*this)
		<< "Error: Event parsing error. "
		<< exception.printMessage() << "\n";
	addEvent(exception.getEvent());
}


EngineParsingError::~EngineParsingError()
{
}

void EngineParsingError::addEvent(const RawEvent& evt)
{
	events.push_back(evt);
}

std::string EngineParsingError::messageText() const
{
	return errMessage;
}


std::string EngineParsingError::print() const
{
	std::string indent = "       ";
	std::stringstream eventMessage;
	if (events.size() > 0) {
		eventMessage << indent << "Event trace: " << endl;
		for (auto& e : events) {
			eventMessage << indent << e.print() << endl;
		}
		eventMessage << indent << "Location: " << endl;
		for (auto& e : events) {
			eventMessage << indent << e.getStackTrace().print(indent) << endl;
		}
	}
	return errMessage + eventMessage.str();
}