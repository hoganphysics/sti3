#include <sti/engine/RawEvent.h>

#include <sti/device/DeviceID.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/StackTraceData.h>
#include <sti/utils/MixedValue.h>
#include <sti/utils/utils.h>

#include "RawStackTrace.h"

#include <sstream>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>

using STI::Engine::RawEvent;
using STI::Engine::RawEventType;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventID;
using STI::Engine::StackTraceData;
using STI::Engine::RawStackTrace;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;


RawEvent::RawEvent()
: _target("", "")
{
}

RawEvent::RawEvent(const RawEventTarget& eventTarget, double time, const STI::Utils::MixedValue& value,
	unsigned eventNumber, const RawEventType& eventType)
: RawEvent(eventTarget, time, value, eventNumber, eventType, StackTrace(), 0)
{
}

RawEvent::RawEvent(const RawEventTarget& eventTarget, double time, const STI::Utils::MixedValue& value,
		unsigned eventNumber, const RawEventType& eventType, const StackTrace& eventStackTrace, 
		const std::shared_ptr<StackTraceData>& stackTraceData)
: _target(eventTarget), _time(time), _eventType(eventType), stackTrace(eventStackTrace), stackTraceData(stackTraceData)
//, _isScheduled(false)
{
	parsedValue.value = value;
	isMeasurement = (eventType == RawEventType::Measurement);
	eventGraphPath.push_back(eventNumber);

	// eventGroupIndex = group.getFullIndex();
}

RawEvent::RawEvent(const RawEvent& newEvent, const RawEvent& referenceEvent, unsigned eventNumber)
: _time(newEvent._time), _target(newEvent.target()), _description(newEvent._description), 
_eventType(newEvent._eventType), isMeasurement(newEvent.isMeasurement) //, _isScheduled(false)
{
	//Creates a new RawEvent based on the data stored in newEvent and the eventGraphPath
	//of the referenceEvent.  This is used for device-generated events to track their source.

	//The path should consist of the existing path to the referenceEvent, plus the new
	//eventNumber appended.
	eventGraphPath = referenceEvent.eventGraphPath;
	eventGraphPath.push_back(eventNumber);
	// fullGroupName = referenceEvent.getGroupName();

	setParentGroup(referenceEvent.parentGroup);
	stackTrace = referenceEvent.getStackTrace();
	stackTraceData = referenceEvent.stackTraceData;

	parsedValue = std::move(newEvent.parsedValue);

	// _value = std::move(newEvent._value);
}

RawEvent::~RawEvent()
{
}

std::string RawEvent::print() const
{
	std::stringstream evt;

	//<Time=2.1, Channel=4, Type=Number, Value=3.4>
	evt << "<Time=" << STI::Utils::printTimeFormated(time());
	evt << ", Channel=" << channel();
	evt << ", Type=";

	evt << MixedValue::TypeToString(value().getType());
	evt << ", Value=" << value().print() << ">";
	
	return evt.str();
}

double RawEvent::time() const
{
	return _time;
}

unsigned short RawEvent::channel() const
{
	return _target.channel().channel();
}

const MixedValue& RawEvent::value() const
{
	return parsedValue.value;
}

const RawEventTarget& RawEvent::target() const
{
	return _target;
}

RawEventTarget& RawEvent::getTarget()
{
	return _target;
}

std::string RawEvent::getGroupName() const
{
	if (parentGroup != 0) {
		return parentGroup->getFullName();
	}
	return "";
}

void RawEvent::setParentGroup(const RawEventGroup* group)
{
	parentGroup = group;
}

RawEventID RawEvent::getEventID() const
{
	RawEventID eventID;
	eventID.groupName = getGroupName();
	eventID.eventGraphPath = getEventGraphPath();

	return eventID;
}

RawStackTrace RawEvent::getRawStackTrace() const
{
	if (stackTraceData != 0) {
		return stackTraceData->getStackTrace( getStackTrace() );
	}
	
	RawStackTrace trace;
	return trace;	
}

void RawEvent::attachFileServer(const std::shared_ptr<STI::Utils::VirtualFileServer>& server)
{
	fileServer = server;
}

bool RawEvent::getFileServer(std::shared_ptr<STI::Utils::VirtualFileServer>& server) const
{
	server = fileServer;
	return server != 0;
}

template<class Archive>
void RawEvent::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("time", _time), 
		cereal::make_nvp("target", _target), 
		cereal::make_nvp("parsedValue", parsedValue),
		cereal::make_nvp("description", _description),
		cereal::make_nvp("eventType", _eventType),
		cereal::make_nvp("stackTrace", stackTrace),
		cereal::make_nvp("eventGraphPath", eventGraphPath), 
		// cereal::make_nvp("parentGroup", parentGroup), 
		cereal::make_nvp("isMeasurement", isMeasurement),
		cereal::make_nvp("stackTraceData", stackTraceData)
		);
}

template void RawEvent::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void RawEvent::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
