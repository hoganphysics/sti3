

#include "LocalEventEngine.h"

#include <sti/device/DeviceCollection.h>
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessageDispatcher.h>
#include <sti/engine/DeviceEventParser.h>
#include <sti/engine/EngineState.h>
#include "EventEngineDependencyTree.h"
#include "LocalEventEngineJob.h"
#include "EventEngineParser.h"
#include <sti/engine/EventEngineScheduler.h>

#include <sti/engine/SynchronousEvent.h>
#include "LocalTriggerCallback.h"
#include <sti/device/Device.h>
#include "Shot.h"
#include "LocalShot.h"
#include <sti/engine/EventEngineJob.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineParsingMessage.h>
#include "MasterTrigger.h"
#include <sti/device/Channel.h>
#include <sti/engine/ResultsCollector.h>
#include "LocalPersistenceManager.h"
#include "ParsedDependencyTree.h"
#include <sti/device/AttributeManager.h>
#include "RawEventGroup.h"
#include <sti/engine/ShotResult.h>
#include <sti/engine/FullShotResult.h>


#include <memory>
#include <thread>
#include <iterator>
#include <vector>
#include <functional>


using STI::Engine::DeviceEventParser;
using STI::Engine::EngineState;
using STI::Engine::LocalEventEngine;
using STI::Engine::ParseID;
using STI::Engine::RawEventVector;
using STI::Engine::TriggerCallback;
using STI::Device::DeviceID;
using STI::Device::EngineSchedulerMessage;
using STI::Engine::DeviceEventMap;
using STI::Engine::TimeStamp;
using STI::Engine::LocalEventEngineJob;
using STI::Engine::LocalShot;
using STI::Engine::LocalTriggerCallback;
using STI::Engine::EventEngineJob;
using STI::Engine::EngineJobID;
using STI::Engine::EngineParsingMessage;
using STI::Engine::MasterTrigger;
using STI::Engine::ResultsCollector;
using STI::Engine::ParsedDependencyTree;
using STI::Engine::ShotResult;
using STI::Engine::RawEventGroup;
using STI::Engine::FullShotResult;


// server1.triggerEvent(ch(server1,slow,4), 5.0)		//trigger just server1
// mainserver.triggerEvent(ch(server1,slow,4), 5.0)		//trigger entire system


LocalEventEngine::LocalEventEngine(const EngineID& engineID, const STI::Device::DeviceID& localID, 
								   const std::shared_ptr<STI::Device::ChannelManager>& channels,
								   const std::shared_ptr<STI::Device::AttributeManager>& attributeManager,
 								   DeviceEventParser* deviceParser, const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher, 
 								   const std::shared_ptr<STI::Device::DeviceCollection>& collection,
								   const std::shared_ptr<STI::Device::PersistenceManager>& persistence) 
  : MessageGenerator(dispatcher),
  	engineID(engineID),
	parser(engineID, localID, channels, deviceParser), 
	deviceParser(deviceParser),
	resultBuffer(3),
	localDeviceID(localID),
	localChannels(channels),
	attributeManager(attributeManager),
	deviceCollection(collection),
	cancelled(false),
	engineStateMessageGrouper(dispatcher),
	isJobOwner(false),
	eventsByTargetCached(false),
	persistenceManager(persistence)
{
	engineStateMessageGrouper.setWarmup(100);   //ms
    engineStateMessageGrouper.setCooldown(500); //ms

	engineStateMessageGrouper.start();

	clear();
}

LocalEventEngine::~LocalEventEngine()
{
	resetPlayThread();
	engineStateMessageGrouper.stop();
	clear();
}

void LocalEventEngine::clear()
{
	parser.clear();

	synchedEvents.clear();

	eventsByTarget.clear();
	eventsByTargetCached = false;
	eventsByAbstractTarget.clear();
	upstreamPartnerEvents = 0;
	handledPartnerEvents = 0;
	unhandledEvents = 0;
	// upstreamPartnerEvents.clear();
	// handledPartnerEvents.clear();
	ownedTargets.clear();
	parsedOwnedTargets.clear();
	playReadyOwnedTargets.clear();
	playedOwnedTargets.clear();

	localParsingMessages.clear();

	baseEventGroupName = "";

	lastParseResult = std::make_shared<ParseResult>();

	//If this fails for some reason, getState() will reflect this.
	setState(EngineState::Idle);
	cancelled = false;
}

/// Get the list of DeviceIDs that name this device as their server
void LocalEventEngine::getOwnedDeviceIDs(std::set<STI::Device::DeviceID>& ownedIDs)
{
	std::vector<DeviceID> ids;
	dependencyTree->getDependedentNodes(localDeviceID, ids);

	//Remove any devices that do not list this device as server
	for(auto& id : ids) {
		if(isActingServerForDevice(id)) {
			ownedIDs.insert(id);
		}
	}
}

bool LocalEventEngine::isTargetServerForDevice(const STI::Device::DeviceID& id)
{
	return id.getTargetServerID() == localDeviceID.getID();
}

bool LocalEventEngine::isActingServerForDevice(const STI::Device::DeviceID& id)
{
	//The local device will act as the server for any of its partner devices *if* it is the job owner.
	//A partner device will return return true for isEventTarget(id).
	bool isPartnerServer = isJobOwner && deviceParser->isEventTarget(id);
	return isTargetServerForDevice(id) || isPartnerServer;
}


void LocalEventEngine::divideEvents(const std::shared_ptr<RawEventGroup>& eventGroup, 
									const std::string& subgroupName, const std::set<STI::Device::DeviceID>& ownedIDs, 
									std::shared_ptr<RawEventGroup>& unhandledEventGroup)
{
	if (eventGroup == 0) return;

	for (auto& g : eventGroup->getSubgroups()) {
		divideEvents(g, subgroupName + "/" + g->getName(), ownedIDs, unhandledEventGroup);
	}

	auto events = eventGroup->getEvents();

	if (events == 0) return;

	RawEventTarget target;

	for (auto& evt : *events) {
		if (evt.getTarget().isAbstract()) {
			
			if (eventGroup->getConcreteTarget(evt.getTarget(), target)) {
				// addEvent(target.device().deviceID(), evt, subgroupName);
				// eventsByTarget[target.device().deviceID()].addEvent( std::move(evt) );
				getTargetEventGroup( target.device().deviceID() ).addEvent( std::move(evt), subgroupName );
			}
			else {
				//abstract target
				eventsByAbstractTarget[evt.getTarget().device()]->addEvent( std::move(evt) );
			}
		}
		else {
			// addEvent(target.device().deviceID(), evt, subgroupName);
			// getTargetEventGroup( target.device().deviceID() ).addEvent( std::move(evt), subgroupName );
			addEvent(evt, subgroupName, ownedIDs, unhandledEventGroup);
		}
	}

}

//should be full group name; take first name and used to construct new base groups. Use the rest (the relative group name) to addEvent on the base group
// void LocalEventEngine::addEvent(const STI::Device::DeviceID& deviceID, const RawEvent& evt, const std::string& subgroupName)
// {
// 	getTargetEventGroup(deviceID).addEvent( std::move(evt), subgroupName );
// }

void LocalEventEngine::addEvent(const RawEvent& evt, const std::string& subgroupName, 
								const std::set<STI::Device::DeviceID>& ownedIDs, std::shared_ptr<RawEventGroup>& unhandledEventGroup)
{
	STI::Device::DeviceID branchID;

	auto it = ownedIDs.find(evt.target().device().deviceID());

	if (localDeviceID == evt.target().device().deviceID() || it != ownedIDs.end()) {
		//Event target is this device or is directly owned by this device
		getTargetEventGroup( evt.target().device().deviceID() ).addEvent( std::move(evt), subgroupName );
	}
	else if (dependencyTree->getBranchToTarget(localDeviceID, evt.target().device().deviceID(), branchID)) {
		//Event target is in the subgraph under branchID
		getTargetEventGroup(branchID).addEvent( std::move(evt), subgroupName );
	}
	else if (upstreamPartnerEvents != 0) {
		//Event target not in this subgraph; these events will handled elsewhere
		unhandledEventGroup->addEvent( std::move(evt), subgroupName );
	}
}


void LocalEventEngine::divideEvents(const std::shared_ptr<RawEventGroup>& eventGroup, std::shared_ptr<RawEventGroup>& unhandledEventGroup)
{
	if (eventGroup == 0) return;

	std::set<STI::Device::DeviceID> ownedIDs;
	getOwnedDeviceIDs(ownedIDs);

	divideEvents(eventGroup, "", ownedIDs, unhandledEventGroup);

	// STI::Device::DeviceID branchID;

	// for(auto& evt : *(eventGroup->getEvents())) {
	// 	auto it = ownedIDs.find(evt.target().device().deviceID());

	// 	if (localDeviceID == evt.target().device().deviceID() || it != ownedIDs.end()) {
	// 		//Event target is this device or is directly owned by this device
	// 		getTargetEventGroup( evt.target().device().deviceID() ).addEvent( std::move(evt) );
	// 		// eventsByTarget[evt.targetDevice()].push_back(std::move(evt));
	// 	}
	// 	else if (dependencyTree->getBranchToTarget(localDeviceID, evt.target().device().deviceID(), branchID)) {
	// 		//Event target is in the subgraph under branchID
	// 		// eventsByTarget[branchID].push_back(std::move(evt));
	// 		getTargetEventGroup(branchID).addEvent( std::move(evt) );
	// 	}
	// 	else {
	// 		//Event target not in this subgraph; these events will be pushed upstream
	// 		upstreamPartnerEvents.push_back(std::move(evt));
	// 	}
	// }


	// for(auto& evt : events) {
	// 	auto it = ownedIDs.find(evt.targetDevice());

	// 	if (localDeviceID == evt.targetDevice() || it != ownedIDs.end()) {
	// 		//Event target is this device or is directly owned by this device
	// 		eventsByTarget[evt.targetDevice()].push_back(std::move(evt));
	// 	}
	// 	else if (dependencyTree->getBranchToTarget(localDeviceID, evt.targetDevice(), branchID)) {
	// 		//Event target is in the subgraph under branchID
	// 		eventsByTarget[branchID].push_back(std::move(evt));
	// 	}
	// 	else {
	// 		//Event target not in this subgraph; these events will be pushed upstream
	// 		upstreamPartnerEvents.push_back(std::move(evt));
	// 	}
	// }
}

RawEventGroup& LocalEventEngine::getTargetEventGroup(const STI::Device::DeviceID& deviceTarget)
{
	auto it = eventsByTarget.find(deviceTarget);
	if (it == eventsByTarget.end()) {
		auto res = eventsByTarget.insert( {deviceTarget, std::make_shared<RawEventGroup>(baseEventGroupName, "") } );
		it = res.first;
	}
	return *(it->second);
}


void LocalEventEngine::mergePartnerEvents(const DeviceEventMap& eventMap)
{
	std::set<STI::Device::DeviceID> ownedIDs;
	getOwnedDeviceIDs(ownedIDs);

	STI::Device::DeviceID branchID;

	for (auto& e : eventMap) {
		
		const STI::Device::DeviceID& id = e.first;
		auto& evtGroup = e.second;
		auto it = ownedIDs.find(id);

		if (localDeviceID == id || it != ownedIDs.end()) {
			//Event target is this device or is directly owned by this device

			//Deep copy to report partner events
			handledPartnerEvents->copyEvents(*evtGroup);
			// handledPartnerEvents.insert(handledPartnerEvents.end(), evts.begin(), evts.end());

			getTargetEventGroup(id).merge(*evtGroup);
		}
		else if (dependencyTree->getBranchToTarget(localDeviceID, id, branchID)) {
			//Event target is in the subgraph under branchID

			//Deep copy to report partner events
			handledPartnerEvents->copyEvents(*evtGroup);
			// handledPartnerEvents.insert(handledPartnerEvents.end(), evts.begin(), evts.end());

			getTargetEventGroup(branchID).merge(*evtGroup);
		}
		else {
			//Event target not in this subgraph; these events will be pushed upstream
			upstreamPartnerEvents->merge(*evtGroup);
			// upstreamPartnerEvents.insert(upstreamPartnerEvents.end(), 
			// 	std::make_move_iterator(evts.begin()), std::make_move_iterator(evts.end()));
		}

		if (isJobOwner) {
			// lastParseResult->baseEventGroup->merge(*evtGroup);
			addEventsToParseResult(evtGroup);
		}
	}
}

void LocalEventEngine::addEventsToParseResult(const std::shared_ptr<RawEventGroup>& newEvents)
{
	if (newEvents != 0 && lastParseResult != 0 && lastParseResult->baseEventGroup != 0) {
		lastParseResult->baseEventGroup->merge(*newEvents);
	}
}

// void LocalEventEngine::mergeAllEvents(std::shared_ptr<RawEventGroup>& targetEventGroup)
// {
// 	targetEventGroup = std::make_shared<RawEventGroup>();
// }

// void LocalEventEngine::mergePartnerEvents(const DeviceEventMap& eventMap)
// {
// 	std::set<STI::Device::DeviceID> ownedIDs;
// 	getOwnedDeviceIDs(ownedIDs);

// 	STI::Device::DeviceID branchID;

// 	for (auto& evtGroup : eventMap) {
		
// 		auto& evts = evtGroup.second;
// 		auto it = ownedIDs.find(evtGroup.first);

// 		if (localDeviceID == evtGroup.first || it != ownedIDs.end()) {
// 			//Event target is this device or is directly owned by this device

// 			//Deep copy to report partner events
// 			handledPartnerEvents.insert(handledPartnerEvents.end(), evts.begin(), evts.end());

// 			eventsByTarget[evtGroup.first].insert(eventsByTarget[evtGroup.first].end(), 
// 				std::make_move_iterator(evts.begin()), std::make_move_iterator(evts.end()));
// 		}
// 		else if (dependencyTree->getBranchToTarget(localDeviceID, evtGroup.first, branchID)) {
// 			//Event target is in the subgraph under branchID

// 			//Deep copy to report partner events
// 			handledPartnerEvents.insert(handledPartnerEvents.end(), evts.begin(), evts.end());

// 			eventsByTarget[branchID].insert(eventsByTarget[branchID].end(), 
// 				std::make_move_iterator(evts.begin()), std::make_move_iterator(evts.end()));
// 		}
// 		else {
// 			//Event target not in this subgraph; these events will be pushed upstream
// 			upstreamPartnerEvents.insert(upstreamPartnerEvents.end(), 
// 				std::make_move_iterator(evts.begin()), std::make_move_iterator(evts.end()));
// 		}
// 	}
// }

const STI::Engine::ParseID& LocalEventEngine::getLastParseID() const 
{
	std::unique_lock<std::mutex> parseLock(parseMutex);

	return lastParseID;
}

bool LocalEventEngine::getParseResult(const ParseID& parseID, std::shared_ptr<ParseResult>& parseResult) const
{
	std::unique_lock<std::mutex> parseLock(parseMutex);

	if (parseID == lastParseID) {
		parseResult = lastParseResult;
		return (parseResult != 0);
	}
	else {
		//check shot result buffer for any recent shots with this parseID
		std::set<ShotID> ids;
		std::shared_ptr<FullShotResult> fullShotResult;

		resultBuffer.getKeys(ids);
		for (auto& id : ids) {
			if (id.parseID == parseID && resultBuffer.get(id, fullShotResult) && fullShotResult != 0) {		
				parseResult = fullShotResult->parseResult;
				return (parseResult != 0);
			}
		}
	}
	return false;
}

// //return a deep copy (rather than a reference) to ensure that the returned events are not modifed by a subsequent operation in LocalEventEngine
// bool LocalEventEngine::getParsedEvents(const STI::Engine::ParseID& parseID, DeviceEventMap& parsedEvents)
// {
// 	std::unique_lock<std::mutex> parseLock(parseMutex);

// 	// bool available = lastParseID == parseID && isState(EngineState::Parsed);
// 	bool available = lastParseID == parseID;

// 	if (!available) return false;

// 	if (!eventsByTargetCached) {
// 		bool success = true;

// 		//get owned device events
// 		for (auto& engine : engines) {
// 			DeviceEventMap engineEventsByTarget;
// 			success = engine.second->getParsedEvents(parseID, engineEventsByTarget);
			
// 			if (!success) return false;	//fails if any owned devices have overwritten parseID

// 			for (auto& tuple : engineEventsByTarget) {
// 				auto& evtGroup = tuple.second;
// 				if (evtGroup != 0) {
// 					getTargetEventGroup(tuple.first).merge(*evtGroup);
// 				}
// 				// eventsByTarget[tuple.first].insert(eventsByTarget[tuple.first].end(),
// 				// 		std::make_move_iterator(evts.begin()), std::make_move_iterator(evts.end()));
// 			}
// 		}

// 		//sort local events
// 		for(auto& tuple : eventsByTarget) {

// 			if (tuple.second != 0) {
// 				tuple.second->sortEvents();
// 			}
// 			// std::sort(tuple.second.begin(), tuple.second.end());
// 		}

// 		eventsByTargetCached = true;
// 	}

// 	parsedEvents = eventsByTarget;	//deep copy
// 	return true;
// }

std::shared_ptr<ParsedDependencyTree> LocalEventEngine::getParsedTree() const 
{
	// auto tree = std::make_shared<ParsedDependencyTree>(dependencyTree);
	// return tree; 

	// lastParseResult->parsedDevices = std::make_shared<ParsedDependencyTree>(dependencyTree);

	return lastParseResult->parsedDevices;
}

void LocalEventEngine::parse(STI::Engine::EventEngineJob& job)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);		//parse function is not reentrant

	clear();

	// std::this_thread::sleep_for(std::chrono::milliseconds(500));

	isJobOwner = (job.getJobOwner() == localDeviceID);

	// job.addMessage(ParsingMessageType::Warning, 100, "Test message")
    //         << "This is a test.";

	if (!setState(EngineState::Parsing)) {
		setState(EngineState::Error);
		return;
	}

	std::shared_ptr<Shot> shot;

    if (!job.getShot(shot)) {
		//Error: no parsed shot
        job.addMessage(ParsingMessageType::Error, 20, "Missing shot")
            << "Parsing aborted: The submited EventEngineJob has a null Shot. There are no events to parse.";
		setState(EngineState::Error);
		return;
	}
    if (!job.getDependencies(dependencyTree)) {
		//Error: no tree
		job.addMessage(ParsingMessageType::Error, 21, "Missing dependency graph")
            << "Parsing aborted: The submited EventEngineJob has a null EventEngineDependencyTree. "
			<< "Cannot proceed without the event target dependency graph.";
		setState(EngineState::Error);
		return;
	}

	lastParseID = job.getJobID().pid;
	lastParseResult->pid = lastParseID;
	lastParseResult->parsedDevices = std::make_shared<ParsedDependencyTree>(dependencyTree);
	lastParseResult->messages = localParsingMessages;
	lastParseResult->stackTraceResult;
//	dependencyTree = job.dependencies;

	

	std::shared_ptr<RawEventGroup> eventGroup;
	shot->getBaseEventGroup(eventGroup);

	if (eventGroup != 0) {
		baseEventGroupName = eventGroup->getName();
		upstreamPartnerEvents = std::make_shared<RawEventGroup>(baseEventGroupName, "");
		handledPartnerEvents = std::make_shared<RawEventGroup>(baseEventGroupName, "");
		unhandledEvents = std::make_shared<RawEventGroup>(baseEventGroupName, ""); 

		lastParseResult->baseEventGroup = eventGroup;

		divideEvents(eventGroup, unhandledEvents);
	}
	else {
		//Error: null event vector
		job.addMessage(ParsingMessageType::Error, 22, "Null event vector")
            << "Parsing aborted: The submited EventEngineJob has a null RawEventVector. "
			<< "There are no events to parse.";
		setState(EngineState::Error);
	}
	
	if (eventsByAbstractTarget.size() > 0) {
		//Warning: Some event targets were not found. Parsing is abstract only; cannot be played.
		job.addMessage(ParsingMessageType::Warning, 1200, "Abstract Shot")
		<< "Some event targets were not found. Parsing is abstract only; cannot be played.";
	}

	//Get the subtree with the localDeviceID as root (Note, the graph is already known to be a DAG)
	localSubtree = std::make_shared<EventEngineDependencyTree>();
	dependencyTree->getSubtree(localDeviceID, *localSubtree);

    std::vector<DeviceID> orderedDependents;
    localSubtree->sortTree(orderedDependents);

	//Parse all devices in order, based on dependency tree.
	int dependencyCount;
	auto nextID = orderedDependents.begin();

	while (isState(EngineState::Parsing) && nextID != orderedDependents.end()) {

		std::string devName = nextID->getName();

		if (!localSubtree->getDependentNodeCount(*nextID, dependencyCount)) {
			//Error; Could not get dependency count (?)
			job.addMessage(ParsingMessageType::Error, 23, "Dependency count failed")
				<< "Failed to get dependency count for device '" << nextID->getID()
				<< "'. The device was not found in the dependency graph. "
				<< "This should not happen and likely indicates a bug in the STI library.";
		}

		if (dependencyCount == 0) {
			parseDevice(*nextID, job);
			++nextID;
		}
		else {
			parseCondition.wait(parseLock);
		}
	}

	//Note that ownedTargets is only modified during the previous while loop,
	//so from now on it is a static list of the owned devices of this device.

	//Wait for all owned target devices to reach Parsed state
	if (ownedTargets.size() > 0) {
		// This device is a server; wait for owned devices to send Parsed messages.
		// When an owned device finishes parsing, it sends this device a message and will be added to parsedOwnedTargets
		while (isState(EngineState::Parsing) && parsedOwnedTargets.size() != ownedTargets.size()) {
			parseCondition.wait(parseLock);
		}
	}

	// Check that owned devices are Parsed
	bool ownedDevicesParsed = allEngineStateCheck(parsedOwnedTargets, EngineState::Parsed);

	job.addMessages(localParsingMessages);
	const std::vector<EngineParsingMessage>& jobMessages = job.getParsingMessages();

	//TODO:  Parse errors and warnings
	//auto& localMessages = parser.getParsingMessages();

	bool errors = false;
	bool cancelJob = false;

	for (auto& m : jobMessages) {
		if (m.getType() == STI::Engine::ParsingMessageType::Error) {
			errors = true;
			break;
		}
	}

	if (errors || !ownedDevicesParsed) {
		cancelJob = true;
	}
	else if (!setState(EngineState::Parsed)) {
		cancelJob = true;
	}

	if (cancelJob) {
		stop();
		setState(EngineState::Error);
		job.markCancelled();
		cancelled = true;
	}

	if (isJobOwner) {
		// The job owner adds all handledPartnerEvents (collected from downstream) to the 
		// top level eventsByTarget list stored locally.  This should then contains all generated partner events.
		// divideEvents(handledPartnerEvents);
		// handledPartnerEvents->clear();
		// mergeAllEvents(lastParseResult->baseEventGroup);
		addEventsToParseResult(handledPartnerEvents);
		handledPartnerEvents->clear();
		// lastParseResult->baseEventGroup->merge(*handledPartnerEvents);

		// lastParseResult->baseEventGroup->merge(...);

		//Any upstreamPartnerEvents at this point implies an abstract shot (targets not found in graph)
		if (upstreamPartnerEvents != 0 && !upstreamPartnerEvents->eventsEmpty()) {
			job.addMessage(ParsingMessageType::Warning, 1200, "Abstract Shot")
			<< "Some event targets were not found. Parsing is abstract only; cannot be played.";
		}
		addEventsToParseResult(upstreamPartnerEvents);
		// lastParseResult->baseEventGroup->merge(*upstreamPartnerEvents);
	}

	// Send message upstream indicating that this device (and all owned devices) has finished
	auto parseCompleteMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID, 
							EngineSchedulerMessage::SchedulerMessageType::ParseComplete);
	parseCompleteMessage->jobID.type = job.getJobID().type;
	parseCompleteMessage->jobID.pid = job.getJobID().pid;
	parseCompleteMessage->upstreamPartnerEvents.swap(upstreamPartnerEvents);
	parseCompleteMessage->handledEvents.swap(handledPartnerEvents);
	parseCompleteMessage->unhandledEvents.swap(unhandledEvents);
	parseCompleteMessage->messages.insert(parseCompleteMessage->messages.end(), jobMessages.begin(), jobMessages.end());
	parseCompleteMessage->engineState = getState();


	//pass local engine reference upstream to server
	std::shared_ptr<STI::Engine::EventEngine> jobEngine;
	job.getEngine(jobEngine);
	parseCompleteMessage->setEngine(jobEngine);

	sendMessage(parseCompleteMessage);

	// if (job.getJobOwner() == localDeviceID) {
	// 	DeviceEventMap eventMap;
	// 	getParsedEvents(lastParseID, eventMap);
	// }

	//TODO: state needs to be contingent on errors from this device or owned devices.

}

void LocalEventEngine::parseDevice(const STI::Device::DeviceID& id, STI::Engine::EventEngineJob& job)
{
	if (id == localDeviceID) {
		//Parse local
		if (parser.parse( getTargetEventGroup(localDeviceID), synchedEvents )) {
			//successfully parsed
			mergePartnerEvents(parser.partnerEvents);
		}
		else {
			//parse failed
			stop();
		}

		auto& engineParserMessages = parser.getParsingMessages();
		localParsingMessages.insert(localParsingMessages.end(), engineParserMessages.begin(), engineParserMessages.end());


		//temp!  Send event?  Must send event (with errors) upstream to client
		localSubtree->removeNode(localDeviceID);
		parseCondition.notify_all();
	}
	else if (isActingServerForDevice(id)) {
		//Start parse job on remote device
	    std::shared_ptr<STI::Device::Device> device;
    	std::shared_ptr<EventEngineScheduler> scheduler;

		auto it = eventsByTarget.find(id);
		if (it == eventsByTarget.end() || it->second->getEvents() == 0 || it->second->getEvents()->size() == 0) {
			//no events for this device; skip this id
			return;
		}

		if (deviceCollection->get(id, device) && device != 0 
			&& device->getEngineScheduler(scheduler)) {

				// auto evts = std::make_shared<RawEventVector>();
				// (*evts) = std::move(eventsByTarget[id]);

				auto shot = scheduler->createShot(job.getJobID().pid.shotConfig, it->second);
				
				auto newJob = std::make_shared<LocalEventEngineJob>(job.getJobID().pid, shot, job.getJobOwner());
				newJob->setDependencies(dependencyTree);
				newJob->setMissingTargets(job.getMissingTargetIDs());
				
				job.attachSubjob(newJob);

				scheduler->addJob(newJob);
				ownedTargets.push_back(id);
		}
		else {
			//Warning: Could not contact device. Parsing is abstract only; cannot be played.
			job.addMessage(ParsingMessageType::Warning, 1100, "Missing device")
			<< "Could not contact device '" << id.getID()
			<< "'. Parsing is abstract only and cannot be played.";
		}
	}
	else {
		//The localDevice is not acting as the server for this id

		auto it = eventsByTarget.find(id);
		
		if (it == eventsByTarget.end() || (it->second != 0 && it->second->eventsEmpty())) {
			//no events for this device; skip this id
			return;
		}

		job.addMessage(ParsingMessageType::Error, 60, "Wrong device server")
			<< "Attempted to parse device '" << id.getID()
			<< "' from server'" << localDeviceID.getID()
			<< "'. This is not the correct acting server for '" << id.getID()
			<< "'.";
	}
}

void LocalEventEngine::handleParseMessage(const std::shared_ptr<EngineSchedulerMessage>& message)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);
	
	if (message == 0) {
		return;
	}

	std::shared_ptr<EventEngine> remoteEngine;
	remoteEngine = message->getEngine();

	if (remoteEngine == 0) {
		//error
		return;
	}

	auto it = std::find(ownedTargets.begin(), ownedTargets.end(), message->originalSourceID());

	//only add engine if it is owned by this device
	if (it != ownedTargets.end()) {
		parsedOwnedTargets[message->originalSourceID()] = message->engineState;
		engines[remoteEngine->getDeviceID()] = remoteEngine;
	}

	localParsingMessages.insert(localParsingMessages.end(), message->messages.begin(), message->messages.end());

	if (!isState(EngineState::Parsing)) {
		return;
	}

	//Attempt to handle generated events locally, or pass upstream
	divideEvents(message->upstreamPartnerEvents, upstreamPartnerEvents);

	//Collect any handled partner events
	if (message->handledEvents != 0 && handledPartnerEvents != 0) {
		handledPartnerEvents->merge( *(message->handledEvents) );		
	}

	//Unhandled events are from missing targets.
	//These events have bounced back and do not need to be added to the parse result again.
	if (message->unhandledEvents != 0 &&  unhandledEvents!= 0) {
		unhandledEvents->merge( *(message->unhandledEvents) );
	}

	//reduce dependency count in localSubtree
	localSubtree->removeNode(message->originalSourceID());

	parseCondition.notify_all();	//wake up event transfer loop
}

void LocalEventEngine::handlePlayReadyMessage(const std::shared_ptr<EngineSchedulerMessage>& message)
{
	std::unique_lock<std::mutex> playLock(playMutex);

	std::shared_ptr<EventEngine> remoteEngine;

	if (message != 0) {
		remoteEngine = message->getEngine();
	}

	if (remoteEngine == 0) {
		//error
		return;
	}

	auto it = std::find(ownedTargets.begin(), ownedTargets.end(), remoteEngine->getDeviceID());

	//only add engine if it is owned by this device
	if (it != ownedTargets.end()) {
		engines[remoteEngine->getDeviceID()] = remoteEngine;
		playReadyOwnedTargets[message->originalSourceID()] = message->engineState;
	}

	playCondition.notify_all();
}

void LocalEventEngine::handlePlayCompleteMessage(const std::shared_ptr<EngineSchedulerMessage>& message)
{
	std::unique_lock<std::mutex> playLock(playMutex);

	if (message == 0) {
		return;
	}

	auto it = std::find(ownedTargets.begin(), ownedTargets.end(), message->originalSourceID());

	//only add if it is owned by this device
	if (it != ownedTargets.end()) {
		playedOwnedTargets[message->originalSourceID()] = message->engineState;
	}

	playCondition.notify_all();
}

TimeStamp LocalEventEngine::getCurrentTimeStamp()
{
	TimeStamp ts;
	// ts.timestamp = 0;
	return ts;
}


void LocalEventEngine::scheduleAllPlayJobs(const EngineJobID& jobID, const DeviceID& jobOwner)
{
	if (ownedTargets.size() == 0) {
		return;
	}

	//Start play job on remote devices
	std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<EventEngineScheduler> scheduler;

	for (auto& id : ownedTargets) {
		
		if (deviceCollection->get(id, device) && device != 0 && device->getEngineScheduler(scheduler)) {

			auto newJob = std::make_shared<LocalEventEngineJob>(jobID, jobOwner);
			scheduler->addJob(newJob);
		}
		else {
			//Error: Could not contact device to play
		}
	}
}


void LocalEventEngine::play(EventEngineJob& job)
{
	if (!isState(EngineState::Parsed) && lastParseID != job.getJobID().pid) {
		//Error: this device is not parsed for this job. Should not happen because EngineScheduler should check.
		//Send message upstream
		return;
	}

	std::unique_lock<std::mutex> playLock(playMutex);
	cancelled = false;

	if (eventsByAbstractTarget.size() > 0) {
		job.addMessage(ParsingMessageType::Error, 70, "Cannot Play Abstract Shot")
            << "The parsed shot is abstract and cannot be played.";
		setState(EngineState::Error);
	}

	if (!setState(EngineState::PreparingPlay)) {
		//error
		cancelled = true;
		return;
	}

	playReadyOwnedTargets.clear();
	playedOwnedTargets.clear();
	// engines.clear();

	//TimeStamp playTime;
	EngineJobID jobID = job.getJobID();
	
	isJobOwner = (job.getJobOwner() == localDeviceID);

	if (isJobOwner) {
		//playTime = getCurrentTimeStamp();
		jobID.runTime = getCurrentTimeStamp();
	}
// 	else {
// 		jobID.runTime = jobID.runTime;
// //		playTime = jobID.sid.playTime;
// 	}
	//jobID.runTime =
	//jobID.sid.playTime = playTime;

	scheduleAllPlayJobs(jobID, job.getJobOwner());

	//Wait for all owned target devices to reach PlayReady state
	if (ownedTargets.size() > 0) {
		// This device is a server; wait for owned devices to send PlayReady messages.
		while (isState(EngineState::PreparingPlay) && playReadyOwnedTargets.size() != ownedTargets.size()) {
			playCondition.wait(playLock);
		}
	}

	// Check that owned devices are PlayReady
	bool ownedDevicesPlayReady = allEngineStateCheck(playReadyOwnedTargets, EngineState::PlayReady);

	if (!ownedDevicesPlayReady) {
		stop();

		// auto& err = job.addMessage(PlayingMessageType::Error, 1, "PlayReady")
		// 	<< "The following devices failed to reach the PlayReady state: \n";
		// for (auto& tuple : playReadyOwnedTargets) {
		// 	if (tuple.second != EngineState::PlayReady) {
		// 		err << tuple.first.getID() << " (EngineState = " << print(tuple.second) << ")\n";
		// 	}
		// }
	}

	if (!setState(EngineState::PlayReady)) {
		setState(EngineState::Error);
	}

	//Send PlayReady message with local engine reference
	auto playReadyMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID, 
								EngineSchedulerMessage::SchedulerMessageType::PlayReady);
	playReadyMessage->jobID.pid = job.getJobID().pid;
	playReadyMessage->jobID.sid = job.getJobID().sid;
	playReadyMessage->jobID.type = job.getJobID().type;
	playReadyMessage->engineState = getState();
	
	//pass local engine reference upstream to server
	std::shared_ptr<STI::Engine::EventEngine> jobEngine;
	job.getEngine(jobEngine);
	playReadyMessage->setEngine(jobEngine);
	
	sendMessage(playReadyMessage);


	if (!isState(EngineState::PlayReady)) {
		// an error occurred, or play was aborted
		job.markCancelled();
		stop();
		return;
	}

	// Setup trigger
	STI::Device::DeviceID triggerDeviceID = job.getJobOwner();	//temp!
	masterTrigger = std::make_shared<MasterTrigger>(triggerDeviceID);
	masterTrigger->arm(ownedTargets);

	if (isJobOwner || ownedTargets.size() > 0) {
		
		masterTriggerCB = std::make_shared<LocalTriggerCallback>(masterTrigger.get());

		if (isJobOwner) {
			play(jobID, masterTriggerCB, false);
		}
	}

	waitForPlayComplete(playLock);	//so job doesn't finish until play finishes or is aborted
	
	// if (isJobOwner || ownedTargets.size() > 0) {
	// 	transferAllMeasurements(jobID.sid);	//need to get all measurements from owned devices
	// }
	
	std::shared_ptr<FullShotResult> cachedShot;
	if (resultBuffer.get(jobID.sid, cachedShot) && cachedShot != 0) {
		if (persistenceManager != 0 && persistenceManager->saveShot(jobID.sid, cachedShot, isJobOwner)) {

			//successfully saved; remove from buffer
			resultBuffer.remove(jobID.sid);
		}
	}

	// if (isJobOwner) {
		
	// 	std::shared_ptr<STI::Engine::EventEngine> localEngine;	//reference to this engine
    //     job.getEngine(localEngine);

	// 	//auto resultsCollector = persistenceManager->createResultsCollector(job.getJobID().sid, localEngine);
		
	// 	//resultsCollector->addEvents(...);
	// 	//resultsCollector->addTimingFiles(...);
	// 	// resultsCollector->addVars(...);
	// 	// persistenceManager->saveShot(resultsCollector);
	// 	bool success = (persistenceManager != 0) && persistenceManager->saveShot(job.getJobID().sid, localEngine);

	// 	if (!success) {
	// 		//stash? or leave in measurement buffer for now (eventually it will be stashed)
	// 	}

	// 	//transferMeasurements(job.getJobID().sid, resultsCollector);

	// 	//get resultsCollector from persistence; should not point to a particular persistence manager
	// 	//addEvents, addTimingFiles, addDeviceTree
	// 	//addEngines;  uses engines if available to pull data; otherwise falls back to deviceID->persistenceManager on remote devices
	// 	//persistenceManager could loop through devices in DeviceTree, calling on engine map by id, and falling back to remotePersistenceManager
	// 	//persistenceManager->saveShot(resultsCollector);  
	// 	// - Uses the first documentation target to setup directory
	// 	// - calls transfer on all engines (or remotePersistenceManager), which sends a callback, which transfers files and then deletes local measurements
	// }

	// saveShot(job);	//job needs record of shot
	
	auto playCompleteMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID, 
								EngineSchedulerMessage::SchedulerMessageType::PlayComplete);
	playCompleteMessage->jobID.pid = job.getJobID().pid;
	playCompleteMessage->jobID.sid = job.getJobID().sid;
	playCompleteMessage->jobID.type = job.getJobID().type;
	playCompleteMessage->engineState = getState();
	// playCompleteMessage->messages
	sendMessage(playCompleteMessage);


	//After play completes (without error or abort), the engine should be in the Parsed state
	if (!isState(EngineState::Parsed)) {
		job.markCancelled();
		cancelled = true;
	}
}


// bool LocalEventEngine::transferResults(const std::shared_ptr<ResultsCollector>& resultsCollector)
// {
// 	if (resultsCollector == 0) return false;

// 	std::shared_ptr<ShotResult> cachedShot;

// 	if (resultBuffer.get(resultsCollector->getShotID(), cachedShot) && cachedShot != 0) {

// 		bool success = true;

// 		if (isJobOwner) {
// 			getParsedEvents(resultsCollector->getShotID().parseID, cachedShot->parsedEvents);
// 			resultsCollector->addEvents(cachedShot->parsedEvents);
// 	//		resultsCollector->addTimingFiles();
// 	//		resultsCollector->addVariables();
// 		}

// 		//Attributes
// 		success &= resultsCollector->addAttributes(localDeviceID, (cachedShot->attributes)[localDeviceID]);

// 		//Measurements
// 		success &= resultsCollector->addMeasurements(cachedShot->measurements);

// 		if (success) {
// 			resultBuffer.remove(resultsCollector->getShotID());
// 		}
// 	}

// // 	std::shared_ptr<MeasurementVector> measurements;

// // 	if (measurementBuffer.get(resultsCollector->getShotID(), measurements)) {
// // 		//Measurements
// // 		if (resultsCollector->addMeasurements(measurements)) {
// // 			measurementBuffer.remove(resultsCollector->getShotID());
// // 		}

// // 		//Attributes
// // 		std::vector<std::shared_ptr<STI::Device::Attribute>> attributes;
// // 		if (attributeManager != 0) {
// // 			attributeManager->getAttributes(attributes);
// // 			resultsCollector->addAttributes(localDeviceID, attributes);
// // 		}

// // 	}

// // 	if (isJobOwner) {
// // //		resultsCollector->addTimingFiles();
// // //		resultsCollector->addVariables();
// // 		DeviceEventMap parsedEvents;
// // 		getParsedEvents(resultsCollector->getShotID().parseID, parsedEvents);
// // 		resultsCollector->addEvents(parsedEvents);
// // 	}

// 	//this device's attributes
// //	resultsCollector->addAttributes(localDeviceID, attributes);

// 	auto tree = resultsCollector->getDependencies();

// 	// EventEngineDependencyTree subtree;
// 	// tree->getSubtree(localDeviceID, subtree);

//     std::vector<DeviceID> nodes;
// 	if (tree != 0) {
// 		tree->getDependedentNodes(localDeviceID, nodes);		
// 	}

// 	std::shared_ptr<STI::Device::Device> device;
//     std::shared_ptr<EventEngineScheduler> scheduler;

// 	bool success;

// 	for (auto& id : nodes) {

// 		if (isActingServerForDevice(id) 	//problem: only works if this shot is the most recently parsed shot
// 			&& deviceCollection->get(id, device) && device != 0 
// 			&& device->getEngineScheduler(scheduler)) 
// 		{
// 			success = scheduler->transferResults(resultsCollector);
// 		}
// 	}

// 	return true;
// }


// void LocalEventEngine::transferAllMeasurements(const ShotID& sid)
// {
// 	std::shared_ptr<MeasurementVector> measurements;
// 	std::shared_ptr<MeasurementVector> targetMeasurements;

// 	measurementBuffer.get(sid, measurements);

// 	for (auto& engine : engines) {
// 		if (engine.second->transferMeasurements(sid, targetMeasurements)) {

// 			measurements->insert(measurements->end(), 
// 							std::make_move_iterator(targetMeasurements->begin()), 
// 							std::make_move_iterator(targetMeasurements->end()));			
// 		}
// 	}
// }



// bool LocalEventEngine::getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements)
// {
// 	std::shared_ptr<ShotResult> shot;

// 	if (resultBuffer.get(sid, shot) && shot != 0) {
// 		measurements = shot->measurements;
// 		return (measurements != 0);
// 	}
// 	return false;
// //	return measurementBuffer.get(sid, measurements) && (measurements != 0);
// }


// // bool LocalEventEngine::transferMeasurements(const ShotID& sid, std::shared_ptr<MeasurementCollector>& collector);
// bool LocalEventEngine::transferMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements)
// {
// 	if (measurementBuffer.get(sid, measurements)) {
// 		//ownership of measurements transfered to caller
// 		return (measurements != 0) && measurementBuffer.remove(sid);
// 	}
// 	return false;
// }

void LocalEventEngine::waitForPlayComplete(std::unique_lock<std::mutex>& playLock)
{
	while (isState(EngineState::PlayReady) || isState(EngineState::PreparingPlay) ||
		   isState(EngineState::WaitingForTrigger) || isState(EngineState::Playing)) {
		playCondition.wait(playLock);
	}
	resetPlayThread();	//calls thread::join on playThread
}


void LocalEventEngine::play(const EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug)
{
	if (!isState(EngineState::PlayReady)) {
		cancelled = true;
		return;
	}

	//std::unique_lock<std::mutex> playLock(playMutex);

	//Make sure we are trying to play the shot that is currently parsed on this engine.
	if (jobID.pid != lastParseID) {
		return;
	}

	//Grab Measurement references generated by parse.
	//These will be filled with data after the shot is played.
	auto newMeasurements = std::make_shared<MeasurementVector>();
	for (auto& synchEvent : synchedEvents) {
		auto& evtMeasurements = synchEvent->getMeasurements();
		newMeasurements->insert(newMeasurements->end(), evtMeasurements.begin(), evtMeasurements.end());
	}

	std::set<STI::Device::DeviceID> ownedIDs;
	getOwnedDeviceIDs(ownedIDs);

	auto cachedShot = std::make_shared<ShotResult>(localDeviceID, ownedIDs);
	cachedShot->playTime = jobID.runTime;
	cachedShot->sid = jobID.sid;
	cachedShot->measurements = newMeasurements;

	auto cachedFullShot = std::make_shared<FullShotResult>();
	cachedFullShot->shotResult = cachedShot;
	cachedFullShot->parseResult = lastParseResult;

	if (attributeManager != 0) {
		std::map<std::string, std::string> attributes;
		attributeManager->getAttributes(attributes);

		(cachedShot->attributes)[localDeviceID] = attributes;
	}

	resultBuffer.add(cachedShot->sid, cachedFullShot);	//Add this shot to the buffer
	//measurementBuffer.add(jobID.sid, newMeasurements);	//Add this shot to the buffer

	//Prepare local events
	for (auto& synchEvent : synchedEvents) {
		synchEvent->reset();		//resets Measurement events if they've played before
		synchEvent->load();			//loads (or reloads) events if needed
	}

	if (ownedTargets.size() > 0 && masterTriggerCB != 0) {
		//masterTrigger is the local server's master trigger (works for all levels of network)
		playAll(jobID, masterTriggerCB, debug);		//all owned devices
	}
	
	//need to wait for owned device to arm before entering playShot to arm locally
	masterTrigger->waitForArm();		//wait for all owned devices to enter WaitingForTrigger state
	masterTrigger->arm(localDeviceID);	//add local device to the arming list

	resetPlayThread();
	playThread = std::thread(&LocalEventEngine::playShot, this, std::ref(*triggerCB));	//callback to calling server (not masterTrigger) 

	//Only the job owner actually calls trigger, even if the trigger is delegated to another device
	if (isJobOwner) {
		
		masterTrigger->waitForArm();		//waits for the local device to enter WaitingForTrigger

		trigger(masterTrigger->triggerID());
	}
}

void LocalEventEngine::playAll(const EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug)
{
	for (auto& engine : engines) {
		engine.second->play(jobID, triggerCB, debug);
	}
}


void LocalEventEngine::resetPlayThread()
{
	if (playThread.joinable()) {
		if (isState(EngineState::Playing) || isState(EngineState::WaitingForTrigger)) {
			//Error; this should not happen
			stop();
		}
		playThread.join();
	}
}


void LocalEventEngine::playShot(TriggerCallback& triggerCB)
{
	if (!armTrigger(triggerCB)) {

		setState(EngineState::Error);
		stop();
		masterTrigger->stop();
		return;
	}

	waitForTrigger();
	
	triggerCB.triggerFired(localDeviceID);		//Callback to the device that called play to confirm that this device is playing.
												//Also, if this device is a delegated system trigger, this indicated 
												//that the rest of the system should now be triggered.

	playDeviceEvents();		//actually play the events on the device
	waitForPlayAll();		//wait until all owned devices complete play

	// bool success = true;
	if (isState(EngineState::Playing)) {
		if (!setState(EngineState::Parsed)) {
			setState(EngineState::Error);
			// success = false;
		}
	}
	
	playCondition.notify_all();		//wake up waitForPlayComplete
}

bool LocalEventEngine::armTrigger(TriggerCallback& triggerCB)
{
	if (!setState(EngineState::WaitingForTrigger)) {
		setState(EngineState::Error);
		return false;
	}
	
	triggerCB.ready(localDeviceID);		//need to include jobID; or make server's callback job dependent (better!)

	masterTrigger->ready(localDeviceID);
	
	return true;
}


void LocalEventEngine::waitForTrigger() const
{
	std::unique_lock<std::mutex> triggerLock(triggerMutex);

	while (isState(EngineState::WaitingForTrigger)) {
		triggerCondition.wait(triggerLock);
	}
}


void LocalEventEngine::waitForPlayAll()
{
	std::unique_lock<std::mutex> playLock(playMutex);
	
	//Wait for all owned target devices to finish play
	if (ownedTargets.size() > 0) {
		// This device is a server; wait for owned devices to send ready messages
		while (isState(EngineState::Playing) && playedOwnedTargets.size() < ownedTargets.size()) {

			playCondition.wait(playLock);
		}
	}

	allEngineStateCheck(playedOwnedTargets, EngineState::Parsed);
}


void LocalEventEngine::trigger(const STI::Device::DeviceID& target)
{
	//Triggers a specific device (allows any device to act as the system trigger)
	if (target == localDeviceID) {
		trigger();
	}
	else {
		//Not this device; pass trigger down the network
		for (auto& engine : engines) {
			engine.second->trigger(target);
		}		
	}
}

void LocalEventEngine::trigger()
{
	if (isState(EngineState::Playing)) {
		return;
	}

	if (!setState(EngineState::Playing)) {
		setState(EngineState::Error);
	}

	std::unique_lock<std::mutex> triggerLock(triggerMutex);
	triggerCondition.notify_all();			//releases waitForTrigger

	triggerOwnedDevices();
}

void LocalEventEngine::triggerOwnedDevices()
{
	for (auto& engine : engines) {
		engine.second->trigger();
	}
}


bool LocalEventEngine::playDeviceEvents()
{
	std::unique_lock<std::mutex> playLock(playMutex);

	if (!setState(EngineState::Playing)) {
		setState(EngineState::Error);
		return false;
	}

	//launch measurements thread
	auto measurementThread = std::thread(&LocalEventEngine::measureData, this);
	
	engineClock.reset();

	auto& rawEventsByTime = parser.rawEvents;
	auto nextRawEvents = rawEventsByTime.begin();

	//play all events (at appropriate times)
	for (auto& evt : synchedEvents) {
		
		if (!isState(EngineState::Playing))
			break;

		//switch(waitUntil(evt->getTime())),  cases for play, pause, stop ?

		if (evt != 0 && waitUntil(playLock, evt->getTime())) {	//success if not interrupted
			evt->waitBeforePlay();
			evt->play();
		}

		//push channel value updates
		if (nextRawEvents != rawEventsByTime.end() 
				&& engineClock.getTime() >= nextRawEvents->first) {
			updateChannelValues(nextRawEvents->second);
			++nextRawEvents;
		}
	}

	//Make sure the last event is saved
	if (nextRawEvents != rawEventsByTime.end()) {
		//only called if the most recent nextRawEvents was not the end() for some reason
		updateChannelValues(rawEventsByTime.rbegin()->second);	//last entry
	}	

	measurementThread.join();	//wait for measurement collection to complete

	return true;	//play and collect were a success, or where cleanly stopped
}


bool LocalEventEngine::waitUntil(std::unique_lock<std::mutex>& lock, double time)
{
	int64_t deltaT;

	while(isState(EngineState::Playing) && (deltaT = engineClock.getWaitInterval(time)) > 0) {
		playCondition.wait_for(lock, std::chrono::nanoseconds(deltaT));
	}

	return isState(EngineState::Playing);
}

void LocalEventEngine::updateChannelValues(const RawEventVector& rawEvents)
{
	//All of these events happen at the same time and have been played.

	std::shared_ptr<STI::Device::Channel> channel;

//	for (const auto& evt : rawEvents) {
	for (unsigned i = 0; i < rawEvents.size(); ++i) {
		if (localChannels->getChannel(rawEvents.at(i).channel(), channel)) {
			channel->saveLastValue(rawEvents.at(i).value());
		}
	}	
}

void LocalEventEngine::pause()
{
	setState(EngineState::Paused);

	pauseOwnedDevices();
}

void LocalEventEngine::pauseOwnedDevices()
{
	for (auto& engine : engines) {
		engine.second->pause();
	}
}

void LocalEventEngine::unpause(bool retrigger) 
{
	//if retrigger, require a trigger before resuming (allows hard time resume)
	if (!setState(EngineState::Playing)) {
		setState(EngineState::Error);
		stop();
	}
	unpauseOwnedDevices(retrigger);
}

void LocalEventEngine::unpauseOwnedDevices(bool retrigger)
{
	for (auto& engine : engines) {
		engine.second->unpause(retrigger);
	}
}


void LocalEventEngine::measureData()
{
	for (auto& evt : synchedEvents) {
		evt->collectData();

		if (!isState(EngineState::Playing))
			break;
	}
}

void LocalEventEngine::stop()
{
	switch (getState()) {
	case EngineState::Parsing:
		setState(EngineState::Idle, EngineState::Error);
		cancelled = true;
		releaseParseLock();
		break;	
	case EngineState::PreparingPlay:
		setState(EngineState::Parsed, EngineState::Error);
		cancelled = true;
		releasePlayLock();
		break;
	case EngineState::PlayReady:
		setState(EngineState::Parsed, EngineState::Error);
		cancelled = true;
		releasePlayLock();
		break;
	case EngineState::WaitingForTrigger:
		setState(EngineState::Parsed, EngineState::Error);
		cancelled = true;
		//trigger();			//in case WaitingForTrigger
		releaseTriggerLock();
		releasePlayLock();
		break;
	case EngineState::Playing:
		setState(EngineState::Parsed, EngineState::Error);
		cancelled = true;
		releasePlayLock();
		stopDeviceEvents();
		break;
	}
	
	stopOwnedDevices();
}


void LocalEventEngine::releaseParseLock()
{
//	std::unique_lock<std::mutex> parseLock(parseMutex);
	parseCondition.notify_all();
}

void LocalEventEngine::releasePlayLock()
{
//	std::unique_lock<std::mutex> playLock(playMutex);
	playCondition.notify_all();		//release play(job)
}

void LocalEventEngine::releaseTriggerLock()
{
//	std::unique_lock<std::mutex> triggerLock(triggerMutex);
	triggerCondition.notify_all();			//releases waitForTrigger
}

void LocalEventEngine::stopOwnedDevices()
{
	for (auto& engine : engines) {
		engine.second->stop();
	}
}

void LocalEventEngine::stopDeviceEvents()
{
	for (auto& evt : synchedEvents) {
		evt->stop();
	}
}

bool LocalEventEngine::isState(EngineState target) const
{
	return stateMachine.isState(target);
}

STI::Engine::EngineState LocalEventEngine::getState() const
{
	return stateMachine.getState();
}

bool LocalEventEngine::setState(EngineState target, EngineState fallback)
{
	bool success = setState(target);
	
	if (!success) {
		success = setState(fallback);
	}
	return success;
}

bool LocalEventEngine::setState(EngineState target)
{
	if (stateMachine.setState(target)) {

		auto eventStatusMessage = std::make_shared<STI::Device::EngineStateMessage>(getDeviceID(), engineID, getState());
		engineStateMessageGrouper.addMessage(eventStatusMessage);

		return true;
	}
	return false;
}


bool LocalEventEngine::allEngineStateCheck(const std::map<STI::Device::DeviceID, STI::Engine::EngineState>& engineStates, const STI::Engine::EngineState& state)
{
	bool success = true;

	for(auto& id : ownedTargets) {
		auto it = engineStates.find(id);
		if (it != engineStates.end()) {
			success &= (it->second == state);
		}
		else {
			success = false;
		}

		if (!success) break;
	}
	return success;
}

