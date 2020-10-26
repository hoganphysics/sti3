
#include "EventEngine.h"
#include "EventEngineParser.h"
#include "DeviceEventParser.h"
#include "RawEvent.h"
#include "SynchronousEvent.h"
#include "Channel.h"
#include "DeviceID.h"
#include "DeviceCollection.h"
#include "EngineState.h"
#include "DeviceEventParser.h"
#include "EventEngineJob.h"
#include "EventEngineScheduler.h"

#include "EventEngineDependencyTree.h"

#include <memory>
#include <thread>

using STI::Engine::DeviceEventParser;
using STI::Engine::EngineState;
using STI::Engine::EventEngine;
//using STI::Engine::ParserCallback;
//using STI::Engine::ParserCallbackMessage;
using STI::Engine::ParseID;
using STI::Engine::RawEventVector;
using STI::Engine::TriggerCallback;
using STI::Device::DeviceID;
using STI::Device::EngineSchedulerMessage;


EventEngine::EventEngine(const STI::Device::DeviceID& localID, STI::Device::ChannelMap& channels, DeviceEventParser* deviceParser,
const std::shared_ptr<STI::Device::DeviceEventDispatcher>& dispatcher, std::shared_ptr<STI::Device::DeviceCollection>& collection) :
	MessageGenerator(dispatcher),
	parser(this, deviceParser), 
	rawEvents(parser.rawEvents), 
	partnerEvents(parser.partnerEvents), 
	measurementBuffer(3),
	localDeviceID(localID),
	localChannels(channels),
	deviceCollection(collection)
{
}

EventEngine::~EventEngine()
{

}

void EventEngine::clear()
{
	rawEvents.clear();
	synchedEvents.clear();
	partnerEvents.clear();

	//If this fails for some reason, getState() will reflect this.
	setState(EngineState::Idle);
}


// void EventEngine::divideEvents(const RawEventVector& events)
// {
// 	std::map<STI::Device::DeviceID, RawEventVector> ownedEvents;
// 	std::map<STI::Device::DeviceID, STI::Device::DeviceID> delegates;
// 	RawEventVector unownedEvents;


// 	std::set<STI::Device::DeviceID> ownedIDs;
// 	deviceCollection->getIDs(ownedIDs);

// 	bool localDeviceIsServer;
// 	bool delegateFound;
// 	std::map<STI::Device::DeviceID, STI::Device::DeviceID>::iterator delegateIt;

// 	for(auto& evt : events) {
// 		//auto it = ownedIDs.find(evt.targetDevice());
// 		// found = (it != ownedIDs.end())
// 		// 		|| (localDeviceID.getID() == evt.targetDevice().getTargetServerID());
// 		localDeviceIsServer = (localDeviceID.getID() == evt.targetDevice().getTargetServerID());

// 		if(!localDeviceIsServer) {
// 			delegateIt = delegates.find(evt.targetDevice());
// 			delegateFound = (delegateIt != delegates.end());			
// 		}

// 		if(localDeviceIsServer) {
// 			ownedEvents[evt.targetDevice()].push_back(evt);
// 		}
// 		else if (delegateFound) {
// 			ownedEvents[delegateIt->second].push_back(evt);
// 		}
// 		else {


// 			unownedEvents.push_back(evt);
// 		}
// 	}
// }

/// Get the list of DeviceIDs that name this device as their server
void EventEngine::getOwnedDeviceIDs(std::set<STI::Device::DeviceID>& ownedIDs)
{
//	deviceCollection->getIDs(ownedIDs);
	std::vector<DeviceID> ids;
	dependencyTree->getDependedentNodes(localDeviceID, ids);

	//Remove any devices that do not list this device as server
	for(auto& id : ids) {
		if (id.getTargetServerID() == localDeviceID.getID()) {
			ownedIDs.insert(id);
		}
	}

    // for (auto it = ownedIDs.begin(); it != ownedIDs.end(); ) {
    //     if (it->getTargetServerID() != localDeviceID.getID()) {
    //         it = ownedIDs.erase(it);
    //     }
    //     else {
    //         ++it;
    //     }
    // }
}

void EventEngine::divideEvents(const RawEventVector& events, RawEventVector& upstreamEvents)
{
	std::set<STI::Device::DeviceID> ownedIDs;
	getOwnedDeviceIDs(ownedIDs);

	STI::Device::DeviceID branchID;

	for(auto& evt : events) {
		auto it = ownedIDs.find(evt.targetDevice());

		if (localDeviceID == evt.targetDevice()) {
			//Event target is this device
			eventsByTarget[localDeviceID].push_back(evt);
		}
		else if (it != ownedIDs.end()) {
			//Event target is directly owned by this device
			eventsByTarget[evt.targetDevice()].push_back(evt);
		}
		else if (dependencyTree->getBranchToTarget(localDeviceID, evt.targetDevice(), branchID)) {
			//Event target is in the subgraph under branchID
			eventsByTarget[branchID].push_back(evt);
		}
		else {
			//Event target not in this subgraph; these events will be pushed upstream
			upstreamEvents.push_back(evt);
		}
	}
}

//void EventEngine::parse(const ParseID& parseID, const RawEventVector& events, const STI::Device::DeviceID& server)
void EventEngine::parse(const std::shared_ptr<STI::Engine::EventEngineJob>& job)
{
	//preliminary event list division
	//Make device tree (if not received?); check for cirular dependencies
	//Devide event list using tree (events sent to direct targets, or to correct branch at least). Event list should only be grouped by owned partners of this device.
	//Recruit direct partners for parse (remote schedulers)
	//wait for direct partners, collecting results
	//Parse dependent devices (including self) as they are ready (all their event dependencies complete)
	//Send (partial parse) messages as dependencies return
	//Send parse complete message upstream

	//sendMessage(...);

	std::unique_lock<std::mutex> parseLock(parseMutex);		//function is not reentrant

	if (!setState(EngineState::Parsing)) {
		return;
	}

	lastParseID = job->getJobID().pid;
	dependencyTree = job->dependencies;

	auto parsedShot = job->parsedShot;
	RawEventVector& events = parsedShot->events;
	RawEventVector upstreamEvents;

	//Divide event list
	eventsByTarget.clear();
	divideEvents(events, upstreamEvents);

	//Get the subtree with the localDeviceID as root (Note, the graph is already known to be a DAG)
	localSubtree = std::make_shared<EventEngineDependencyTree>();
	dependencyTree->getSubtree(localDeviceID, *localSubtree);

    std::vector<DeviceID> orderedDependents;
    localSubtree->sortTree(orderedDependents);

	// //Make a copy of the subtree to track parse dependency status
	// EventEngineDependencyTree depTree;
	// depTree.addTree(*localSubtree);

	//Parse all devices in order, based on dependency tree.
	int dependencyCount;
	auto nextID = orderedDependents.begin();

	while (isState(EngineState::Parsing) && nextID != orderedDependents.end()) {

		if (!localSubtree->getDependentNodeCount(*nextID, dependencyCount)) {
			//Error; Could not get dependency count (?)
		}

		if (dependencyCount == 0) {
			parseDevice(*nextID, job);
		}
		else {
			parseCondition.wait(parseLock);
		}
	}


	auto newMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID, localDeviceID, 
							EngineSchedulerMessage::SchedulerMessageType::ParseComplete);
	//newMessage->engine;

	
	// if (parser.parse(events, synchedEvents, errors)) {
	// 	//successfully parsed
	// 	//lastParseID = job->getJobID().pid;
	// }

//	ParserCallbackMessage message;
//	message.errors = errors;
//	cb.returnResults(message);
}

void EventEngine::parseDevice(const STI::Device::DeviceID& id, const std::shared_ptr<STI::Engine::EventEngineJob>& job)
{
	if (id == localDeviceID) {
		//Parse local
		if (parser.parse(eventsByTarget[localDeviceID], synchedEvents)) {
			//successfully parsed
		}
	}
	else {
		//Start parse job on remote device
	    std::shared_ptr<STI::Device::Device> device;
    	std::shared_ptr<STI::Engine::EventEngineScheduler> scheduler;

		auto shot = std::make_shared<ParsedShot>();
		shot->events = std::move(eventsByTarget[id]);		//expensive deep copy?

		auto newJob = std::make_shared<EventEngineJob>(job->getJobID().pid, shot,
                   dependencyTree, job->jobOwner, job->missingTargetIDs);

		if (deviceCollection->get(id, device) && device != 0 
			&& device->getEngineScheduler(scheduler)) {
				
				scheduler->addJob(newJob);
		}
		else {
			//Warning: Could not contact device. Parsing is abstract only; cannot be played.
		}
	}
}

void EventEngine::handleParseMessage(const std::shared_ptr<STI::Device::EngineSchedulerMessage>& evt)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);
	//divide events
	RawEventVector upstreamEvents;
	//divideEvents(evt->events, upstreamEvents);		//problem: evt->evts and upstreamEvents are duplicated...
	
	//try to handle 
	divideEvents(evt->upstreamEvents, upstreamEvents);
	evt->upstreamEvents.swap(upstreamEvents);	//whatever is left is sent upstream

	//reduce dependency count in localSubtree
	localSubtree->removeNode(evt->originalSource);

	parseCondition.notify_all();	//wake up event transfer loop

	//send new message upstream?
	// auto newMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID, evt->originalSource, 
	// 						EngineSchedulerMessage::SchedulerMessageType::ParseComplete);
	
	sendMessage(evt);
}

// void EventEngine::handleParsingResults(const ParserCallbackMessage& message)	//ParserCallbackTarget interface
// {

// }


bool EventEngine::play(const ParseID& parseID, TriggerCallback& triggerCB, STI::Engine::ResultTicket& resultsOut, bool debug)
{
	if (!isState(EngineState::Parsed)) {
		return false;
	}

	//Make sure we are trying to play the shot that is currently parsed on this engine.
	if (parseID != lastParseID) {
		return false;
	}

	//Make ShotID and grab Measurement references generated by parse.
	//These will be filled with data after the shot is played.
	ShotID currentShot;
	auto newMeasurements = std::make_shared<MeasurementVector>();
	for (auto& synchEvent : synchedEvents) {
		auto& evtMeasurements = synchEvent->getMeasurements();
		for (auto& m : evtMeasurements) {
			newMeasurements->push_back(m);
		}
	}
	measurementBuffer.add(currentShot, newMeasurements);	//Add this shot to the buffer

	//Prepare events
	for (auto& synchEvent : synchedEvents) {
		synchEvent->reset();		//resets Measurement events if they've played before
		synchEvent->load();			//loads (or reloads) events if needed
	}

	////all devices that depend on this device for play
	//for (auto& dev : dependentDevices) {
	//	dev->play(parseID, dependentTriggerCB, dependentResultsOut, debug);
	//}

	if (!armTrigger(triggerCB)) {
		return false;
	}

	waitForTrigger();
	triggerCB.triggerFired();		//Callback to the device that called play to confirm that this device is playing.
									//Also, if this device is a delegated system trigger, this indicated 
									//that the rest of the system should now be triggered.

	return playDeviceEvents();
}
// server1.triggerEvent(ch(server1,slow,4), 5.0)		//trigger just server1
// mainserver.triggerEvent(ch(server1,slow,4), 5.0)		//trigger entire system

//play()
//{
//	setState(Arming);
//	setupTriggers();
//
//	//all devices that depend on this device for play
//	for (auto& dev : dependentDevices) {
//		dev->play();
//	}
//
//	waitForDependentDevices();
//
//	if (setState(WaitingForTrigger)) {
//		triggerCB->ready();
//		triggerCB->wait();
//	}
//
//}

bool EventEngine::armTrigger(TriggerCallback& triggerCB)
{
	if (!setState(EngineState::Arming)) {
		return setState(EngineState::Error);;
	}

	//arm triggers on dependent devices ( = those that declare this device as their server)
	//	waitForArmingDevices();

	if (!setState(EngineState::WaitingForTrigger)) {
		return setState(EngineState::Error);;
	}

	triggerCB.ready(localDeviceID);		//need to include jobID; or may server's callback job dependent (better!)

	return true;
}

void EventEngine::waitForTrigger() const
{
	std::unique_lock<std::mutex> triggerLock(triggerMutex);

	while (isState(EngineState::WaitingForTrigger)) {
		triggerCondition.wait(triggerLock);
	}
}

void EventEngine::trigger(STI::Device::DeviceID& target)
{
	//Triggers a specific device (allows any device to act as the system trigger)
	if (target == localDeviceID) {
		trigger();
	}
}

void EventEngine::trigger()
{
	//added 10/12/20
	if (!setState(EngineState::Playing)) {
		setState(EngineState::Error);
	}

	std::unique_lock<std::mutex> triggerLock(triggerMutex);
	triggerCondition.notify_all();			//releases waitForTrigger
}

bool EventEngine::playDeviceEvents()
{
	if (!setState(EngineState::Playing)) {
		return setState(EngineState::Error);
	}

	//launch measurements thread
	auto measurementThread = std::thread(&EventEngine::measureData, this);
	
	//time.reset();

	//play all events (at appropriate times)
	for (auto& evt : synchedEvents) {
		
		if (!isState(EngineState::Playing))
			break;

		//switch(waitUntil(evt->getTime())),  cases for play, pause, stop ?

		if (waitUntil(evt->getTime())) {	//success if not interrupted
			evt->play();
		}
	}

	measurementThread.join();	//wait for measurement collection to complete

	if (isState(EngineState::Playing)) {
		if (!setState(EngineState::Parsed)) {
			return setState(EngineState::Error);
		}
	}

	return true;	//play and collect were a success
}

bool EventEngine::waitUntil(double time)
{
	return isState(EngineState::Playing);
}


void EventEngine::pause()
{
	setState(EngineState::Paused);
}

void EventEngine::unpause(bool retrigger) 
{
	//if retrigger, require a trigger before resuming (allows hard time resume)
	if (!setState(EngineState::Playing)) {
		setState(EngineState::Error);
		stop();
	}

}

void EventEngine::measureData()
{
	for (auto& evt : synchedEvents) {
		evt->collectData();

		if (!isState(EngineState::Playing))
			break;
	}
}

void EventEngine::stop()
{
	bool success = true;

	switch (getState()) {
	case EngineState::Parsing:
		success = setState(EngineState::Idle);
		break;	
	case EngineState::Arming:
	case EngineState::WaitingForTrigger:
	case EngineState::Playing:
		success = setState(EngineState::Parsed);
		break;
	}

	stopDeviceEvents();
	trigger();			//in case WaitingForTrigger

	if (!success) {
		setState(EngineState::Error);
	}
}

void EventEngine::stopDeviceEvents()
{
	for (auto& evt : synchedEvents) {
		evt->stop();
	}
}

bool EventEngine::isState(EngineState target) const
{
	return stateMachine.isState(target);
}

STI::Engine::EngineState EventEngine::getState() const
{
	return stateMachine.getState();
}

bool EventEngine::setState(EngineState target)
{
	return stateMachine.setState(target);
}

