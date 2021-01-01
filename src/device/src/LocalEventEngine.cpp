

#include "LocalEventEngine.h"

//#include "Channel.h"
#include "DeviceCollection.h"
#include "DeviceID.h"
#include "DeviceEventDispatcher.h"
#include "DeviceEventParser.h"
#include "EngineState.h"
#include "EventEngineDependencyTree.h"
#include "LocalEventEngineJob.h"
#include "EventEngineParser.h"
#include "EventEngineScheduler.h"
//#include "RawEvent.h"
#include "SynchronousEvent.h"
#include "LocalTriggerCallback.h"
#include "Device.h"
#include "ParsedShot.h"
#include "LocalParsedShot.h"
#include "EventEngineJob.h"
#include "EngineJobID.h"


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
using STI::Engine::LocalParsedShot;
using STI::Engine::LocalTriggerCallback;
using STI::Engine::EventEngineJob;
using STI::Engine::EngineJobID;

// server1.triggerEvent(ch(server1,slow,4), 5.0)		//trigger just server1
// mainserver.triggerEvent(ch(server1,slow,4), 5.0)		//trigger entire system


LocalEventEngine::LocalEventEngine(const STI::Device::DeviceID& localID, const std::shared_ptr<STI::Device::ChannelManager>& channels,
 								   DeviceEventParser* deviceParser, const std::shared_ptr<STI::Device::DeviceEventDispatcher>& dispatcher, 
 								   const std::shared_ptr<STI::Device::DeviceCollection>& collection) 
  : MessageGenerator(dispatcher),
	parser(this, deviceParser), 
	rawEvents(parser.rawEvents), 
	partnerEvents(parser.partnerEvents), 
	measurementBuffer(3),
	localDeviceID(localID),
	localChannels(channels),
	deviceCollection(collection),
	cancelled(false)
{
}

LocalEventEngine::~LocalEventEngine()
{
}

void LocalEventEngine::clear()
{
	rawEvents.clear();
	synchedEvents.clear();
	partnerEvents.clear();

	eventsByTarget.clear();
	upstreamEvents.clear();
	handledPartnerEvents.clear();
	ownedTargets.clear();
	parsedOwnedTargets.clear();

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
		if(isTargetServerForDevice(id)) {
			ownedIDs.insert(id);
		}
	}
}

bool LocalEventEngine::isTargetServerForDevice(const STI::Device::DeviceID& id)
{
	return id.getTargetServerID() == localDeviceID.getID();
}

void LocalEventEngine::divideEvents(const RawEventVector& events)
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

void LocalEventEngine::mergePartnerEvents(const DeviceEventMap& eventMap)
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

void LocalEventEngine::parse(STI::Engine::EventEngineJob& job)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);		//parse function is not reentrant

	clear();

	if (!setState(EngineState::Parsing)) {
		return;
	}

	std::shared_ptr<ParsedShot> parsedShot;

    if (!job.getParsedShot(parsedShot)) {
		//Error: no parsed shot
		return;
	}
    if (!job.getDependencies(dependencyTree)) {
		//Error: no tree
		return;
	}

	lastParseID = job.getJobID().pid;
//	dependencyTree = job.dependencies;

//	auto parsedShot = job.parsedShot;
	std::shared_ptr<RawEventVector> events;
	parsedShot->getEvents(events);

	if (events != 0) {
		divideEvents(*events);
	}
	else {
		//Error: null event vector
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

		if (isTargetServerForDevice(*nextID) || (*nextID) == localDeviceID) {
			
			std::string devName = nextID->getName();

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


	//Note that ownedTargets is only modified during the previous while loop,
	//so from now on it is a static list of the owned devices of this device.

	//Wait for all owned target devices to reach Parsed state
	if (ownedTargets.size() > 0) {
		// This device is a server; wait for owned devices to send Parsed messages
		while (isState(EngineState::Parsing)) {
			
			//When an owned device finishes parsing, it sends this device a message and will be added to parsedOwnedTargets
			if (parsedOwnedTargets.size() == ownedTargets.size()) {
				if (!setState(EngineState::Parsed)) {
					setState(EngineState::Error);
				}
			}
			else {
				parseCondition.wait(parseLock);
			}
		}
	}
	else {
		// Non-server device
		if (!setState(EngineState::Parsed)) {
			setState(EngineState::Error);
			//error
		}
	}


	//TODO:  Parse errors and warnings



	// Send message upstream indicating that this device (and all owned devices) has finished
	auto newMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID, localDeviceID, 
							EngineSchedulerMessage::SchedulerMessageType::ParseComplete);
	newMessage->jobID.pid = job.getJobID().pid;
	newMessage->unhandledEvents.swap(upstreamEvents);
	newMessage->handledEvents.swap(handledPartnerEvents);

	sendMessage(newMessage);


	//TODO: state needs to be contingent on errors from this device or owned devices.


	// if (!setState(EngineState::Parsed)) {
	// 	setState(EngineState::Error);
	// 	//error
	// }
}

void LocalEventEngine::parseDevice(const STI::Device::DeviceID& id, STI::Engine::EventEngineJob& job)
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

		auto shot = std::make_shared<LocalParsedShot>();
		auto evts = std::make_shared<RawEventVector>();
		(*evts) = std::move(eventsByTarget[id]);
		//shot->events = std::move(eventsByTarget[id]);		//expensive deep copy?
		shot->setEvents(evts);

		// auto newJob = std::make_shared<LocalEventEngineJob>(job.getJobID().pid, shot,
        //            dependencyTree, job.getJobOwner(), job.getMissingTargetIDs());

		if (deviceCollection->get(id, device) && device != 0 
			&& device->getEngineScheduler(scheduler)) {
				
				//make the scheduler into a factory so remote schedulers will make remotejobs?
				auto newJob = scheduler->createJob(job.getJobID().pid, shot,
                   dependencyTree, job.getJobOwner(), job.getMissingTargetIDs());
				
				job.attachSubjob(newJob);

				scheduler->addJob(newJob);
				ownedTargets.push_back(id);
		}
		else {
			//Warning: Could not contact device. Parsing is abstract only; cannot be played.
		}
	}
}

void LocalEventEngine::handleParseMessage(const std::shared_ptr<EngineSchedulerMessage>& evt)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);
	
	if (evt == 0) {
		return;
	}

	//Try to handle device generated events locally
	divideEvents(evt->unhandledEvents);
	
	//Collect any handled partner events
	auto& evts = evt->handledEvents;
	handledPartnerEvents.insert(handledPartnerEvents.end(), 
								std::make_move_iterator(evts.begin()), 
								std::make_move_iterator(evts.end()) );
	
	//reduce dependency count in localSubtree
	localSubtree->removeNode(evt->originalSource);

	auto it = std::find(ownedTargets.begin(), ownedTargets.end(), evt->originalSource);
	
	//only add engine if it is owned by this device
	if (it != ownedTargets.end()) {
		parsedOwnedTargets.push_back(evt->originalSource);
	}

	parseCondition.notify_all();	//wake up event transfer loop
}

void LocalEventEngine::handlePlayMessage(const std::shared_ptr<EngineSchedulerMessage>& evt)
{
	std::unique_lock<std::mutex> playLock(playMutex);

	std::shared_ptr<EventEngine> remoteEngine;

	if (evt != 0) {
		remoteEngine = evt->engine;
	}

	if (remoteEngine == 0) {
		//error
		return;
	}

	auto it = std::find(ownedTargets.begin(), ownedTargets.end(), remoteEngine->getDeviceID());

	//only add engine if it is owned by this device
	if (it != ownedTargets.end()) {
		engines[remoteEngine->getDeviceID()] = remoteEngine;
	}

	playCondition.notify_all();
}

void LocalEventEngine::preparePlayAll(const EngineJobID& jobID, const DeviceID& jobOwner)
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

TimeStamp LocalEventEngine::getCurrentTimeStamp()
{
	TimeStamp ts;
	ts.timestamp = 0;
	return ts;
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

	if (!setState(EngineState::PreparingPlay)) {
		//error
		cancelled = true;
		return;
	}

	engines.clear();

	TimeStamp playTime;
	EngineJobID jobID = job.getJobID();
	
	isJobOwner = (job.getJobOwner() == localDeviceID);

	if (isJobOwner) {
		playTime = getCurrentTimeStamp();
	}
	else {
		playTime = job.getJobID().sid.playTime;
	}
	jobID.sid.playTime = playTime;

	preparePlayAll(jobID, job.getJobOwner());

	//Wait for all owned target devices to reach PlayReady state
	if (ownedTargets.size() > 0) {
		// This device is a server; wait for owned devices to send ready messages
		while (isState(EngineState::PreparingPlay)) {

			//When an owned device is in PlayReady, it will send its engine to this device
			if (engines.size() == ownedTargets.size()) {
				if (!setState(EngineState::PlayReady)) {
					setState(EngineState::Error);
				}
			}
			else {
				playCondition.wait(playLock);
			}
		}
	}
	else {
		// Non-server device
		if (!setState(EngineState::PlayReady)) {
			setState(EngineState::Error);
		}
	}

	if (!isState(EngineState::PlayReady)) {
		// an error occurred, or parse was aborted
		return;
	}

	//Send PlayReady message with local engine reference
	auto newMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID, localDeviceID, 
								EngineSchedulerMessage::SchedulerMessageType::PlayReady);
	newMessage->jobID.pid = job.getJobID().pid;
	newMessage->jobID.sid = job.getJobID().sid;
	newMessage->jobID.type = job.getJobID().type;
	job.getEngine( newMessage->engine );	//pass local engine reference upstream to server

	if (isState(EngineState::PlayReady)) {
		sendMessage(newMessage);
	}

	// Setup trigger

	STI::Device::DeviceID triggerDeviceID = job.getJobOwner();	//temp!
	masterTrigger = std::make_shared<LocalEventEngine::MasterTrigger>(this, triggerDeviceID);
	masterTrigger->arm();

	if (isJobOwner || ownedTargets.size() > 0) {
		
		masterTriggerCB = std::make_shared<LocalTriggerCallback>(masterTrigger.get());

		if (isJobOwner) {
			play(jobID, masterTriggerCB, false);
		}
	}

	waitForPlayComplete(playLock);	//so job doesn't finish until play finishes or is aborted

	//After play completes (without error or abort), the engine should be in the Parsed state
	if (!isState(EngineState::Parsed)) {
		job.markCancelled();
		cancelled = true;
	}
}

void LocalEventEngine::waitForPlayComplete(std::unique_lock<std::mutex>& playLock)
{
	while (isState(EngineState::PlayReady) || isState(EngineState::PreparingPlay) ||
		   isState(EngineState::WaitingForTrigger) || isState(EngineState::Playing)) {
		playCondition.wait(playLock);
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
		playAll(jobID, masterTriggerCB, debug);		//all owned devices
	}

	if (isJobOwner || ownedTargets.size() > 0) {	//this device is a server for some devices
		
		masterTrigger->wait();
	}

	resetPlayThread();
	playThread = std::thread(&LocalEventEngine::preplay, this, std::ref(*triggerCB));	//callback to calling server (not masterTrigger) 

	if (isJobOwner) {
		masterTrigger->arm(localDeviceID);
		masterTrigger->wait();		//need to wait for the local device to arm its trigger

		trigger(masterTrigger->triggerID());
	}
}

void LocalEventEngine::playAll(const EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug)
{
	for (auto& engine : engines) {
		engine.second->play(jobID, triggerCB, debug);
	}
}

void LocalEventEngine::preplay(TriggerCallback& triggerCB)
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

bool LocalEventEngine::armTrigger(TriggerCallback& triggerCB)
{
	if (!setState(EngineState::WaitingForTrigger)) {
		setState(EngineState::Error);
		return false;
	}

	triggerCB.ready(localDeviceID);		//need to include jobID; or make server's callback job dependent (better!)

	return true;
}

void LocalEventEngine::waitForTrigger() const
{
	std::unique_lock<std::mutex> triggerLock(triggerMutex);

	while (isState(EngineState::WaitingForTrigger)) {
		triggerCondition.wait(triggerLock);
	}
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

	bool success = true;
	if (isState(EngineState::Playing)) {
		if (!setState(EngineState::Parsed)) {
			setState(EngineState::Error);
			success = false;
		}
	}
	playCondition.notify_all();

	return true;	//play and collect were a success, or where cleanly stopped
}

bool LocalEventEngine::waitUntil(double time)
{
	return isState(EngineState::Playing);
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
	bool success = true;

	switch (getState()) {
	case EngineState::Parsing:
		success = setState(EngineState::Idle, EngineState::Error);
		cancelled = true;
		releaseParseLock();
		break;	
	case EngineState::PreparingPlay:
		success = setState(EngineState::Parsed, EngineState::Error);
		cancelled = true;
		releasePlayLock();
		break;
	case EngineState::PlayReady:
		success = setState(EngineState::Parsed, EngineState::Error);
		cancelled = true;
		releasePlayLock();
		break;
	case EngineState::WaitingForTrigger:
		success = setState(EngineState::Parsed, EngineState::Error);
		cancelled = true;
		//trigger();			//in case WaitingForTrigger
		releaseTriggerLock();
		releasePlayLock();
		break;
	case EngineState::Playing:
		success = setState(EngineState::Parsed, EngineState::Error);
		cancelled = true;
		releasePlayLock();
		stopDeviceEvents();
		break;
	}
	
	stopOwnedDevices();

//	if (!success) {
//		setState(EngineState::Error);
//	}
}



void LocalEventEngine::releaseParseLock()
{
	std::unique_lock<std::mutex> parseLock(parseMutex);
	parseCondition.notify_all();
}

void LocalEventEngine::releasePlayLock()
{
	std::unique_lock<std::mutex> playLock(playMutex);
	playCondition.notify_all();		//release play(job)
}

void LocalEventEngine::releaseTriggerLock()
{
	std::unique_lock<std::mutex> triggerLock(triggerMutex);
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
	return stateMachine.setState(target);
}


// MasterTrigger

void LocalEventEngine::MasterTrigger::arm()
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	status.clear();
	running = false;

	for (auto& id : engine->ownedTargets) {
		status[id] = MasterTrigger::TriggerStatus::Arming;
	}
	//status[engine->localDeviceID] = MasterTrigger::TriggerStatus::Arming;
}

void LocalEventEngine::MasterTrigger::arm(const DeviceID& id)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	status[id] = MasterTrigger::TriggerStatus::Arming;
}

void LocalEventEngine::MasterTrigger::wait()
{
	//Wait for all devices to be in WaitingForTrigger state; device callback to this when then are ready
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);
	running = true;

	while (running && !_allStatusMatch(MasterTrigger::TriggerStatus::Waiting)) {
		mtriggerCondition.wait(mtriggerLock);
	}
}

bool LocalEventEngine::MasterTrigger::allStatusMatch(const LocalEventEngine::MasterTrigger::TriggerStatus& target)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);
	return _allStatusMatch(target);
}

bool LocalEventEngine::MasterTrigger::_allStatusMatch(const LocalEventEngine::MasterTrigger::TriggerStatus& target)
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

void LocalEventEngine::MasterTrigger::stop()
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);
	
	running = false;
	mtriggerCondition.notify_all();

}

void LocalEventEngine::MasterTrigger::ready(const STI::Device::DeviceID& id)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	auto it = status.find(id);
	if (it != status.end()) {
		it->second = MasterTrigger::TriggerStatus::Waiting;
	}
	mtriggerCondition.notify_all();
}

void LocalEventEngine::MasterTrigger::triggerFired(const STI::Device::DeviceID& id)
{
	std::unique_lock<std::mutex> mtriggerLock(mtriggerMutex);

	auto it = status.find(id);
	if (it != status.end()) {
		it->second = MasterTrigger::TriggerStatus::Triggered;
	}
	mtriggerCondition.notify_all();
}
