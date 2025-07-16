#ifndef STI_ENGINE_RAWEVENT_H
#define STI_ENGINE_RAWEVENT_H

#include <sti/fwd/RawEvent_fwd.h>

#include <sti/device/DeviceID.h>
#include <sti/engine/ParsedVar.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/CompressedStackTrace.h>
#include <sti/utils/MixedValue.h>
#include <sti/utils/GraphPathLabel.h>
#include <sti/utils/VirtualFileServer.h>

#include <string>
#include <map>
#include <memory>


namespace STI
{
namespace Engine
{

class RawEventGroup;
class StackTraceData;
class StackTrace;


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
		unsigned eventNumber, const RawEventType& eventType, const CompressedStackTrace& eventStackTrace, 
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
	void refreshGroupName();
	void setParentGroup(const RawEventGroup* group);

	RawEventID getEventID() const;
	
	const RawEventType& type() const { return _eventType; }

	const CompressedStackTrace& getCompressedStackTrace() const { return stackTrace; }
	CompressedStackTrace& getCompressedStackTrace() { return stackTrace; }
	void setStackTrace(const CompressedStackTrace& eventStackTrace) { stackTrace = eventStackTrace; }
	void setStackTraceData(const std::shared_ptr<StackTraceData>& traceData) { stackTraceData = traceData; }

	StackTrace getStackTrace() const;

	const STI::Utils::GraphPathLabel& getEventGraphPath() const { return eventGraphPath; }

	bool isMeasurementEvent() const { return isMeasurement; }

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
	void setGroupName(const std::string& group) { _groupName = group; }

	void attachFileServer(const std::shared_ptr<STI::Utils::VirtualFileServer>& server);
	bool getFileServer(std::shared_ptr<STI::Utils::VirtualFileServer>& server) const;

	std::string print() const;

	template<class Archive>
	void serialize(Archive& archive);

private:
	
	double _time;
	RawEventTarget _target;		//target device and channel
	ParsedVar parsedValue;
	
	std::string _description;
	std::string _groupName;	//default group name; used if parentGroup is not set
	
	CompressedStackTrace stackTrace;
	std::shared_ptr<StackTraceData> stackTraceData;

	std::shared_ptr<STI::Utils::VirtualFileServer> fileServer;	//for measurement events that attach files
	
	bool isMeasurement;
	RawEventType _eventType;
	const RawEventGroup* parentGroup;

	STI::Utils::GraphPathLabel eventGraphPath;	//ordered list of event numbers; records the path leading to this event

};


} //Engine
} //STI

#endif
