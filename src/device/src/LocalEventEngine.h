#ifndef STI_ENGINE_LOCALEVENTENGINE_H
#define STI_ENGINE_LOCALEVENTENGINE_H

#include "EventEngine.h"
#include "MessageGenerator.h"

#include "DeviceMessage.h"
#include "DeviceCollection.h"
#include "EventEngineParser.h"
#include "EventEngineStateMachine.h"
#include "ParseID.h"
#include "ResultTicket.h"
#include "ShotID.h"
#include "TimeStamp.h"

#include "utils/OrderedBufferMap.h"
#include "ChannelManager.h"

#include "fwd/Channel_fwd.h"
#include "fwd/DeviceEventParser_fwd.h"
#include "fwd/DeviceID_fwd.h"
#include "fwd/Measurement_fwd.h"
#include "fwd/RawEvent_fwd.h"
#include "fwd/SynchronousEvent_fwd.h"

#include <memory>
#include <mutex>
#include <condition_variable>

namespace STI
{
namespace Engine
{

class EventEngine;
class EventEngineJob;
class EventEngineDependencyTree;
class DeviceMessageDispatcher;
class MasterTrigger;
class TriggerCallback;


class EventTime
{
	EventTime(double time);
};


class LocalEventEngine : public EventEngine, public STI::Device::MessageGenerator
{
public:

	LocalEventEngine(
		const EngineID& engineID,
		const STI::Device::DeviceID& localID, 
		const std::shared_ptr<STI::Device::ChannelManager>& channels,
		DeviceEventParser* deviceParser,
		const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher,
		const std::shared_ptr<STI::Device::DeviceCollection>& collection);
	virtual ~LocalEventEngine();

	//Could pass in a DocumentationTarget that the engine (attempts) to use to save data.  Falls back on its local DocTarget.
	//DocTarget would be passed from the instigating server, and would save to disk.
	//For all devices, DocTarget would provide persistent access to any data it stored.
	//--in the case of devices using their local DocTarget, only events that failed to be pulled would be available
	//If DocTarget pulls, it is now the owner of the shot's data and is reponsible for providing persistence.
	
	void clear();
	void parse(STI::Engine::EventEngineJob& job);

	void play(STI::Engine::EventEngineJob& job);
	void play(const EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug = false);	//ticket is a callback that will fire when results are ready;  playCB could push event number

	void trigger();
	void trigger(const STI::Device::DeviceID& target);		//triggers just target

	void stop();
	void pause();
	void unpause(bool retrigger);		//if retrigger, require a trigger before resuming (allows hard time resume)

	//debugging; separate class?
//	void addBreakpoint(time);
//	void clearBreakpoints();
//	void getBreakpoints(ids);
//	id getBreakpointAt(time);
//	void removeBreakpoint(id);
//	void getResults();

//	Debug: inspect state:  return current values of all out channels, and what RawEvent was last played, what time it thinks it is...

	void handleParseMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& message);
	void handlePlayReadyMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& message);
	void handlePlayCompleteMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& message);

	bool isState(EngineState target) const;
	STI::Engine::EngineState getState() const;

	//Local device details
	STI::Device::DeviceID getDeviceID() const { return localDeviceID; }
	
	//STI::Device::ChannelMap& localChannels;
	std::shared_ptr<STI::Device::ChannelManager> getLocalChannels() { return localChannels; }

	const STI::Engine::ParseID& getLastParseID() const { return lastParseID; }

	bool jobCancelled() const { return cancelled; }

	const DeviceEventMap& getParsedEvents();

	std::shared_ptr<EventEngineDependencyTree> getParsedTree() const { return dependencyTree; }

	DeviceEventParser* getDeviceParser() { return deviceParser; }

private:

	void parseDevice(const STI::Device::DeviceID& id, STI::Engine::EventEngineJob& job);

	bool isTargetServerForDevice(const STI::Device::DeviceID& id);
	void getOwnedDeviceIDs(std::set<STI::Device::DeviceID>& ownedIDs);
	void divideEvents(const RawEventVector& events);
	void mergePartnerEvents(const DeviceEventMap& events);

	bool setState(EngineState target);
	bool setState(EngineState target, EngineState fallback);

	void scheduleAllPlayJobs(const EngineJobID& jobID, const STI::Device::DeviceID& jobOwner);
	void playAll(const EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug);

	void playShot(TriggerCallback& triggerCB);

	void resetPlayThread();
	void waitForPlayComplete(std::unique_lock<std::mutex>& playLock);

	void waitForPlayAll();

	bool armTrigger(TriggerCallback& triggerCB);
	void waitForTrigger() const;
	void triggerOwnedDevices();

	bool playDeviceEvents();
	void measureData();
	void stopDeviceEvents();

	void stopOwnedDevices();
	void pauseOwnedDevices();
	void unpauseOwnedDevices(bool retrigger);

	bool waitUntil(double time);
	TimeStamp getCurrentTimeStamp();

	bool allEngineStateCheck(const std::map<STI::Device::DeviceID, STI::Engine::EngineState>& engineStates, const STI::Engine::EngineState& state);



	std::shared_ptr<MasterTrigger> masterTrigger;
	std::shared_ptr<TriggerCallback> masterTriggerCB;

	
	//Clock time;

	EngineID engineID;
	STI::Device::DeviceID localDeviceID;	//The DeviceID of the host of this engine
	STI::Engine::ParseID lastParseID;		//The ID of the most recently parsed shot

	EventEngineStateMachine stateMachine;
	
	EventEngineParser parser;
	DeviceEventParser* deviceParser;

	std::shared_ptr<STI::Device::DeviceCollection> deviceCollection;
	
	bool cancelled;

	std::shared_ptr<STI::Device::ChannelManager> localChannels;

	//server events
	DeviceEventMap eventsByTarget;
	//std::map<STI::Device::DeviceID, RawEventVector> eventsByTarget;
	RawEventVector upstreamEvents;
	std::vector<STI::Engine::RawEvent> handledPartnerEvents;
	std::shared_ptr<EventEngineDependencyTree> dependencyTree;
	std::shared_ptr<EventEngineDependencyTree> localSubtree;

	std::vector<EngineParsingMessage> localParsingMessages;

	bool isJobOwner;
	std::vector<STI::Device::DeviceID> ownedTargets;
	std::map<STI::Device::DeviceID, STI::Engine::EngineState> parsedOwnedTargets;
	std::map<STI::Device::DeviceID, STI::Engine::EngineState> playReadyOwnedTargets;
	std::map<STI::Device::DeviceID, STI::Engine::EngineState> playedOwnedTargets;
	std::map<STI::Device::DeviceID, std::shared_ptr<EventEngine>> engines;

	std::thread playThread;

	//local events
	//RawEventMap& rawEvents;					//Originating from python timing file (STIpy library)
	SynchronousEventVector synchedEvents;	//Generated by device specific parseDeviceEvents()
	//DeviceEventMap& partnerEvents;

	//Each ShotID refers to a single shot's measurement vector.
	STI::Utils::OrderedBufferMap<ShotID, std::shared_ptr<MeasurementVector>> measurementBuffer;

	void releaseParseLock();
	void releasePlayLock();
	void releaseTriggerLock();

	//trigger
	mutable std::mutex triggerMutex;
	mutable std::condition_variable triggerCondition;

	//parser
	mutable std::mutex parseMutex;
	mutable std::condition_variable parseCondition;

	//play
	mutable std::mutex playMutex;
	mutable std::condition_variable playCondition;

};

} //Engine
} //STI

#endif
