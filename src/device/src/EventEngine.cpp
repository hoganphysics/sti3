
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
#include "fwd/RawEvent_fwd.h"

#include "EventEngineDependencyTree.h"

#include <memory>
#include <thread>
#include <iterator>
#include <vector>
#include <functional>

#include <iostream>

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
using STI::Engine::DeviceEventMap;
using STI::Engine::TimeStamp;

EventEngine::EventEngine(const STI::Device::DeviceID& localID, STI::Device::ChannelMap& channels, DeviceEventParser* deviceParser,
const std::shared_ptr<STI::Device::DeviceEventDispatcher>& dispatcher, const std::shared_ptr<STI::Device::DeviceCollection>& collection) :
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
		if(isTargetServerForDevice(id)) {
//		if (id.getTargetServerID() == localDeviceID.getID()) {
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

bool EventEngine::isTargetServerForDevice(const STI::Device::DeviceID& id)
{
	return id.getTargetServerID() == localDeviceID.getID();
}

void EventEngine::divideEvents(const RawEventVector& events)
{
	std::set<STI::Device::DeviceID> ownedIDs;
	getOwnedDeviceIDs(ownedIDs);

	STI::Device::DeviceID branchID;

	for(auto& evt : events) {
		auto it = ownedIDs.find(evt.targetDevice());

		if (localDeviceID == evt.targetDevice() || it != ownedIDs.end()) {
			//Event target is this device or is directly owned by this device
			eventsByTarget[evt.targetDevice()].push_back(std::move(evt));
		}
		else if (dependencyTree->getBranchToTarget(localDeviceID, evt.targetDevice(), branchID)) {
			//Event target is in the subgraph under branchID
			eventsByTarget[branchID].push_back(std::move(evt));
		}
		else {
			//Event target not in this subgraph; these events will be pushed upstream
			upstreamEvents.push_back(std::move(evt));
		}
	}
}

void EventEngine::mergePartnerEvents(const DeviceEventMap& eventMap)
{
	std::set<STI::Device::DeviceID> ownedIDs;
	getOwnedDeviceIDs(ownedIDs);

	STI::Device::DeviceID branchID;

	for (auto& evtGroup : eventMap) {
		
		auto& evts = evtGroup.second;
		auto it = ownedIDs.find(evtGroup.first);

		if (localDeviceID == evtGroup.first || it != ownedIDs.end()) {
			//Event target is this device or is directly owned by this device

			//Deep copy to report partner events
			handledPartnerEvents.insert(handledPartnerEvents.begin(), evts.begin(), evts.end());

			eventsByTarget[evtGroup.first].insert(eventsByTarget[evtGroup.first].end(), 
				std::make_move_iterator(evts.begin()), std::make_move_iterator(evts.end()));
		}
		else if (dependencyTree->getBranchToTarget(localDeviceID, evtGroup.first, branchID)) {
			//Event target is in the subgraph under branchID

			//Deep copy to report partner events
			handledPartnerEvents.insert(handledPartnerEvents.begin(), evts.begin(), evts.end());

			eventsByTarget[branchID].insert(eventsByTarget[branchID].end(), 
				std::make_move_iterator(evts.begin()), std::make_move_iterator(evts.end()));
		}
		else {
			//Event target not in this subgraph; these events will be pushed upstream
			upstreamEvents.insert(upstreamEvents.end(), 
				std::make_move_iterator(evts.begin()), std::make_move_iterator(evts.end()));
		}
	}
}

//void EventEngine::parse(const ParseID& parseID, const RawEventVector& events, const STI::Device::DeviceID& server)
void EventEngine::parse(const STI::Engine::EventEngineJob& job)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);		//function is not reentrant

	if (!setState(EngineState::Parsing)) {
		return;
	}

	lastParseID = job.getJobID().pid;
	dependencyTree = job.dependencies;

	auto parsedShot = job.parsedShot;
	RawEventVector& events = parsedShot->events;
	
	//Divide event list
	eventsByTarget.clear();
	upstreamEvents.clear();
	handledPartnerEvents.clear();
	divideEvents(events);

	//Get the subtree with the localDeviceID as root (Note, the graph is already known to be a DAG)
	localSubtree = std::make_shared<EventEngineDependencyTree>();
	dependencyTree->getSubtree(localDeviceID, *localSubtree);

    std::vector<DeviceID> orderedDependents;
    localSubtree->sortTree(orderedDependents);
	ownedTargets.clear();

	//Parse all devices in order, based on dependency tree.
	int dependencyCount;
	auto nextID = orderedDependents.begin();

	while (isState(EngineState::Parsing) && nextID != orderedDependents.end()) {

		if (isTargetServerForDevice(*nextID) || (*nextID) == localDeviceID) {
			
			if (!localSubtree->getDependentNodeCount(*nextID, dependencyCount)) {
				//Error; Could not get dependency count (?)
			}

			if (dependencyCount == 0) {
				parseDevice(*nextID, job);
				++nextID;
			}
			else {
				parseCondition.wait(parseLock);
			}
		}
		else {
			++nextID;
		}
	}

	// Send message upstream indicating that this device (and all owned devices) has finished
	auto newMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID, localDeviceID, 
							EngineSchedulerMessage::SchedulerMessageType::ParseComplete);
	newMessage->jobID.pid = job.getJobID().pid;
	//newMessage->engine = job.getEngine();	//pass local engine reference upstream to server
	newMessage->unhandledEvents.swap(upstreamEvents);
	newMessage->handledEvents.swap(handledPartnerEvents);

	sendMessage(newMessage);


	//TODO: state needs to be contingent on errors from this device or owned devices.

	if (!setState(EngineState::Parsed)) {
		setState(EngineState::Error);
	}

}

void EventEngine::parseDevice(const STI::Device::DeviceID& id, const STI::Engine::EventEngineJob& job)
{
	if (id == localDeviceID) {
		//Parse local
		if (parser.parse(eventsByTarget[localDeviceID], synchedEvents)) {
			//successfully parsed
			mergePartnerEvents(parser.partnerEvents);
		}

		//temp!  Send event?  Must send event (with errors) upstream to client
		localSubtree->removeNode(localDeviceID);
		parseCondition.notify_all();
	}
	else {
		//Start parse job on remote device
	    std::shared_ptr<STI::Device::Device> device;
    	std::shared_ptr<EventEngineScheduler> scheduler;

		auto shot = std::make_shared<ParsedShot>();
		shot->events = std::move(eventsByTarget[id]);		//expensive deep copy?

		auto newJob = std::make_shared<EventEngineJob>(job.getJobID().pid, shot,
                   dependencyTree, job.jobOwner, job.missingTargetIDs);

		if (deviceCollection->get(id, device) && device != 0 
			&& device->getEngineScheduler(scheduler)) {
				
				scheduler->addJob(newJob);
				ownedTargets.push_back(id);
		}
		else {
			//Warning: Could not contact device. Parsing is abstract only; cannot be played.
		}
	}
}

void EventEngine::handleParseMessage(const std::shared_ptr<EngineSchedulerMessage>& evt)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);
	//divide events
	//RawEventVector upstreamEvents;
	//divideEvents(evt->events, upstreamEvents);		//problem: evt->evts and upstreamEvents are duplicated...
	
	//Try to handle device generated events locally
	divideEvents(evt->unhandledEvents);
	
	//Collect any handled partner events
	auto& evts = evt->handledEvents;
	handledPartnerEvents.insert(handledPartnerEvents.end(), 
								std::make_move_iterator(evts.begin()), std::make_move_iterator(evts.end()) );
	
	//evt->upstreamEvents.swap(upstreamEvents);	//whatever is left is sent upstream

	//reduce dependency count in localSubtree
	localSubtree->removeNode(evt->originalSource);

	parseCondition.notify_all();	//wake up event transfer loop

	//send new message upstream?
	// auto newMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID, evt->originalSource, 
	// 						EngineSchedulerMessage::SchedulerMessageType::ParseComplete);
	
//	sendMessage(evt);
}

void EventEngine::handlePlayMessage(const std::shared_ptr<EngineSchedulerMessage>& evt)
{
	std::unique_lock<std::mutex> playLock(playMutex);

//	auto it = std::find(ownedTargets.begin(), ownedTargets.end(), evt->sourceID());
	auto it = std::find(ownedTargets.begin(), ownedTargets.end(), evt->engine->localDeviceID);

	std::cout << "handlePlayMessage: " << evt->sourceID().getID() << " : " << evt->engine->localDeviceID.getID() << std::endl;

	//only add engine if it is owned by this device
	if (it != ownedTargets.end()) {
		
//		engines[evt->sourceID()] = evt->engine;
		engines[evt->engine->localDeviceID] = evt->engine;
	}

	if (engines.size() == ownedTargets.size()) {
		if (!setState(EngineState::PlayReady)) {
			setState(EngineState::Error);
		}
	}

	playCondition.notify_all();
}

// void EventEngine::handleParsingResults(const ParserCallbackMessage& message)	//ParserCallbackTarget interface
// {

// }

void EventEngine::preparePlayAll(const EngineJobID& jobID, const DeviceID& jobOwner)
{
	if (ownedTargets.size() == 0) {
		return;
	}

	//Start play job on remote devices
	std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<EventEngineScheduler> scheduler;

	for (auto& id : ownedTargets) {
		
		if (deviceCollection->get(id, device) && device != 0 && device->getEngineScheduler(scheduler)) {
			
				auto newJob = std::make_shared<EventEngineJob>(jobID, jobOwner);
				scheduler->addJob(newJob);
		}
		else {
			//Error: Could not contact device to play
		}
	}
}

TimeStamp EventEngine::getCurrentTimeStamp()
{
	TimeStamp ts;
	ts.timestamp = 0;
	return ts;
}

void EventEngine::play(const EventEngineJob& job)
{
	std::cout << "play(job): " << localDeviceID.getID()  << " : " << job.getEngine()->localDeviceID.getID() << std::endl;

	if (!isState(EngineState::Parsed) && lastParseID != job.getJobID().pid) {
		//Error: this device is not parsed for this job. Should not happen because EngineScheduler should check.
		//Send message upstream
		return;
	}

	std::unique_lock<std::mutex> playLock(playMutex);

	if (!setState(EngineState::PreparingPlay)) {
		//error
		return;
	}

	engines.clear();

	TimeStamp playTime;
	EngineJobID jobID = job.getJobID();
	
	isJobOwner = (job.jobOwner == localDeviceID);

	if (isJobOwner) {
		playTime = getCurrentTimeStamp();
	}
	else {
		playTime = job.getJobID().sid.playTime;
	}
	jobID.sid.playTime = playTime;

	preparePlayAll(jobID, job.jobOwner);

	if (ownedTargets.size() > 0) {
		//Wait for all owned devices (devices that this is the server for)
		while (isState(EngineState::PreparingPlay)) {
			playCondition.wait(playLock);
		}

		if (!isState(EngineState::PlayReady)) {
			return;
		}		
	}
	else {
		if (!setState(EngineState::PlayReady)) {
			setState(EngineState::Error);
		}
	}

	//Send ready message with engine reference
	auto newMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID, localDeviceID, 
							EngineSchedulerMessage::SchedulerMessageType::PlayReady);
	newMessage->jobID.pid = job.getJobID().pid;
	newMessage->jobID.sid = job.getJobID().sid;
	newMessage->jobID.type = job.getJobID().type;

	newMessage->engine = job.getEngine();	//pass local engine reference upstream to server
	std::cout << "newMessage->engine " << localDeviceID.getID()  << " : " << newMessage->engine->localDeviceID.getID() << std::endl;


	std::cout << "About to send message: " << localDeviceID.getName() << " ? " << isState(EngineState::PlayReady) << std::endl;
	if (isState(EngineState::PlayReady)) {
		sendMessage(newMessage);
	}

	STI::Device::DeviceID triggerDeviceID = job.jobOwner;	//temp!
	masterTrigger = std::make_shared<EventEngine::MasterTrigger>(this, triggerDeviceID);
	masterTrigger->arm();


	if (isJobOwner || ownedTargets.size() > 0) {
		
		masterTriggerCB = std::make_shared<TriggerCallback>(masterTrigger.get());

		if (isJobOwner) {
			play(jobID, *masterTriggerCB, false);
		}
	}


}

void EventEngine::playAll(const EngineJobID& jobID, TriggerCallback& triggerCB, bool debug)
{
	for (auto& engine : engines) {
		std::cout << "playAll engineID = " << engine.first.getID() << " : " << engine.second->localDeviceID.getID() << std::endl;
		engine.second->play(jobID, triggerCB, debug);
	}
}

void EventEngine::play(const EngineJobID& jobID, TriggerCallback& triggerCB, bool debug)
{

	std::cout << "play " << localDeviceID.getName() << std::endl;

	if (!isState(EngineState::PlayReady)) {
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
		for (auto& m : evtMeasurements) {
			newMeasurements->push_back(m);
		}
	}
	measurementBuffer.add(jobID.sid, newMeasurements);	//Add this shot to the buffer

	//Prepare local events
	for (auto& synchEvent : synchedEvents) {
		synchEvent->reset();		//resets Measurement events if they've played before
		synchEvent->load();			//loads (or reloads) events if needed
	}

	if (ownedTargets.size() > 0 && masterTriggerCB != 0) {
		//masterTrigger is the local server's master trigger (works for all levels of network)
		playAll(jobID, *masterTriggerCB, debug);		//all owned devices
	}


	// auto triggerThread = std::thread(&EventEngine::armTrigger, this, triggerCB);

	// if (!armTrigger(triggerCB)) {
	// 	return false;
	// }
	
	// if (ownedTargets.size() > 0) {

	// }
	
	if (isJobOwner || ownedTargets.size() > 0) {	//this device is a server for some devices
		
		masterTrigger->wait();
		std::cout << "masterTrigger done waiting " << localDeviceID.getName() << std::endl;
	}

	resetPlayThread();
	playThread = std::thread(&EventEngine::preplay, this, std::ref(triggerCB));	//callback to calling server (not masterTrigger) 

	// while (!isState(EngineState::WaitingForTrigger)) {
	// 	playCondition.wait(playLock);
	// }



	if (isJobOwner) {
		masterTrigger->arm(localDeviceID);
		masterTrigger->wait();		//need to wait for the local device to arm its trigger

		trigger(masterTrigger->triggerID());
	}

	// waitForTrigger();
	// triggerCB.triggerFired();		//Callback to the device that called play to confirm that this device is playing.
	// 								//Also, if this device is a delegated system trigger, this indicated 
	// 								//that the rest of the system should now be triggered.

	// return playDeviceEvents();
}

void EventEngine::resetPlayThread()
{
	if (playThread.joinable()) {
		stop();
		playThread.join();
	}
}

void EventEngine::preplay(TriggerCallback& triggerCB)
{
	if (!armTrigger(triggerCB)) {
		return;
	}

	waitForTrigger();
	
	triggerCB.triggerFired(localDeviceID);		//Callback to the device that called play to confirm that this device is playing.
									//Also, if this device is a delegated system trigger, this indicated 
									//that the rest of the system should now be triggered.

	playDeviceEvents();
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

void EventEngine::triggerOwnedDevices()
{
	std::cout << "triggerOwnedDevices " << localDeviceID.getName() << std::endl;
	for (auto& engine : engines) {
		engine.second->trigger();
	}
}

bool EventEngine::armTrigger(TriggerCallback& triggerCB)
{
	// if (!setState(EngineState::Arming)) {
	// 	return setState(EngineState::Error);;
	// }

	//arm triggers on dependent devices ( = those that declare this device as their server)
	//	waitForArmingDevices();

	if (!setState(EngineState::WaitingForTrigger)) {
		std::cout << "failed to wait in armTrigger " << localDeviceID.getName() << std::endl;
		setState(EngineState::Error);
		return false;
	}

	triggerCB.ready(localDeviceID);		//need to include jobID; or make server's callback job dependent (better!)

	return true;
}

void EventEngine::waitForTrigger() const
{
	std::unique_lock<std::mutex> triggerLock(triggerMutex);

	std::cout << "waitForTrigger " << localDeviceID.getName() << std::endl;

	while (isState(EngineState::WaitingForTrigger)) {
		triggerCondition.wait(triggerLock);
	}
}

void EventEngine::trigger(STI::Device::DeviceID& target)
{
	std::cout << "trigger(" << target.getName() << ") in " << localDeviceID.getName() << std::endl;
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

void EventEngine::trigger()
{
	std::cout << "trigger() " << localDeviceID.getName() << std::endl;

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
	case EngineState::PreparingPlay:
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


// MasterTrigger

void EventEngine::MasterTrigger::arm()
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	status.clear();
	running = false;

	for (auto& id : engine->ownedTargets) {
		status[id] = MasterTrigger::TriggerStatus::Arming;
	}
	//status[engine->localDeviceID] = MasterTrigger::TriggerStatus::Arming;
}

void EventEngine::MasterTrigger::arm(const DeviceID& id)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	status[id] = MasterTrigger::TriggerStatus::Arming;
}

void EventEngine::MasterTrigger::wait()
{
	//Wait for all devices to be in WaitingForTrigger state; device callback to this when then are ready
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);
	running = true;

	while (running && !_allStatusMatch(MasterTrigger::TriggerStatus::Waiting)) {
		mtriggerCondition.wait(mtriggerLock);
	}
}

bool EventEngine::MasterTrigger::allStatusMatch(const EventEngine::MasterTrigger::TriggerStatus& target)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);
	return _allStatusMatch(target);
}

bool EventEngine::MasterTrigger::_allStatusMatch(const EventEngine::MasterTrigger::TriggerStatus& target)
{
	bool success = true;

	for (auto& s : status) {
		if (s.second != target) {
			success = false;
			break;
		}
	}
	return success;
}

void EventEngine::MasterTrigger::stop()
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);
	
	running = false;
	mtriggerCondition.notify_all();

}

void EventEngine::MasterTrigger::ready(const STI::Device::DeviceID& id)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	auto it = status.find(id);
	if (it != status.end()) {
		it->second = MasterTrigger::TriggerStatus::Waiting;
	}
	mtriggerCondition.notify_all();
}

void EventEngine::MasterTrigger::triggerFired(const STI::Device::DeviceID& id)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	auto it = status.find(id);
	if (it != status.end()) {
		it->second = MasterTrigger::TriggerStatus::Triggered;
	}
	mtriggerCondition.notify_all();
}
