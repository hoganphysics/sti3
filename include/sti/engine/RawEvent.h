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

#include <sti/fwd/RawEvent_fwd.h>

#include <sti/device/DeviceID.h>

#include <sti/engine/ParsedVar.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/StackTrace.h>

#include <sti/utils/MixedValue.h>
#include <sti/utils/GraphPathLabel.h>

#include <string>
#include <map>
#include <memory>


namespace STI
{
namespace Engine
{

class SynchronousEvent;		//for confirming measurement scheduling
class RawEventGroup;
class StackTraceData;
class RawStackTrace;

struct RawEventID
{
	std::string groupName;
	STI::Utils::GraphPathLabel eventGraphPath;

	bool operator<(const RawEventID& rhs) const
	{
		if (groupName == rhs.groupName) {
			return eventGraphPath < rhs.eventGraphPath;
		}
		return groupName < rhs.groupName;
	}

	bool operator==(const RawEventID& rhs) const { return (groupName == rhs.groupName && eventGraphPath == rhs.eventGraphPath); }
	bool operator!=(const RawEventID& rhs) const { return !((*this) == rhs); }
};


class RawEvent
{
public:

	RawEvent();
	RawEvent(const RawEventTarget& eventTarget, double time, const STI::Utils::MixedValue& value,
		unsigned eventNumber, const RawEventType& eventType);
	RawEvent(const RawEventTarget& eventTarget, double time, const STI::Utils::MixedValue& value,
		unsigned eventNumber, const RawEventType& eventType, const StackTrace& eventStackTrace, 
		const std::shared_ptr<StackTraceData>& stackTraceData);

	//For device generated events
	RawEvent(const RawEvent& newEvent, const RawEvent& referenceEvent, unsigned eventNumber);
	
	~RawEvent();

	double time() const;		//time in nanoseconds
	unsigned short channel() const;
	const STI::Utils::MixedValue& value() const;
	std::string description() const { return _description; }

	const RawEventTarget& target() const;
	RawEventTarget& getTarget();

	std::string getGroupName() const;
	void setParentGroup(const RawEventGroup* group);

	RawEventID getEventID() const;
	
	const RawEventType& type() const { return _eventType; }

	const StackTrace& getStackTrace() const { return stackTrace; }
	StackTrace& getStackTrace() { return stackTrace; }
	void setStackTrace(const StackTrace& eventStackTrace) { stackTrace = eventStackTrace; }
	void setStackTraceData(const std::shared_ptr<StackTraceData>& traceData) { stackTraceData = traceData; }

	RawStackTrace getRawStackTrace() const;

	const STI::Utils::GraphPathLabel& getEventGraphPath() const { return eventGraphPath; }

	bool isMeasurementEvent() const { return isMeasurement; }
	//bool isScheduled() const { return _isScheduled; }

	bool operator<(const RawEvent& rhs) const { 
		return time() < rhs.time() ||
			( time() == rhs.time() && target() < rhs.target() );
	}

	bool operator==(const RawEvent& rhs) const { return eventGraphPath == rhs.eventGraphPath; }
	bool operator!=(const RawEvent& rhs) const { return !((*this) == rhs); }

	void setTime(double time) { _time = time; }
	void setTarget(const STI::Engine::RawEventTarget& target) { _target = target; }
	void setValue(const STI::Utils::MixedValue& value) { parsedValue.value = value; }
	void setDescription(const std::string& description) { _description = description; }
	void setEventGraphPath(const STI::Utils::GraphPathLabel& pathLabel) { eventGraphPath = pathLabel;}
	void setEventType(const RawEventType& eventType) 
	{ 
		isMeasurement = (eventType == RawEventType::Measurement);
		_eventType = eventType;
	}

	std::string print() const;

	template<class Archive>
	void serialize(Archive& archive);

private:
	
	double _time;
	RawEventTarget _target;		//target device and channel
	// STI::Utils::MixedValue _value;
	ParsedVar parsedValue;
	
	std::string _description;
	
	StackTrace stackTrace;
	std::shared_ptr<StackTraceData> stackTraceData;
	
	bool isMeasurement;
	RawEventType _eventType;
	// STI::Device::DeviceID targetDeviceID;
	// std::string fullGroupName;
	const RawEventGroup* parentGroup;

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
