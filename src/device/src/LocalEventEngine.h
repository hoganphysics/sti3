#ifndef STI_ENGINE_LOCALEVENTENGINE_H
#define STI_ENGINE_LOCALEVENTENGINE_H

#include "EventEngine.h"

#include <sti/fwd/Channel_fwd.h>
#include <sti/fwd/DeviceID_fwd.h>
#include <sti/fwd/Measurement_fwd.h>
#include <sti/fwd/RawEvent_fwd.h>
#include <sti/fwd/SynchronousEvent_fwd.h>

#include <sti/device/ChannelManager.h>
#include <sti/device/DeviceCollection.h>
#include <sti/device/DeviceMessage.h>
#include <sti/device/PersistenceManager.h>

#include <sti/engine/ParseID.h>
#include <sti/engine/ResultTicket.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/EnginePlayingMessage.h>
#include <sti/utils/TimeStamp.h>

#include "fwd/DeviceEventParser_fwd.h"
#include "DeviceMessageGrouper.h"
#include "EngineClock.h"
#include "EventEngineParser.h"
#include "EventEngineStateMachine.h"
#include "MessageGenerator.h"
#include "utils/OrderedBufferMap.h"

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
class ResultsCollector;
class ShotResult;
class RawEventGroup;
class FullShotResult;
class EngineTriggerTarget;

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
		const std::shared_ptr<STI::Device::AttributeManager>& attributeManager,
		DeviceEventParser* deviceParser,
		EngineTriggerTarget* triggerTarget,
		const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher,
		const std::shared_ptr<STI::Device::DeviceCollection>& collection,
		const std::shared_ptr<STI::Device::PersistenceManager>& persistence);
	virtual ~LocalEventEngine();

	void clear();
	void unload();
	void parse(STI::Engine::EventEngineJob& job);	//include results callback in job?

	void play(STI::Engine::EventEngineJob& job);
	void play(const EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug = false);	//ticket is a callback that will fire when results are ready;  playCB could push event number

	void trigger();
	void trigger(const STI::Device::DeviceID& target);		//triggers just target

	void stop();
	void pause();
	void unpause(bool retrigger);		//if retrigger, require a trigger before resuming (allows hard time resume)


	//debugging;
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
	
	std::shared_ptr<STI::Device::ChannelManager> getLocalChannels() { return localChannels; }

	const STI::Engine::ParseID& getLastParseID() const;
	bool jobCancelled() const { return cancelled; }

	bool getParseResult(const ParseID& parseID, std::shared_ptr<ParseResult>& parseResult) const;
	bool getShotResult(const ShotID& shotID, std::shared_ptr<ShotResult>& shotResult) const;
	std::shared_ptr<ParsedDependencyTree> getParsedTree() const;

	DeviceEventParser* getDeviceParser() { return deviceParser; }
	EngineTriggerTarget* getTriggerTarget() { return triggerTarget; }

private:

	void parseDevice(const STI::Device::DeviceID& id, STI::Engine::EventEngineJob& job);
	void cancelParse(const EngineJobID& jobID);
	void cancelParseJob(STI::Engine::EventEngineJob& job);
	void cancelPlayJob();
	bool isPlayCancelled() const;

	bool isTargetServerForDevice(const STI::Device::DeviceID& id);
	bool isActingServerForDevice(const STI::Device::DeviceID& id);
	void getOwnedDeviceIDs(std::set<STI::Device::DeviceID>& ownedIDs);

	void divideEvents(const std::shared_ptr<RawEventGroup>& events, std::shared_ptr<RawEventGroup>& unhandledEventGroup);
	void divideEvents(const std::shared_ptr<RawEventGroup>& events, std::shared_ptr<RawEventGroup>& unhandledEventGroup, std::shared_ptr<RawEventGroup>& handledEventGroup);
	void divideEvents(const std::shared_ptr<RawEventGroup>& eventGroup, 
		const std::string& subgroupName, 
		const std::set<STI::Device::DeviceID>& ownedIDs, 
		std::shared_ptr<RawEventGroup>& unhandledEventGroup, 
		std::shared_ptr<RawEventGroup>& handledEventGroup);

	void addEvent(const RawEvent& evt, const std::string& subgroupName, const std::set<STI::Device::DeviceID>& ownedIDs, std::shared_ptr<RawEventGroup>& unhandledEventGroup, std::shared_ptr<RawEventGroup>& handledEventGroup);

	RawEventGroup& getTargetEventGroup(const STI::Device::DeviceID& deviceTarget);
	RawEventGroup& getAbstractTargetEventGroup(const RawEventTargetDevice& deviceTarget);

	void mergePartnerEvents(const DeviceEventMap& events);

	void addEventsToParseResult(const std::shared_ptr<RawEventGroup>& newEvents);

	void updateChannelValues(const RawEventVector& rawEvents);

	void scheduleAllPlayJobs(const EngineJobID& jobID, const std::shared_ptr<Shot>& shot, const STI::Device::DeviceID& jobOwner);

	void playAll(const EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug);
	void playShot(TriggerCallback& triggerCB);

	void resetPlayThread();
	void waitForPlayComplete();
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

	bool waitUntil(std::unique_lock<std::mutex>& lock, double time);
	STI::Utils::TimeStamp getCurrentTimeStamp();

	bool setState(EngineState target);
	bool setState(EngineState target, EngineState fallback);
	bool allEngineStateCheck(const std::map<STI::Device::DeviceID, STI::Engine::EngineState>& engineStates, const STI::Engine::EngineState& state);

	void releaseParseLock();
	void releasePlayLock();
	void releaseTriggerLock();

	bool appendPlayMessages(const std::vector<EnginePlayingMessage>& messages);
	EnginePlayingMessage& addPlayMessage(std::vector<EnginePlayingMessage>& messages, const PlayingMessageType& type, const std::string& name);

	STI::Device::DeviceMessageGrouper<STI::Device::EngineStateMessage> engineStateMessageGrouper;

	EngineClock engineClock;

	EngineID engineID;
	STI::Device::DeviceID localDeviceID;	//The DeviceID of the host of this engine
	STI::Engine::ParseID lastParseID;		//The ID of the most recently parsed shot
	EngineJobID activeParseJobID;
	EngineJobID activePlayJobID;
	STI::Engine::EventEngineJob* activePlayJobPtr = nullptr;
	STI::Device::DeviceID triggerDeviceID;	//The DeviceID used for triggering (could be localDeviceID or another device)

	std::shared_ptr<ParseResult> lastParseResult;

	EventEngineStateMachine stateMachine;
	
	EventEngineParser parser;
	DeviceEventParser* deviceParser;
	EngineTriggerTarget* triggerTarget;

	std::shared_ptr<STI::Device::DeviceCollection> deviceCollection;
	std::shared_ptr<STI::Device::ChannelManager> localChannels;
	std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
	std::shared_ptr<STI::Device::AttributeManager> attributeManager;

	std::shared_ptr<EventEngineDependencyTree> dependencyTree;
	std::shared_ptr<EventEngineDependencyTree> localSubtree;

	STI::Utils::OrderedBufferMap<ShotID, std::shared_ptr<FullShotResult>> resultBuffer;	//Each ShotID refers to a single shot's cached results.

	DeviceEventMap eventsByTarget;
	TargetDeviceEventMap eventsByAbstractTarget;

	std::shared_ptr<RawEventGroup> upstreamPartnerEvents;
	std::shared_ptr<RawEventGroup> handledPartnerEvents;
	std::shared_ptr<RawEventGroup> unhandledEvents;

	std::vector<EnginePlayingMessage> playReadyMessages;
	EnginePlayingMessageCount localPlayMsgCounter;
	std::vector<EnginePlayingMessage> localPlayMessages;

	std::vector<EngineParsingMessage> parsingMessages;
	SynchronousEventVector synchedEvents;	//Generated by device specific parseDeviceEvents()

	std::shared_ptr<MasterTrigger> masterTrigger;
	std::shared_ptr<TriggerCallback> masterTriggerCB;

	std::vector<STI::Device::DeviceID> ownedTargets;
	std::set<STI::Device::DeviceID> missingTargets;
	std::map<STI::Device::DeviceID, STI::Engine::EngineState> parsedOwnedTargets;
	std::map<STI::Device::DeviceID, STI::Engine::EngineState> playReadyOwnedTargets;
	std::map<STI::Device::DeviceID, STI::Engine::EngineState> playedOwnedTargets;

	std::map<STI::Device::DeviceID, std::shared_ptr<EventEngine>> engines;

	std::thread playThread;

	bool cancelled;
	bool isJobOwner;
	bool activeParseJob = false;
	bool activePlayJob = false;
	bool eventsByTargetCached;
	std::string baseEventGroupName;

	//play message
	mutable std::mutex playMessageMutex;

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
