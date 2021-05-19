/*! \file RawEvent.h
 *  \author Jason Michael Hogan
 *  \brief Include-file for the class RawEvent
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

#ifndef STI_ENGINE_RAWEVENT_H
#define STI_ENGINE_RAWEVENT_H

#include "fwd/RawEvent_fwd.h"

#include "MixedValue.h"
#include "EventStackTrace.h"
#include "DeviceID.h"
//#include "fwd/SynchronousEvent_fwd.h"
#include "utils/GraphPathLabel.h"

#include <string>
#include <map>
#include <memory>


namespace STI
{
namespace Engine
{

class SynchronousEvent;		//for confirming measurement scheduling


class RawEvent
{
public:

	//RawEvent(const STI::Device::DeviceID& targetDeviceID,
	//	double time, unsigned short channel, const STI::Utils::MixedValue& value,
	//	const std::string& description, unsigned eventNumber, bool isMeasurementEvent);
	
	RawEvent() {}

	RawEvent(const STI::Device::DeviceID& targetDeviceID,
		double time, unsigned short channel, const STI::Utils::MixedValue& value,
		const std::string& description, unsigned eventNumber, const RawEventType& eventType);


	//For device generated events
	RawEvent(const RawEvent& newEvent, const RawEvent& referenceEvent, unsigned eventNumber);
	
	~RawEvent();

	std::string print() const;

	double time() const;		//time in nanoseconds
	unsigned short channel() const;
	const STI::Utils::MixedValue& value() const;
	std::string description() const { return _description; }

	//struct Command
	//{
	//	enum class CommandType { Play, Pause, Waveform, Jump };	//...
	//	CommandType type;
	//};
	//const Command& command() const;
	
	//enum class EventType { Output, Measurement, Pause, Waveform, Jump };	//...
	const RawEventType& type() const { return _eventType; }

	const STI::Device::DeviceID& targetDevice() const;

	const EventStackTrace& getStackTrace() const { return trace; }

	const std::vector<unsigned>& getEventGraphPath() const { return eventGraphPath; }

	bool isMeasurementEvent() const { return isMeasurement; }
	//bool isScheduled() const { return _isScheduled; }

	bool operator<(const RawEvent& rhs) const { 
		return (time() < rhs.time()) 
					|| ((time() == rhs.time()) && (channel() < rhs.channel())); }

	bool operator==(const RawEvent& rhs) const { return eventGraphPath == rhs.eventGraphPath; }
	bool operator!=(const RawEvent& rhs) const { return !((*this) == rhs); }


	void setTargetID(const STI::Device::DeviceID& targetID) { targetDeviceID = targetID; }
	void setTime(double time) { _time = time; }
	void setChannel(unsigned short channel) { _channel = channel; }
	void setValue(const STI::Utils::MixedValue& value) { _value = value; }
	void setDescription(const std::string& description) { _description = description; }
	void setEventGraphPath(const STI::Utils::GraphPathLabel& pathLabel) { eventGraphPath = pathLabel;}
	void setEventType(const RawEventType& eventType) 
	{ 
		isMeasurement = (eventType == RawEventType::Measurement);
		_eventType = eventType;
	}

private:
	
	double _time;
	unsigned short _channel;
	STI::Utils::MixedValue _value;
	std::string _description;
	EventStackTrace trace;
	bool isMeasurement;
	RawEventType _eventType;
	STI::Device::DeviceID targetDeviceID;

	STI::Utils::GraphPathLabel eventGraphPath;	//ordered list of event numbers; records the path leading to this event

	//Somewhat of a hack here. Allowing friend access so SynchronousEvent
	//can confirm that this RawEvent has been scheduled when it is added as a Measurement.
	//Keeping private so only SynchronousEvent can access.
	//friend STI::Engine::SynchronousEvent;	
	//void setScheduled() { _isScheduled = true; }
	//bool _isScheduled;		//tracks if the RawEvent was associated with a SynchronousEvent during parse

};


} //Engine
} //STI

#endif
