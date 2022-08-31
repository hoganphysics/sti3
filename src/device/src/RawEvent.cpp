/*! \file RawEvent.cpp
 *  \author Jason Michael Hogan
 *  \brief Source-file for the class RawEvent
 *  \section license License
 *
 *  Copyright (C) 2008 Jason Hogan <hogan@stanford.edu>\n
 *  This file is part of the Stanford Timing Interface (STI).
 *
 *  The STI is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The STI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the STI.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sti/engine/RawEvent.h>
#include <sti/device/DeviceID.h>
#include <sti/utils/MixedValue.h>
#include <sti/utils/utils.h>

// #include "RawEventGroup.h"

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>

#include "StackTraceData.h"
#include "RawStackTrace.h"


#include <sstream>

using STI::Engine::RawEvent;
using STI::Engine::RawEventType;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;
using STI::Engine::RawEventTarget;
using STI::Engine::RawEventID;
using STI::Engine::StackTraceData;
using STI::Engine::RawStackTrace;

RawEvent::RawEvent()
: _target("", "")
{
}

// RawEvent::RawEvent(const STI::Device::DeviceID& targetDeviceID, 
// 	double time, unsigned short channel, const MixedValue& value, 
// 	const StackTrace& eventStackTrace, unsigned eventNumber, const RawEventType& eventType)

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
	fullGroupName = referenceEvent.getGroupName();

	// setGroupName(referenceEvent.groupName());
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
	return fullGroupName;
}

void RawEvent::setGroupName(const std::string& name)
{
	fullGroupName = name;
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

// STI::Device::DeviceID RawEvent::targetDevice() const
// {
// 	return _target.device().deviceID();
// }

// const STI::Utils::GraphPathLabel& RawEvent::groupIndex()
// {
// 	return eventGroupIndex;
// }

// void RawEvent::setGroupIndex(const STI::Utils::GraphPathLabel& index)
// {
// 	eventGroupIndex = index;
// }

template<class Archive>
void RawEvent::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("time", _time), 
		cereal::make_nvp("target", _target), 
		// cereal::make_nvp("channel", _channel), 
		// cereal::make_nvp("targetDeviceID", targetDeviceID), 
		cereal::make_nvp("parsedValue", parsedValue),
		cereal::make_nvp("description", _description),
		cereal::make_nvp("eventType", _eventType),
		cereal::make_nvp("stackTrace", stackTrace),
		cereal::make_nvp("eventGraphPath", eventGraphPath), 
		cereal::make_nvp("fullGroupName", fullGroupName), 
		cereal::make_nvp("isMeasurement", isMeasurement),
		cereal::make_nvp("stackTraceData", stackTraceData)
		// cereal::make_nvp("eventGroupIndex", eventGroupIndex)
		);
}


template void RawEvent::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void RawEvent::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );

