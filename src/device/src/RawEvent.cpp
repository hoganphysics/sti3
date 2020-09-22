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

#include "RawEvent.h"
#include "DeviceID.h"
#include "MixedValue.h"
#include "utils.h"

#include <sstream>

using STI::Engine::RawEvent;
using STI::Engine::RawEventType;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;

RawEvent::RawEvent(const STI::Device::DeviceID& targetDeviceID, 
	double time, unsigned short channel, const MixedValue& value, 
	const std::string& description, unsigned eventNumber, const RawEventType& eventType)
	:
	_time(time), _channel(channel), _description(description), 
	_eventType(eventType), targetDeviceID(targetDeviceID)//, _isScheduled(false)
{
	isMeasurement = (eventType == RawEventType::Measurement);
	eventGraphPath.push_back(eventNumber);
}

RawEvent::RawEvent(const RawEvent& newEvent, const RawEvent& referenceEvent, unsigned eventNumber)
	: _time(newEvent._time), _channel(newEvent._channel), _description(newEvent._description), 
	_eventType(newEvent._eventType),
	isMeasurement(newEvent.isMeasurement), targetDeviceID(newEvent.targetDeviceID)//, _isScheduled(false)
{
	//Creates a new RawEvent based on the data stored in newEvent and the eventGraphPath
	//of the referenceEvent.  This is used for device-generated events to track their source.

	//The path should consist of the existing path to the referenceEvent, plus the new
	//eventNumber appended.
	eventGraphPath = referenceEvent.eventGraphPath;
	eventGraphPath.push_back(eventNumber);

	_value = std::move(newEvent._value);
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
	return _channel;
}

const MixedValue& RawEvent::value() const
{
	return _value;
}

const STI::Device::DeviceID& RawEvent::targetDevice() const
{
	return targetDeviceID;
}
