#include "LocalEventEngine.h"

#include <sti/device/AttributeManager.h>
#include <sti/device/Channel.h>
#include <sti/device/Device.h>
#include <sti/device/DeviceCollection.h>
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceMessageDispatcher.h>

#include <sti/engine/DeviceEventParser.h>
#include <sti/engine/EngineState.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/EnginePlayingMessageCount.h>
#include <sti/engine/EnginePlayingMessage.h>
#include <sti/engine/EventEngineJob.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/EngineTriggerTarget.h>
#include <sti/engine/FullShotResult.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/ParsedDependencyTree.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/ResultsCollector.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/SynchronousEvent.h>
#include <sti/engine/StackTraceResult.h>

#include "EventEngineDependencyTree.h"
#include "EventEngineParser.h"
#include "LocalEventEngineJob.h"
#include "LocalEventEngineScheduler.h"
#include "LocalPersistenceManager.h"
#include "LocalShot.h"
#include "LocalTriggerCallback.h"
#include "MasterTrigger.h"

#include <algorithm>
#include <chrono>
#include <exception>
#include <memory>
#include <thread>
#include <iterator>
#include <vector>
#include <functional>

using STI::Device::DeviceID;
using STI::Device::EngineSchedulerMessage;

using STI::Engine::DeviceEventParser;
using STI::Engine::EngineState;
using STI::Engine::LocalEventEngine;
using STI::Engine::ParseID;
using STI::Engine::RawEvent;
using STI::Engine::RawEventVector;
using STI::Engine::TriggerCallback;
using STI::Engine::DeviceEventMap;
using STI::Utils::TimeStamp;
using STI::Engine::LocalEventEngineJob;
using STI::Engine::LocalShot;
using STI::Engine::LocalTriggerCallback;
using STI::Engine::EventEngineJob;
using STI::Engine::EngineJobID;
using STI::Engine::EngineParsingMessage;
using STI::Engine::EnginePlayingMessageCount;
using STI::Engine::MasterTrigger;
using STI::Engine::ResultsCollector;
using STI::Engine::ParsedDependencyTree;
using STI::Engine::ShotResult;
using STI::Engine::ShotResultStatus;
using STI::Engine::RawEventGroup;
using STI::Engine::FullShotResult;
using STI::Engine::EnginePlayingMessage;
using STI::Engine::EventEngine;

namespace {

bool isPlayState(STI::Engine::EngineState state)
{
	return state == STI::Engine::EngineState::PreparingPlay
		|| state == STI::Engine::EngineState::PlayReady
		|| state == STI::Engine::EngineState::WaitingForTrigger
		|| state == STI::Engine::EngineState::Playing;
}

bool hasErrorMessages(const std::vector<EnginePlayingMessage>& messages)
{
	EnginePlayingMessageCount count(messages);
	return count.errorCount > 0;
}

bool mixedValueMatchesType(const STI::Utils::MixedValue& value, STI::Utils::MixedValueType expectedType)
{
	return expectedType == STI::Utils::MixedValueType::Any || value.isType(expectedType);
}

bool hasMessageNamed(const std::vector<EnginePlayingMessage>& messages, const std::string& name)
{
	for (const auto& message : messages) {
		if (message.getName() == name) {
			return true;
		}
	}
	return false;
}

ShotResultStatus shotStatusFromMessages(const std::vector<EnginePlayingMessage>& messages, bool cancelled)
{
	if (hasMessageNamed(messages, "Play canceled")) {
		return ShotResultStatus::CanceledByUser;
	}
	if (cancelled) {
		return ShotResultStatus::AbortedByError;
	}
	if (hasErrorMessages(messages)) {
		return ShotResultStatus::CompletedWithErrors;
	}
	return ShotResultStatus::Success;
}

} // namespace

// server1.triggerEvent(ch(server1,slow,4), 5.0)		//trigger just server1
// mainserver.triggerEvent(ch(server1,slow,4), 5.0)		//trigger entire system


LocalEventEngine::LocalEventEngine(const EngineID& engineID, const STI::Device::DeviceID& localID, 
								   const std::shared_ptr<STI::Device::ChannelManager>& channels,
								   const std::shared_ptr<STI::Device::AttributeManager>& attributeManager,
 								   DeviceEventParser* deviceParser, 
								   EngineTriggerTarget* triggerTarget,
								   const std::shared_ptr<STI::Device::DeviceMessageDispatcher>& dispatcher, 
 								   const std::shared_ptr<STI::Device::DeviceCollection>& collection,
								   const std::shared_ptr<STI::Device::PersistenceManager>& persistence) 
  : MessageGenerator(dispatcher),
  	engineID(engineID),
	parser(engineID, localID, channels, persistence, deviceParser), 
	deviceParser(deviceParser),
	triggerTarget(triggerTarget),
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
	engineStateMessageGrouper.stop();
	clear();
	resetPlayThread();
}

void LocalEventEngine::setPlaybackTimeouts(std::chrono::milliseconds playReadyTimeout,
	std::chrono::milliseconds triggerTimeout,
	std::chrono::milliseconds playCompleteGrace)
{
	if (playReadyTimeout.count() > 0) {
		ownedDevicePlayReadyTimeout = playReadyTimeout;
	}
	if (triggerTimeout.count() > 0) {
		ownedDeviceTriggerTimeout = triggerTimeout;
	}
	if (playCompleteGrace.count() > 0) {
		ownedDevicePlayCompleteGrace = playCompleteGrace;
	}
}

void LocalEventEngine::setMeasurementGrace(std::chrono::milliseconds maxMeasurementGrace,
	std::chrono::milliseconds pollInterval)
{
	if (maxMeasurementGrace.count() >= 0) {	//zero disables the post-grace extension
		ownedDeviceMaxMeasurementGrace = maxMeasurementGrace;
	}
	if (pollInterval.count() > 0) {
		ownedDeviceMeasurementPollInterval = pollInterval;
	}
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
	missingTargets.clear();
	missingPostProcessingTargets.clear();
	resolvedPostProcessRequests.clear();
	ownedTargets.clear();
	parsedOwnedTargets.clear();
	playReadyOwnedTargets.clear();
	playedOwnedTargets.clear();
	activeParseJob = false;
	activePlayJob = false;
	activeParseJobID = EngineJobID();
	activePlayJobID = EngineJobID();

	parsingMessages.clear();
	localPlayMessages.clear();
	localPlayMsgCounter.clearCounts();

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

void LocalEventEngine::getResultDependencyIDs(std::set<STI::Device::DeviceID>& ownedIDs)
{
	getOwnedDeviceIDs(ownedIDs);

	//Post-process targets were added to the dependency tree for routing, so a
	//directly-owned analysis device can surface in getOwnedDeviceIDs. Such a device
	//produces no measurements, so it must not become a ShotResult result-collection
	//dependency (saveShot would pull a non-existent result from it). Drop any resolved
	//post-process target that is not also a hard-timed event owner (ownedTargets); a
	//device that is both an event owner and a post-process target stays.
	for (auto& request : resolvedPostProcessRequests) {
		const DeviceID& ppID = request.target().device().deviceID();
		if (std::find(ownedTargets.begin(), ownedTargets.end(), ppID) == ownedTargets.end()) {
			ownedIDs.erase(ppID);
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

	// Check for partner server
	std::set<DeviceID> ids;
	dependencyTree->getNodes(ids);

	auto it = ids.find(DeviceID(id.getTargetServerID()));
	if (it == ids.end()) {
		// The target server for this id is not in the tree
		// if id is an eventTarget of this device, this device will act as the server.
		isPartnerServer |= deviceParser->isEventTarget(id);
	}

	return isTargetServerForDevice(id) || isPartnerServer;
}

void LocalEventEngine::divideEvents(const std::shared_ptr<RawEventGroup>& eventGroup, std::shared_ptr<RawEventGroup>& unhandledEventGroup)
{
	std::shared_ptr<RawEventGroup> handledEventGroup;

	divideEvents(eventGroup, unhandledEventGroup, handledEventGroup);
}

void LocalEventEngine::divideEvents(const std::shared_ptr<RawEventGroup>& eventGroup, std::shared_ptr<RawEventGroup>& unhandledEventGroup, std::shared_ptr<RawEventGroup>& handledEventGroup)
{
	if (eventGroup == 0) return;

	std::set<STI::Device::DeviceID> ownedIDs;
	getOwnedDeviceIDs(ownedIDs);

	divideEvents(eventGroup, "", ownedIDs, unhandledEventGroup, handledEventGroup);
}

void LocalEventEngine::divideEvents(const std::shared_ptr<RawEventGroup>& eventGroup, 
									const std::string& subgroupName, const std::set<STI::Device::DeviceID>& ownedIDs, 
									std::shared_ptr<RawEventGroup>& unhandledEventGroup, std::shared_ptr<RawEventGroup>& handledEventGroup)
{
	if (eventGroup == 0) return;

	for (auto& g : eventGroup->getSubgroups()) {
		divideEvents(g, subgroupName + "/" + g->getName(), ownedIDs, unhandledEventGroup, handledEventGroup);
	}

	auto events = eventGroup->getEvents();

	if (events == 0) return;

	RawEventTarget target;

	for (auto& evt : *events) {
		if (evt.getTarget().isAbstract()) {
			
			if (eventGroup->getConcreteTarget(evt.getTarget(), target)) {
				getTargetEventGroup( target.device().deviceID() ).addEvent(evt, subgroupName);
			}
			else {
				//abstract target
				getAbstractTargetEventGroup( evt.getTarget().device() ).addEvent(evt);
			}

			if (handledEventGroup != 0) {
				handledEventGroup->addEvent(evt, subgroupName);
			}
		}
		else {
			addEvent(evt, subgroupName, ownedIDs, unhandledEventGroup, handledEventGroup);
		}
	}
}

void LocalEventEngine::addEvent(const RawEvent& evt, const std::string& subgroupName, 
								const std::set<STI::Device::DeviceID>& ownedIDs, 
								std::shared_ptr<RawEventGroup>& unhandledEventGroup, 
								std::shared_ptr<RawEventGroup>& handledEventGroup)
{
	STI::Device::DeviceID branchID;
	auto canonicalEvent = canonicalizeEventTarget(evt);
	auto eventTargetID = canonicalEvent.target().device().deviceID();

	auto it = ownedIDs.find(eventTargetID);

	if (localDeviceID == eventTargetID || it != ownedIDs.end()) {
		//Event target is this device or is directly owned by this device
		getTargetEventGroup(eventTargetID).addEvent(canonicalEvent, subgroupName);
		
		if (handledEventGroup != 0) {
			handledEventGroup->addEvent(canonicalEvent, subgroupName);
		}
	}
	else if (dependencyTree->getBranchToTarget(localDeviceID, eventTargetID, branchID)) {
		//Event target is in the subgraph under branchID
		getTargetEventGroup(branchID).addEvent(canonicalEvent, subgroupName);
		
		if (handledEventGroup != 0) {
			handledEventGroup->addEvent(canonicalEvent, subgroupName);
		}
	}
	else if (upstreamPartnerEvents != 0) {
		//Event target not in this subgraph; these events will handled elsewhere
		unhandledEventGroup->addEvent(canonicalEvent, subgroupName);
	}
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

RawEventGroup& LocalEventEngine::getAbstractTargetEventGroup(const RawEventTargetDevice& deviceTarget)
{
	auto it = eventsByAbstractTarget.find(deviceTarget);
	if (it == eventsByAbstractTarget.end()) {
		auto res = eventsByAbstractTarget.insert( {deviceTarget, std::make_shared<RawEventGroup>(baseEventGroupName, "") } );
		it = res.first;
	}
	return *(it->second);
}

DeviceID LocalEventEngine::findCanonicalDeviceID(const STI::Device::DeviceID& deviceID) const
{
	if (deviceID == localDeviceID) {
		return localDeviceID;
	}

	if (deviceCollection != 0) {
		std::set<DeviceID> ids;
		deviceCollection->getIDs(ids);

		auto found = ids.find(deviceID);
		if (found != ids.end()) {
			return *found;
		}
	}

	if (deviceParser != 0 && deviceParser->isEventTarget(deviceID) && deviceID.getTargetServerID().empty()) {
		return DeviceID(deviceID.getName(), deviceID.getAddress(), deviceID.getModule(), localDeviceID.getID());
	}

	return deviceID;
}

RawEvent LocalEventEngine::canonicalizeEventTarget(const RawEvent& evt) const
{
	if (evt.target().device().isAbstract()) {
		return evt;
	}

	auto canonicalID = findCanonicalDeviceID(evt.target().device().deviceID());
	if (canonicalID == evt.target().device().deviceID()
		&& canonicalID.getTargetServerID() == evt.target().device().deviceID().getTargetServerID()) {
		return evt;
	}

	RawEvent canonicalEvent = evt;
	canonicalEvent.getTarget().getDevice().setTargetDeviceID(canonicalID);
	return canonicalEvent;
}

std::shared_ptr<RawEventGroup> LocalEventEngine::canonicalizeEventGroupTargets(const std::shared_ptr<RawEventGroup>& eventGroup) const
{
	if (eventGroup == 0) {
		return {};
	}

	auto canonicalGroup = std::make_shared<RawEventGroup>(eventGroup->getName(), eventGroup->getParentGroupName(), eventGroup->getStackTraceData());
	copyCanonicalEvents(*eventGroup, *canonicalGroup);
	return canonicalGroup;
}

void LocalEventEngine::copyCanonicalEvents(const RawEventGroup& source, RawEventGroup& target) const
{
	auto events = source.getEvents();
	if (events != 0) {
		for (auto& evt : *events) {
			target.addEvent(canonicalizeEventTarget(evt));
		}
	}

	for (auto& subgroup : source.getSubgroups()) {
		if (subgroup != 0) {
			auto targetSubgroup = target.group(subgroup->getName());
			copyCanonicalEvents(*subgroup, *targetSubgroup);
		}
	}
}

void LocalEventEngine::mergePartnerEvents(const DeviceEventMap& eventMap)
{
	std::set<STI::Device::DeviceID> ownedIDs;
	getOwnedDeviceIDs(ownedIDs);

	STI::Device::DeviceID branchID;

	for (auto& e : eventMap) {
		
		auto id = findCanonicalDeviceID(e.first);
		auto evtGroup = canonicalizeEventGroupTargets(e.second);
		if (evtGroup == 0) {
			continue;
		}
		auto it = ownedIDs.find(id);

		if (localDeviceID == id || it != ownedIDs.end()) {
			//Event target is this device or is directly owned by this device

			//Deep copy to report partner events
			if (handledPartnerEvents != 0) {
				handledPartnerEvents->copyEvents(*evtGroup);
			}
			
			getTargetEventGroup(id).merge(*evtGroup);
		}
		else if (dependencyTree->getBranchToTarget(localDeviceID, id, branchID)) {
			//Event target is in the subgraph under branchID

			//Deep copy to report partner events
			if (handledPartnerEvents != 0) {
				handledPartnerEvents->copyEvents(*evtGroup);
			}

			getTargetEventGroup(branchID).merge(*evtGroup);
		}
		else {
			//Event target not in this subgraph; these events will be pushed upstream
			upstreamPartnerEvents->merge(*evtGroup);
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

void LocalEventEngine::addMissingTargetsFromEvents(const std::shared_ptr<RawEventGroup>& eventGroup)
{
	if (eventGroup == 0) return;

	auto events = eventGroup->getEvents();
	if (events != 0) {
		for (auto& evt : *events) {
			if (!evt.target().isAbstract()) {
				auto id = evt.target().device().deviceID();
				if (!id.empty()) {
					missingTargets.insert(id);
				}
			}
		}
	}

	for (auto& subgroup : eventGroup->getSubgroups()) {
		addMissingTargetsFromEvents(subgroup);
	}
}

bool LocalEventEngine::isAbstractShot() const
{
	return eventsByAbstractTarget.size() > 0 || missingTargets.size() > 0;
}

void LocalEventEngine::recordAbstractShotState(STI::Engine::EventEngineJob& job)
{
	if (isJobOwner) {
		//At the owner these groups contain events that no concrete engine could
		//parse. They are the shot-specific evidence that a missing declaration is
		//actually required, including unresolved partner events returned by a child.
		addMissingTargetsFromEvents(upstreamPartnerEvents);
		addMissingTargetsFromEvents(unhandledEvents);
	}

	if (eventsByAbstractTarget.size() > 0 || missingTargets.size() > 0) {
		bool warningAlreadyReported = false;
		for (auto& message : parsingMessages) {
			if (message.getName() == "Abstract Shot") {
				warningAlreadyReported = true;
				break;
			}
		}

		if (!warningAlreadyReported) {
			auto& warning = parser.addParsingWarning("Abstract Shot")
				<< "Some event targets were not found. Parsing is abstract only; cannot be played.";

			if (missingTargets.size() > 0) {
				warning << " Missing targets: \n";
				for (auto& id : missingTargets) {
					warning << "    " << id.getID() << "\n";
				}
			}
		}
	}

	job.setMissingTargets(missingTargets);
}

void LocalEventEngine::collectPostProcessRequests(const std::shared_ptr<RawEventGroup>& eventGroup,
												  std::vector<STI::Engine::PostProcessRequest>& requests) const
{
	if (eventGroup == 0) return;

	const auto& groupRequests = eventGroup->postProcessRequests();
	requests.insert(requests.end(), groupRequests.begin(), groupRequests.end());

	for (auto& subgroup : eventGroup->getSubgroups()) {
		collectPostProcessRequests(subgroup, requests);
	}
}

bool LocalEventEngine::resolvePostProcessDevice(const RawEventTargetDevice& target, STI::Device::DeviceID& resolvedID) const
{
	//Abstract (name-only) targets are not concretized yet (binding is WIP), so they
	//never enter the dependency tree (findPostProcessTargets skips them) and cannot
	//be routed. Drop them with the non-fatal warning at the call site.
	if (target.isAbstract()) {
		return false;
	}

	//The dependency tree — built for the event targets and extended during parse with
	//the post-processing targets' owning-server chains — is the source of truth for
	//reachability and routing. A target is dispatchable iff it is the tree root (this
	//device) or reachable from the root along the ownership chain. Using the same
	//branch lookup the dispatch uses keeps resolve and dispatch consistent.
	DeviceID canonicalID = findCanonicalDeviceID(target.deviceID());

	if (canonicalID == localDeviceID) {
		resolvedID = canonicalID;
		return true;
	}
	if (dependencyTree != 0 && dependencyTree->hasBranchToTarget(localDeviceID, canonicalID)) {
		resolvedID = canonicalID;
		return true;
	}
	return false;
}

void LocalEventEngine::resolvePostProcessRequests(const std::shared_ptr<RawEventGroup>& eventGroup)
{
	//Only the job owner dispatches post-processing (it owns the play job and the
	//PlayComplete signal), so only the owner needs to resolve the side-list.
	if (!isJobOwner || eventGroup == 0) {
		return;
	}

	std::vector<STI::Engine::PostProcessRequest> rawRequests;
	collectPostProcessRequests(eventGroup, rawRequests);

	if (rawRequests.empty()) {
		return;
	}

	for (auto& request : rawRequests) {
		DeviceID resolvedID;
		if (resolvePostProcessDevice(request.target().device(), resolvedID)) {
			STI::Engine::PostProcessTarget resolvedTarget(RawEventTargetDevice(resolvedID), request.target().name());
			resolvedPostProcessRequests.emplace_back(resolvedTarget, request.options(), request.trace());
		}
		else {
			//Non-fatal: record separately and warn. Never touches missingTargets /
			//isAbstractShot(), so the shot still plays.
			missingPostProcessingTargets.insert(request.target());
		}
	}

	if (!missingPostProcessingTargets.empty()) {
		auto& warning = parser.addParsingWarning("Missing post-processing target")
			<< "Some post-processing targets were not found on the network. "
			<< "These post-processing requests are skipped; the shot still plays.";
		for (auto& target : missingPostProcessingTargets) {
			warning << "\n    " << target.device().name() << " :: " << target.name();
		}
	}
}

std::vector<STI::Engine::PostProcessRequest> LocalEventEngine::takeResolvedPostProcessRequests()
{
	std::vector<STI::Engine::PostProcessRequest> requests = std::move(resolvedPostProcessRequests);
	resolvedPostProcessRequests.clear();   //moved-from vector is valid but unspecified
	return requests;
}

void LocalEventEngine::appendMissingTargets(EnginePlayingMessage& message) const
{
	if (missingTargets.size() == 0) return;

	message << " Missing targets: \n";
	for (auto& id : missingTargets) {
		message << "    " << id.getID() << "\n";
	}
}

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

bool LocalEventEngine::getShotResult(const ShotID& shotID, std::shared_ptr<ShotResult>& shotResult) const
{
	std::shared_ptr<FullShotResult> fullShotResult;
	if (resultBuffer.get(shotID, fullShotResult) && fullShotResult != 0) {
		shotResult = fullShotResult->shotResult;
		return (shotResult != 0);
	}

	return false;
}

bool LocalEventEngine::getLastParseResult(std::shared_ptr<ParseResult>& parseResult) const
{
	std::unique_lock<std::mutex> parseLock(parseMutex);

	if (lastParseResult != 0 && lastParseResult->pid == lastParseID) {
		parseResult = lastParseResult;
		return true;
	}

	return false;
}

bool LocalEventEngine::getLastShotResult(std::shared_ptr<ShotResult>& shotResult) const
{
	std::shared_ptr<FullShotResult> fullShotResult;
	if (resultBuffer.getFirst(fullShotResult) && fullShotResult != 0) {
		shotResult = fullShotResult->shotResult;
		return (shotResult != 0);
	}

	return false;
}

std::shared_ptr<ParsedDependencyTree> LocalEventEngine::getParsedTree() const 
{
	if (lastParseResult == 0) {
		return std::shared_ptr<ParsedDependencyTree>();
	}
	return lastParseResult->parsedDevices;
}

void LocalEventEngine::parse(STI::Engine::EventEngineJob& job)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);		//parse function is not reentrant

	clear();

	isJobOwner = (job.getJobOwner() == localDeviceID);

	lastParseID = job.getJobID().pid;
	lastParseResult->pid = lastParseID;
	lastParseResult->jobOwner = job.getJobOwner();
	lastParseResult->stackTraceResult = std::make_shared<StackTraceResult>(lastParseID);
	missingTargets = job.getMissingTargetIDs();

	if (!setState(EngineState::Parsing)) {
		parser.addParsingError("Bad engine state")
			<< "Parsing aborted: The EventEngine was not able to change its state to Parsing. "
			<< "EngineState: " << print(getState());
		cancelParseJob(job);
	}

	std::shared_ptr<Shot> shot;

    if (!job.getShot(shot)) {
		//Error: no parsed shot
		parser.addParsingError("Missing shot")
			<< "Parsing aborted: The submited EventEngineJob has a null Shot. There are no events to parse.";
		cancelParseJob(job);
	}
	else {
		lastParseResult->shotConfig = shot->getShotConfig();
	}

	if (!job.getDependencies(dependencyTree)) {
		//Error: no tree
		parser.addParsingError("Missing dependency tree")
			<< "Parsing aborted: The submited EventEngineJob has a null EventEngineDependencyTree. "
			<< "Cannot proceed without the event target dependency graph.";
		cancelParseJob(job);
	}
	else {
		lastParseResult->parsedDevices = std::make_shared<ParsedDependencyTree>(dependencyTree);
	}

	std::shared_ptr<RawEventGroup> eventGroup;

	if (shot != 0) {
		shot->getRootEventGroup(eventGroup);
	}
	
	if (shot != 0 && eventGroup == 0) {
		//Error: null event group
		parser.addParsingError("Null event group")
			<< "Parsing aborted: The submited EventEngineJob has a null RawEventGroup. "
			<< "There are no events to parse.";
		cancelParseJob(job);
	}

	if (cancelled) {
		lastParseResult->messages = job.getParsingMessages();
		lastParseResult->messages.insert(lastParseResult->messages.end(), parser.getParsingMessages().begin(), parser.getParsingMessages().end());
		job.addMessages(parser.getParsingMessages());
		return; 	//not recoverable
	}

	// *** All pre-checks passed; begin parsing *** //

	// Initialize event groups

	baseEventGroupName = eventGroup->getName();
	upstreamPartnerEvents = std::make_shared<RawEventGroup>(baseEventGroupName, "");
	handledPartnerEvents = std::make_shared<RawEventGroup>(baseEventGroupName, "");
	unhandledEvents = std::make_shared<RawEventGroup>(baseEventGroupName, ""); 

	lastParseResult->baseEventGroup = eventGroup;
	lastParseResult->stackTraceResult->stackTraceData = eventGroup->getStackTraceData();

	divideEvents(eventGroup, unhandledEvents);

	// Setup trigger
	triggerDeviceID = job.getJobOwner();	//default trigger device is job owner

	// Look for delegated trigger in root event group meta data
	const auto& metaData = eventGroup->getMetaData();
	if (metaData.contains("delegatedTriggerID")) {
		STI::Utils::MixedValue delegatedTriggerValue = metaData.getMetaData("delegatedTriggerID");

		DeviceID delegatedTriggerID;
		if (delegatedTriggerValue.isType(STI::Utils::MixedValueType::String) 
			&& DeviceID::stringToDeviceID(delegatedTriggerValue.getString(), delegatedTriggerID)) {
			triggerDeviceID = delegatedTriggerID;
		}
	}

	//Get the subtree with the localDeviceID as root (Note, the graph is already known to be a DAG)
	localSubtree = std::make_shared<EventEngineDependencyTree>();
	dependencyTree->getSubtree(localDeviceID, *localSubtree);

    std::vector<DeviceID> orderedDependents;
    localSubtree->sortTree(orderedDependents);

	//Parse all devices in order, based on dependency tree.
	int dependencyCount;
	auto nextID = orderedDependents.begin();
	activeParseJobID = job.getJobID();
	activeParseJob = true;

	while (isState(EngineState::Parsing) && nextID != orderedDependents.end()) {

		std::string devName = nextID->getName();

		if (!localSubtree->getDependentNodeCount(*nextID, dependencyCount)) {
			//Error; Could not get dependency count (?)
			parser.addParsingError("Dependency count failed")
				<< "Failed to get dependency count for device '" << nextID->getID()
				<< "'. The device was not found in the dependency graph.";
			// 	<< "This should not happen and likely indicates a bug in the STI library.";
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

	EngineParsingMessageCount localMessageCount(parser.getParsingMessages());
	EngineParsingMessageCount otherMessageCount(parsingMessages);	//from owned devices

	bool errors = (localMessageCount.errorCount + otherMessageCount.errorCount) > 0;

	if (errors || !ownedDevicesParsed) {
		cancelParseJob(job);
	}
	else if (!setState(EngineState::Parsed)) {
		parser.addParsingError("Bad engine state")
			<< "Parsing aborted: The EventEngine was not able to change its state to Parsed. "
			<< "EngineState: " << print(getState());
		cancelParseJob(job);
	}

	// Collect all parsing messages
	recordAbstractShotState(job);

	//Post-processing requests are a side-list (not hard-timed): resolve them and stash
	//the resolved list on this engine. A missing target warns but never blocks play.
	resolvePostProcessRequests(eventGroup);

	parsingMessages.insert(parsingMessages.end(), parser.getParsingMessages().begin(), parser.getParsingMessages().end());
	lastParseResult->messages = job.getParsingMessages();
	lastParseResult->messages.insert(lastParseResult->messages.end(), parsingMessages.begin(), parsingMessages.end());
	job.addMessages(parsingMessages);


	if (isJobOwner) {
		// The job owner adds all handledPartnerEvents (collected from downstream) to the 
		// top level eventsByTarget list stored locally.  This should then contains all generated partner events.

		addEventsToParseResult(handledPartnerEvents);
		handledPartnerEvents->clear();

		addEventsToParseResult(upstreamPartnerEvents);
	}

	// Send message upstream indicating that this device (and all owned devices) has finished
	auto parseCompleteMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID, 
							EngineSchedulerMessage::SchedulerMessageType::ParseComplete);
	parseCompleteMessage->jobID.type = job.getJobID().type;
	parseCompleteMessage->jobID.pid = job.getJobID().pid;
	parseCompleteMessage->upstreamPartnerEvents.swap(upstreamPartnerEvents);
	parseCompleteMessage->handledEvents.swap(handledPartnerEvents);
	parseCompleteMessage->unhandledEvents.swap(unhandledEvents);
	parseCompleteMessage->messages = parsingMessages;
	parseCompleteMessage->engineState = getState();

	//pass local engine reference upstream to server
	std::shared_ptr<STI::Engine::EventEngine> jobEngine;
	job.getEngine(jobEngine);
	parseCompleteMessage->setEngine(jobEngine);

	sendMessage(parseCompleteMessage);

	activeParseJob = false;
}

void LocalEventEngine::cancelParseJob(STI::Engine::EventEngineJob& job)
{
	// stop();
	setState(EngineState::Error);
	job.markCancelled();
	cancelled = true;
	cancelParse(job.getJobID());
}

void LocalEventEngine::cancelPlayJob()
{
	std::unique_lock<std::mutex> playLock(playMutex);
	cancelled = true;
}

bool LocalEventEngine::isPlayCancelled() const
{
	std::unique_lock<std::mutex> playLock(playMutex);
	return cancelled;
}

void LocalEventEngine::cancelParse(const EngineJobID& jobID)
{
	std::shared_ptr<STI::Device::Device> device;
	std::shared_ptr<EventEngineScheduler> scheduler;

	// ownedTargets contains all deviceIDs that this device sent a parse job
	for (auto& id : ownedTargets) {
		if (deviceCollection->get(id, device) && device != 0 && device->getEngineScheduler(scheduler)) {
			scheduler->cancelJob(jobID);
		}
	}	
}

void LocalEventEngine::parseDevice(const STI::Device::DeviceID& id, STI::Engine::EventEngineJob& job)
{
	if (id == localDeviceID) {
		//Parse local
		if (!parser.parse( getTargetEventGroup(localDeviceID), synchedEvents, lastParseID )) {
			//parse failed
			setState(EngineState::Error);
		}

		mergePartnerEvents(parser.partnerEvents);

		localSubtree->removeNode(localDeviceID);
		parseCondition.notify_all();
	}
	else if (isActingServerForDevice(id)) {
		//Start parse job on remote device
		std::shared_ptr<STI::Device::Device> device;
		std::shared_ptr<EventEngineScheduler> scheduler;

		auto it = eventsByTarget.find(id);
		if (it == eventsByTarget.end() || it->second->eventsEmpty() ) {
			//no events for this device; skip this id
			localSubtree->removeNode(id);
			parseCondition.notify_all();
			return;
		}

		if (deviceCollection->get(id, device) && device != 0 
			&& device->getEngineScheduler(scheduler)) {

			std::shared_ptr<Shot> jshot;
			job.getShot(jshot);

			auto shot = scheduler->createShot(jshot->getShotConfig(), it->second);

			auto newJob = std::make_shared<LocalEventEngineJob>(job.getJobID().pid, shot, job.getJobOwner());
			newJob->setDependencies(dependencyTree);
			newJob->setMissingTargets(job.getMissingTargetIDs());

			job.attachSubjob(newJob);	//needed to keep shot reference alive

			scheduler->addJob(newJob);
			ownedTargets.push_back(id);
		}
		else {
			//Warning: Could not contact device. Parsing is abstract only; cannot be played.
			parser.addParsingWarning("Missing device")
			<< "Could not contact device '" << id.getID()
			<< "'. Parsing is abstract only and cannot be played.";
			//Preserve the concrete events that made this missing target required.
			//A non-owner returns them to the owning server in its ParseComplete
			//message, allowing the owner to record the final abstract-shot state.
			if (unhandledEvents != 0) {
				unhandledEvents->merge(*(it->second));
			}
			missingTargets.insert(id);
			localSubtree->removeNode(id);
			parseCondition.notify_all();
		}
	}
	else {
		//The localDevice is not acting as the server for this id
		//Remove id from local dependency tree

		localSubtree->removeNode(id);
		parseCondition.notify_all();	//wake up event transfer loop
	}
}

void LocalEventEngine::handleParseMessage(const std::shared_ptr<EngineSchedulerMessage>& message)
{
	std::unique_lock<std::mutex> parseLock(parseMutex);
	
	if (message == 0) {
		return;
	}

	if (!activeParseJob || !(message->jobID == activeParseJobID)) {
		return;
	}

	if (!isState(EngineState::Parsing)) {
		return;
	}

	std::shared_ptr<EventEngine> remoteEngine;
	remoteEngine = message->getEngine();

	auto it = std::find(ownedTargets.begin(), ownedTargets.end(), message->originalSourceID());

	//only add engine if it is owned by this device
	if (it != ownedTargets.end()) {
		parsedOwnedTargets[message->originalSourceID()] = message->engineState;
		
		if (remoteEngine != 0) {
			engines[remoteEngine->getDeviceID()] = remoteEngine;
		}
		else {
			//error
			parser.addParsingError("Missing engine reference")
				<< "Received a ParseComplete message with a null engine reference from "
				<< message->originalSourceID().getID() << ".";
		}
	}

	parsingMessages.insert(parsingMessages.end(), message->messages.begin(), message->messages.end());

	//Attempt to handle generated events locally, or pass upstream
	divideEvents(message->upstreamPartnerEvents, upstreamPartnerEvents, handledPartnerEvents);

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

	if (message == 0) {
		addPlayMessage(playReadyMessages, PlayingMessageType::Warning, "PlayReady message invalid")
			<< "Received a null PlayReady message.";
		return;
	}

	if (message != 0) {
		remoteEngine = message->getEngine();
	}

	if (!activePlayJob) {
		addPlayMessage(playReadyMessages, PlayingMessageType::Warning, "PlayReady message invalid")
			<< "Received a PlayReady message while no play job is active.";
		return;
	}

	if (!(message->jobID == activePlayJobID)) {
		addPlayMessage(playReadyMessages, PlayingMessageType::Warning, "PlayReady message invalid")
			<< "Received a PlayReady message for an unexpected job (expected pid="
			<< activePlayJobID.pid.print() << ", sid=" << activePlayJobID.sid.print()
			<< "; received pid=" << message->jobID.pid.print() << ", sid="
			<< message->jobID.sid.print() << ").";
		return;
	}

	if (remoteEngine == 0) {
		addPlayMessage(playReadyMessages, PlayingMessageType::Warning, "PlayReady message invalid")
			<< "Received a PlayReady message with a null engine reference from "
			<< message->originalSourceID().getID() << ".";
		return;
	}

	auto sourceID = message->originalSourceID();
	auto it = std::find(ownedTargets.begin(), ownedTargets.end(), sourceID);

	//only add engine if it is owned by this device
	if (it != ownedTargets.end()) {
		engines[remoteEngine->getDeviceID()] = remoteEngine;
		playReadyOwnedTargets[sourceID] = message->engineState;
		if (activePlayJobPtr != nullptr) {
			activePlayJobPtr->addPlayMessages(message->playMessages);
		}
	}

	playCondition.notify_all();
}

void LocalEventEngine::handlePlayCompleteMessage(const std::shared_ptr<EngineSchedulerMessage>& message)
{
	std::unique_lock<std::mutex> playLock(playMutex);

	if (message == 0) {
		std::vector<EnginePlayingMessage> warnings;
		auto& msg = addPlayMessage(warnings, PlayingMessageType::Warning, "PlayComplete message invalid");
		msg << "Received a null PlayComplete message.";
		appendPlayMessages(warnings);
		return;
	}

	if (!activePlayJob) {
		std::vector<EnginePlayingMessage> warnings;
		auto& msg = addPlayMessage(warnings, PlayingMessageType::Warning, "PlayComplete message invalid");
		msg << "Received a PlayComplete message while no play job is active.";
		appendPlayMessages(warnings);
		return;
	}

	if (!(message->jobID == activePlayJobID)) {
		std::vector<EnginePlayingMessage> warnings;
		auto& msg = addPlayMessage(warnings, PlayingMessageType::Warning, "PlayComplete message invalid");
		msg << "Received a PlayComplete message for an unexpected job (expected pid="
			<< activePlayJobID.pid.print() << ", sid=" << activePlayJobID.sid.print()
			<< "; received pid=" << message->jobID.pid.print() << ", sid="
			<< message->jobID.sid.print() << ").";
		appendPlayMessages(warnings);
		return;
	}

	auto sourceID = message->originalSourceID();
	auto it = std::find(ownedTargets.begin(), ownedTargets.end(), sourceID);

	//only add if it is owned by this device
	if (it != ownedTargets.end()) {
		playedOwnedTargets[sourceID] = message->engineState;
		if (activePlayJobPtr != nullptr) {
			activePlayJobPtr->addPlayMessages(message->playMessages);
		}
		EnginePlayingMessageCount messageCount(message->playMessages);
		if (messageCount.errorCount > 0) {
			stop();
		}
	}

	playCondition.notify_all();
}

TimeStamp LocalEventEngine::getCurrentTimeStamp()
{
	TimeStamp ts;
	return ts;
}

bool LocalEventEngine::validateOwnedTargetsReadyForPlay(std::vector<std::pair<DeviceID, EngineState>>& invalidTargets) const
{
	invalidTargets.clear();

	for (const auto& id : ownedTargets) {
		EngineState state = EngineState::Missing;
		auto engine = engines.find(id);

		if (engine != engines.end() && engine->second != nullptr) {
			state = engine->second->getState();
		}

		if (state != EngineState::Parsed) {
			invalidTargets.emplace_back(id, state);
		}
	}

	return invalidTargets.empty();
}

std::vector<DeviceID> LocalEventEngine::pendingOwnedTargets(const std::map<DeviceID, EngineState>& observedTargets) const
{
	std::vector<DeviceID> pending;

	for (const auto& id : ownedTargets) {
		if (observedTargets.find(id) == observedTargets.end()) {
			pending.push_back(id);
		}
	}

	return pending;
}

std::vector<std::pair<DeviceID, std::shared_ptr<EventEngine>>> LocalEventEngine::snapshotOwnedTargetEngines(const std::vector<DeviceID>& targets) const
{
	std::vector<std::pair<DeviceID, std::shared_ptr<EventEngine>>> snapshot;
	snapshot.reserve(targets.size());

	for (const auto& id : targets) {
		std::shared_ptr<EventEngine> engine;
		auto it = engines.find(id);
		if (it != engines.end()) {
			engine = it->second;
		}
		snapshot.emplace_back(id, engine);
	}

	return snapshot;
}

std::vector<std::pair<DeviceID, EngineState>> LocalEventEngine::queryOwnedTargetStates(const std::vector<std::pair<DeviceID, std::shared_ptr<EventEngine>>>& targets) const
{
	std::vector<std::pair<DeviceID, EngineState>> states;
	states.reserve(targets.size());

	for (const auto& target : targets) {
		EngineState state = EngineState::Missing;

		if (target.second != nullptr) {
			try {
				state = target.second->getState();
			}
			catch (const std::exception&) {
				state = EngineState::Unknown;
			}
			catch (...) {
				state = EngineState::Unknown;
			}
		}

		states.emplace_back(target.first, state);
	}

	return states;
}

EnginePlayingMessage& LocalEventEngine::addOwnedTargetTimeoutMessage(std::vector<EnginePlayingMessage>& messages, const std::string& name, const std::string& expectedState, const std::vector<std::pair<DeviceID, EngineState>>& targets)
{
	auto& msg = addPlayMessage(messages, PlayingMessageType::Error, name)
		<< "Timed out waiting for owned devices to reach " << expectedState << ": \n";

	for (const auto& target : targets) {
		msg << " * " << target.first.getID()
			<< " (EngineState = " << print(target.second) << ")\n";
	}

	return msg;
}

bool LocalEventEngine::waitForOwnedTargetsPlayReady(std::unique_lock<std::mutex>& playLock)
{
	if (ownedTargets.size() == 0) {
		return true;
	}

	const auto deadline = std::chrono::steady_clock::now() + ownedDevicePlayReadyTimeout;

	while (!cancelled
		&& isState(EngineState::PreparingPlay)
		&& playReadyOwnedTargets.size() != ownedTargets.size()) {
		if (playCondition.wait_until(playLock, deadline) == std::cv_status::timeout) {
			break;
		}
	}

	if (playReadyOwnedTargets.size() == ownedTargets.size()) {
		return true;
	}

	if (cancelled || !isState(EngineState::PreparingPlay)) {
		return false;
	}

	auto pendingTargets = pendingOwnedTargets(playReadyOwnedTargets);
	auto targetEngines = snapshotOwnedTargetEngines(pendingTargets);

	playLock.unlock();
	auto targetStates = queryOwnedTargetStates(targetEngines);
	playLock.lock();

	if (playReadyOwnedTargets.size() != ownedTargets.size()
		&& !cancelled
		&& isState(EngineState::PreparingPlay)) {
		addOwnedTargetTimeoutMessage(playReadyMessages, "Owned device PlayReady timeout", "PlayReady", targetStates);
		cancelled = true;
		return false;
	}

	return playReadyOwnedTargets.size() == ownedTargets.size();
}

void LocalEventEngine::scheduleAllPlayJobs(const EngineJobID& jobID, const std::shared_ptr<Shot>& shot, const DeviceID& jobOwner)
{
	if (ownedTargets.size() == 0) {
		return;
	}

	//Start play job on remote devices
	std::shared_ptr<STI::Device::Device> device;
    std::shared_ptr<EventEngineScheduler> scheduler;

	for (auto& id : ownedTargets) {
		
		if (deviceCollection->get(id, device) && device != 0 && device->getEngineScheduler(scheduler)) {

			auto newJob = std::make_shared<LocalEventEngineJob>(jobID, shot, jobOwner);
			scheduler->addJob(newJob);
		}
		else {
			//Error: Could not contact device to play
			addPlayMessage(playReadyMessages, PlayingMessageType::Error, "Failed to contact owned device")
				<< "Failed to contact owned device '" << id.getID() << "' to start play job.";
			playReadyOwnedTargets[id] = EngineState::Error;
		}
	}
}

EnginePlayingMessage& LocalEventEngine::addPlayMessage(std::vector<EnginePlayingMessage>& messages, const PlayingMessageType& type, const std::string& name)
{
	unsigned id = 0;
	auto it = LocalEventEngineScheduler::getPlayMessageIDs().find(name);
	if (it != LocalEventEngineScheduler::getPlayMessageIDs().end()) {
		id = it->second;
	}
	messages.emplace_back(localDeviceID, type, id, name);
	return messages.back();
}

void LocalEventEngine::play(EventEngineJob& job)
{
	EngineJobID jobID = job.getJobID();
	std::shared_ptr<Shot> shot;
	job.getShot(shot);

	std::shared_ptr<TriggerCallback> localMasterTriggerCB;
	EnginePlayingMessageCount playReadyCount;
	bool isJobOwnerLocal = false;
	std::vector<std::pair<DeviceID, EngineState>> invalidTargets;
	bool ownedTargetsReadyForPlay = validateOwnedTargetsReadyForPlay(invalidTargets);

	{
		std::unique_lock<std::mutex> playLock(playMutex);
		cancelled = false;
		playReadyMessages.clear();
		localPlayMessages.clear();
		localPlayMsgCounter.clearCounts();
		masterTriggerCB.reset();
		masterTrigger.reset();

		if (!isState(EngineState::Parsed) || lastParseID != jobID.pid) {
			//Error: this device is not parsed for this job. Should not happen because EngineScheduler should check.
			addPlayMessage(playReadyMessages, PlayingMessageType::Error, "Not Parsed")
				<< "The EventEngine cannot play the submitted job because it is not in the Parsed state for this job.";
			cancelled = true;
		}

		if (isAbstractShot()) {
			auto& message = addPlayMessage(playReadyMessages, PlayingMessageType::Error, "Cannot Play Abstract Shot")
				 << "The parsed shot is abstract and cannot be played.";
			appendMissingTargets(message);
			job.addPlayMessages(playReadyMessages);
			job.markCancelled();
			cancelled = true;
			setState(EngineState::Error);
			return;
		}

		if (!ownedTargetsReadyForPlay) {
			auto& message = addPlayMessage(playReadyMessages, PlayingMessageType::Error, "Owned device state invalid")
				<< "Cannot play because one or more owned devices are not parsed and ready for this shot: \n";

			for (auto& target : invalidTargets) {
				message << " * " << target.first.getID()
					<< " (EngineState = " << print(target.second) << ")\n";
			}
			cancelled = true;
		}

		if (!cancelled && !setState(EngineState::PreparingPlay)) {
			addPlayMessage(playReadyMessages, PlayingMessageType::Error, "Bad engine state")
				<< "[LocalEventEngine:" << localDeviceID.getID()
				<< "] play cancel: failed to enter PreparingPlay (state=" << print(getState())
				<< ", sid=" << job.getJobID().sid.print() << ")\n";
			cancelled = true;
		}

		activePlayJobID = jobID;
		activePlayJob = true;
		activePlayJobPtr = &job;

		playReadyOwnedTargets.clear();
		playedOwnedTargets.clear();

		isJobOwner = (job.getJobOwner() == localDeviceID);
		isJobOwnerLocal = isJobOwner;

		if (isJobOwner) {
			jobID.runTime = getCurrentTimeStamp();
		}

		std::shared_ptr<FullShotResult> cachedShot;
		if (!resultBuffer.get(jobID.sid, cachedShot) || cachedShot == 0) {
			std::set<STI::Device::DeviceID> ownedIDs;
			getResultDependencyIDs(ownedIDs);

			auto shotResult = std::make_shared<ShotResult>(localDeviceID, ownedIDs);
			shotResult->playTime = jobID.runTime;
			shotResult->sid = jobID.sid;
			shotResult->jobOwner = job.getJobOwner();

			auto fullShot = std::make_shared<FullShotResult>();
			fullShot->shotResult = shotResult;
			fullShot->parseResult = lastParseResult;

			resultBuffer.add(jobID.sid, fullShot);
		}

		if (!cancelled) {
			scheduleAllPlayJobs(jobID, shot, job.getJobOwner());
		}

		//Wait for all owned target devices to reach PlayReady state
		if (!cancelled && ownedTargets.size() > 0) {
			waitForOwnedTargetsPlayReady(playLock);
		}

		// Check that owned devices are PlayReady
		bool ownedDevicesPlayReady = allEngineStateCheck(playReadyOwnedTargets, EngineState::PlayReady);

		if (!cancelled && !ownedDevicesPlayReady) {
			auto& msg = addPlayMessage(playReadyMessages, PlayingMessageType::Error, "Owned devices not PlayReady")
				<< "The following devices failed to reach the PlayReady state: \n";

			for (auto& tuple : playReadyOwnedTargets) {
				if (tuple.second != EngineState::PlayReady) {
					msg	<< " * " << tuple.first.getID() << " (EngineState = " << print(tuple.second) << ")\n";
				}
			}
			cancelled = true;
		}

		if (!cancelled && !setState(EngineState::PlayReady)) {
			addPlayMessage(playReadyMessages, PlayingMessageType::Error, "Bad engine state")
				<< "[LocalEventEngine:" << localDeviceID.getID()
				<< "] play cancel: failed to enter PlayReady (state=" << print(getState())
				<< ", sid=" << job.getJobID().sid.print() << ")\n";
			setState(EngineState::Error);
			cancelled = true;
		}

		// Setup trigger
		if (!cancelled) {
			masterTrigger = std::make_shared<MasterTrigger>(triggerDeviceID);
			masterTrigger->arm(ownedTargets);

			if (isJobOwner || ownedTargets.size() > 0) {
				masterTriggerCB = std::make_shared<LocalTriggerCallback>(masterTrigger.get());
			}
		}

		localMasterTriggerCB = masterTriggerCB;

		if (!cancelled && !isState(EngineState::PlayReady)) {
			// an error occurred, or play was aborted
			addPlayMessage(playReadyMessages, PlayingMessageType::Error, "Bad engine state")
				<< "[LocalEventEngine:" << localDeviceID.getID()
				<< "] play cancel: PlayReady aborted before send (state=" << print(getState())
				<< ", sid=" << job.getJobID().sid.print() << ")\n";
			cancelled = true;
		}

		//Send PlayReady message with local engine reference
		auto playReadyMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID,
									EngineSchedulerMessage::SchedulerMessageType::PlayReady);
		playReadyMessage->jobID.pid = job.getJobID().pid;
		playReadyMessage->jobID.sid = job.getJobID().sid;
		playReadyMessage->jobID.type = job.getJobID().type;
		playReadyMessage->engineState = getState();
		playReadyMessage->playMessages = playReadyMessages;

		if (activePlayJobPtr != nullptr) {
			activePlayJobPtr->addPlayMessages(playReadyMessages);
		}

		//pass local engine reference upstream to server
		std::shared_ptr<STI::Engine::EventEngine> jobEngine;
		job.getEngine(jobEngine);
		playReadyMessage->setEngine(jobEngine);

		playReadyCount = EnginePlayingMessageCount(playReadyMessages);
		
		sendMessage(playReadyMessage);
	}

	if (playReadyCount.errorCount > 0) {
		job.markCancelled();
		cancelPlayJob();
		stop();
	}

	if (isJobOwnerLocal && playReadyCount.errorCount == 0 && localMasterTriggerCB != nullptr) {
		play(jobID, localMasterTriggerCB, false);
	}

	waitForPlayComplete();	//so job doesn't finish until play finishes or is aborted

	//save shot result
	std::shared_ptr<FullShotResult> cachedShot;
	if (resultBuffer.get(jobID.sid, cachedShot) && cachedShot != 0) {
		if (cachedShot->shotResult != 0) {
			cachedShot->shotResult->jobOwner = job.getJobOwner();
			std::vector<EnginePlayingMessage> mergedMessages;
			mergedMessages.reserve(playReadyMessages.size() + localPlayMessages.size());
			mergedMessages.insert(mergedMessages.end(), playReadyMessages.begin(), playReadyMessages.end());
			mergedMessages.insert(mergedMessages.end(), localPlayMessages.begin(), localPlayMessages.end());
			cachedShot->shotResult->messages = std::move(mergedMessages);
			cachedShot->shotResult->status = shotStatusFromMessages(cachedShot->shotResult->messages, isPlayCancelled());
		}
		
		if (persistenceManager != 0 && persistenceManager->saveShot(jobID.sid, cachedShot, isJobOwner)) {
			//successfully saved; remove from buffer
			// resultBuffer.remove(jobID.sid);
		}
	}

	//Tree-routed post-processing dispatch. The job owner's engine holds the resolved
	//side-list (parse stashed it here) and the shot result is now persisted, so deliver
	//each request along the dependency tree. Thin: each requestPostProcessing enqueues
	//and returns; the analysis runs asynchronously on the target's worker thread.
	if (isJobOwner && scheduler != nullptr && !resolvedPostProcessRequests.empty()) {
		scheduler->distributePostProcessing(takeResolvedPostProcessRequests(), dependencyTree,
											jobID.sid, job.getJobOwner());
	}

	auto playCompleteMessage = std::make_shared<EngineSchedulerMessage>(localDeviceID,
								EngineSchedulerMessage::SchedulerMessageType::PlayComplete);
	playCompleteMessage->jobID.pid = job.getJobID().pid;
	playCompleteMessage->jobID.sid = job.getJobID().sid;
	playCompleteMessage->jobID.type = job.getJobID().type;
	playCompleteMessage->engineState = getState();
	playCompleteMessage->playMessages = localPlayMessages;

	sendMessage(playCompleteMessage);

	//After play completes (without error or abort), the engine should be in the Parsed state
	if (!isState(EngineState::Parsed)) {
		job.markCancelled();
		cancelPlayJob();
	}

	{
		std::unique_lock<std::mutex> playLock(playMutex);
		activePlayJob = false;
		activePlayJobPtr = nullptr;
	}
}


void LocalEventEngine::waitForPlayComplete(std::unique_lock<std::mutex>& playLock)
{
	while (isState(EngineState::PlayReady) || isState(EngineState::PreparingPlay) ||
		   isState(EngineState::WaitingForTrigger) || isState(EngineState::Playing)) {
		playCondition.wait(playLock);
	}
	// resetPlayThread();	//calls thread::join on playThread
}

void LocalEventEngine::waitForPlayComplete()
{
	std::unique_lock<std::mutex> playLock(playMutex);
	waitForPlayComplete(playLock);
}


void LocalEventEngine::play(const EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug)
{
	localPlayMessages.clear();
	localPlayMsgCounter.clearCounts();

	if (!isState(EngineState::PlayReady)) {
		std::vector<EnginePlayingMessage> errors;
		addPlayMessage(errors, PlayingMessageType::Error, "Play called while not PlayReady")
			<< "[LocalEventEngine:" << localDeviceID.getID()
			<< "] play cancel: play() called while not PlayReady (state=" << print(getState())
			<< ", sid=" << jobID.sid.print() << ")\n";
		appendPlayMessages(errors);
		cancelPlayJob();
		stop();
		return;
	}

	//std::unique_lock<std::mutex> playLock(playMutex);

	//Make sure we are trying to play the shot that is currently parsed on this engine.
	if (jobID.pid != lastParseID) {
		std::vector<EnginePlayingMessage> errors;
		addPlayMessage(errors, PlayingMessageType::Error, "Play parse ID mismatch")
			<< "play() called with a parse ID that does not match the currently parsed shot (expected pid="
			<< lastParseID.print() << ", received pid=" << jobID.pid.print() << ").";
		appendPlayMessages(errors);
		cancelPlayJob();
		setState(EngineState::Error);
		return;
	}

	std::set<STI::Device::DeviceID> ownedIDs;
	getResultDependencyIDs(ownedIDs);
	STI::Device::DeviceID resultJobOwner = localDeviceID;
	if (activePlayJobPtr != nullptr) {
		resultJobOwner = activePlayJobPtr->getJobOwner();
	}

	std::shared_ptr<FullShotResult> cachedFullShot;
	if (!resultBuffer.get(jobID.sid, cachedFullShot) || cachedFullShot == 0) {
		auto cachedShot = std::make_shared<ShotResult>(localDeviceID, ownedIDs);
		cachedShot->playTime = jobID.runTime;
		cachedShot->sid = jobID.sid;
		cachedShot->jobOwner = resultJobOwner;

		cachedFullShot = std::make_shared<FullShotResult>();
		cachedFullShot->shotResult = cachedShot;
		cachedFullShot->parseResult = lastParseResult;

		resultBuffer.add(cachedShot->sid, cachedFullShot);	//Add this shot to the buffer
	}

	auto cachedShot = cachedFullShot->shotResult;
	cachedShot->playTime = jobID.runTime;
	cachedShot->sid = jobID.sid;
	cachedShot->jobOwner = resultJobOwner;

	//Grab Measurement references generated by parse.
	//These will be filled with data after the shot is played.
	if (cachedShot->measurements == 0) {
		cachedShot->measurements = std::make_shared<STI::Engine::MeasurementMap>();
	}
	auto& newMeasurements = (*cachedShot->measurements)[localDeviceID];
	newMeasurements.clear();
	for (auto& synchEvent : synchedEvents) {
		auto& evtMeasurements = synchEvent->getMeasurements();
		newMeasurements.insert(newMeasurements.end(), evtMeasurements.begin(), evtMeasurements.end());
	}

	if (attributeManager != 0) {
		std::map<std::string, std::string> attributes;
		attributeManager->getAttributes(attributes);

		(cachedShot->attributes)[localDeviceID] = attributes;
	}

	//Prepare local events
	for (auto& synchEvent : synchedEvents) {
		if (synchEvent == nullptr) {
			continue;
		}

		synchEvent->reset();		//resets Measurement events if they've played before
		synchEvent->load();			//loads (or reloads) events if needed
		
		if (appendPlayMessages(synchEvent->getLoadMessages())) {
			// error message found
			setState(EngineState::Error);
			cancelPlayJob();
			playCondition.notify_all();
		}
	}

	if (localPlayMsgCounter.errorCount > 0) {
		cancelPlayJob();
		stop();
		return;
	}

	if (ownedTargets.size() > 0 && masterTriggerCB != 0) {
		//masterTrigger is the local server's master trigger (works for all levels of network)
		playAll(jobID, masterTriggerCB, debug);		//all owned devices
	}
	
	//need to wait for owned device to arm before entering playShot to arm locally
	if (!waitForTriggerArm("owned devices")) {		//wait for all owned devices to enter WaitingForTrigger state
		return;
	}
	if (isPlayCancelled() || !isState(EngineState::PlayReady)) {
		return;
	}
	masterTrigger->arm(localDeviceID);	//add local device to the arming list

	resetPlayThread();
	playThread = std::thread(&LocalEventEngine::playShot, this, std::ref(*triggerCB));	//callback to calling server (not masterTrigger) 

	//Only the job owner actually calls trigger, even if the trigger is delegated to another device
	if (isJobOwner) {
		
		if (!waitForTriggerArm("local device")) {		//waits for the local device to enter WaitingForTrigger
			return;
		}

		trigger(masterTrigger->triggerID());
	}
}

void LocalEventEngine::playAll(const EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug)
{
	for (auto& engine : engines) {
		if (engine.second == 0) {
			continue;
		}
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

void LocalEventEngine::unload()
{
	std::thread unloadThread([this]() {
		// Attempt unload in background thread
		std::unique_lock<std::mutex> playLock(playMutex);

		if (getState() != EngineState::Parsed) {
			//Can only unload when in Parsed state
			return;
		}

		for (auto& evt : synchedEvents) {
			if (evt == nullptr) {
				continue;
			}
			evt->unload();
		}
	});
	unloadThread.detach();	//let it run in background
}

void LocalEventEngine::playShot(TriggerCallback& triggerCB)
{
	if (!armTrigger(triggerCB)) {
		stop();
		masterTrigger->stop();
		return;
	}

	waitForTrigger();
	
	triggerCB.triggerFired(localDeviceID);		//Callback to the device that called play to confirm that this device is playing.
												//Also, if this device is a delegated system trigger, this indicated 
												//that the rest of the system should now be triggered.

	bool playedOk = playDeviceEvents();		//actually play the events on the device
	bool ownedDevicesPlayed = waitForPlayAll();		//wait until all owned devices complete play

	if (!playedOk || !ownedDevicesPlayed || localPlayMsgCounter.errorCount > 0) {
		stop();
	}

	if (isState(EngineState::Playing)) {
		if (!setState(EngineState::Parsed)) {
			setState(EngineState::Error);
		}
	}
	
	playCondition.notify_all();		//wake up waitForPlayComplete
}

bool LocalEventEngine::armTrigger(TriggerCallback& triggerCB)
{
	if (!setState(EngineState::WaitingForTrigger)) {
		std::vector<EnginePlayingMessage> errors;
		addPlayMessage(errors, PlayingMessageType::Error, "Failed to enter WaitingForTrigger")
			<< "[LocalEventEngine:" << localDeviceID.getID()
			<< "] play cancel: failed to enter WaitingForTrigger (state=" << print(getState()) << ")\n";
		appendPlayMessages(errors);
		cancelPlayJob();
		return false;
	}
	
	triggerCB.ready(localDeviceID);		//need to include jobID; or make server's callback job dependent (better!)

	masterTrigger->ready(localDeviceID);
	
	return true;
}

bool LocalEventEngine::waitForTriggerArm(const std::string& timeoutContext)
{
	if (masterTrigger == nullptr) {
		return true;
	}

	std::vector<DeviceID> pendingTargets;
	if (masterTrigger->waitForArmFor(ownedDeviceTriggerTimeout, pendingTargets)) {
		return true;
	}

	if (pendingTargets.empty() && (isPlayCancelled() || !isState(EngineState::PlayReady))) {
		return false;
	}

	std::vector<std::pair<DeviceID, EngineState>> targetStates;
	std::vector<DeviceID> remoteTargets;

	for (const auto& id : pendingTargets) {
		if (id == localDeviceID) {
			targetStates.emplace_back(id, getState());
		}
		else {
			remoteTargets.push_back(id);
		}
	}

	std::vector<std::pair<DeviceID, std::shared_ptr<EventEngine>>> targetEngines;
	{
		std::unique_lock<std::mutex> playLock(playMutex);
		targetEngines = snapshotOwnedTargetEngines(remoteTargets);
	}

	auto remoteStates = queryOwnedTargetStates(targetEngines);
	targetStates.insert(targetStates.end(), remoteStates.begin(), remoteStates.end());

	std::vector<EnginePlayingMessage> errors;
	addOwnedTargetTimeoutMessage(errors, "Owned device trigger timeout", "WaitingForTrigger", targetStates)
		<< "Timeout context: " << timeoutContext << ".";
	appendPlayMessages(errors);
	cancelPlayJob();
	stop();
	return false;
}


void LocalEventEngine::waitForTrigger() const
{
	std::unique_lock<std::mutex> triggerLock(triggerMutex);

	while (isState(EngineState::WaitingForTrigger)) {
		triggerCondition.wait(triggerLock);
	}
}


std::chrono::nanoseconds LocalEventEngine::ownedTargetsPlayCompleteTimeout() const
{
	int64_t latestOwnedEventTime = 0;

	for (const auto& id : ownedTargets) {
		auto events = eventsByTarget.find(id);
		if (events != eventsByTarget.end() && events->second != nullptr && !events->second->eventsEmpty()) {
			latestOwnedEventTime = std::max(latestOwnedEventTime, static_cast<int64_t>(events->second->endTime()));
		}
	}

	const int64_t elapsed = engineClock.getTime();
	const int64_t remaining = std::max<int64_t>(0, latestOwnedEventTime - elapsed);

	return std::chrono::nanoseconds(remaining) + ownedDevicePlayCompleteGrace;
}

bool LocalEventEngine::waitForPlayAll()
{
	std::unique_lock<std::mutex> playLock(playMutex);

	//Wait for all owned target devices to finish play
	if (ownedTargets.size() > 0) {
		// This device is a server; wait for owned devices to send ready messages
		auto deadline = std::chrono::steady_clock::now() + ownedTargetsPlayCompleteTimeout();

		//Devices do not send PlayComplete until all measurement data is collected, which can
		//take much longer than the last event time (e.g., camera image readout and encoding).
		//After the grace deadline, keep waiting (up to maxDeadline) as long as the pending
		//devices are verifiably still Playing.
		const auto maxDeadline = deadline + ownedDeviceMaxMeasurementGrace;
		bool measurementGraceMessageSent = false;

		while (isState(EngineState::Playing) && playedOwnedTargets.size() < ownedTargets.size()) {
			if (playCondition.wait_until(playLock, deadline) != std::cv_status::timeout) {
				continue;	//woken; recheck the predicate
			}

			//Grace deadline expired. Check whether the stragglers are still actively playing
			//(i.e., collecting measurement data) before giving up.
			if (ownedDeviceMaxMeasurementGrace.count() == 0
				|| std::chrono::steady_clock::now() >= maxDeadline) {
				break;
			}

			auto pendingTargets = pendingOwnedTargets(playedOwnedTargets);
			auto targetEngines = snapshotOwnedTargetEngines(pendingTargets);

			playLock.unlock();
			auto targetStates = queryOwnedTargetStates(targetEngines);	//synchronous round trip; doubles as a liveness check
			playLock.lock();

			bool anyStillPlaying = false;
			for (const auto& targetState : targetStates) {
				if (targetState.second == EngineState::Playing) {
					anyStillPlaying = true;
					break;
				}
			}

			if (!anyStillPlaying) {
				break;	//dead, unresponsive, or errored out; handled by the timeout logic below
			}

			if (!measurementGraceMessageSent) {
				measurementGraceMessageSent = true;
				std::vector<EnginePlayingMessage> messages;
				addPlayMessage(messages, PlayingMessageType::Information, "Waiting for measurement collection")
					<< "Owned device(s) are still playing after the PlayComplete grace period;"
					<< " continuing to wait while measurement data is collected.";
				appendPlayMessages(messages);
			}

			deadline = (std::min)(std::chrono::steady_clock::now() + ownedDeviceMeasurementPollInterval, maxDeadline);
		}
	}

	// allEngineStateCheck(playedOwnedTargets, EngineState::Parsed);
	if (playedOwnedTargets.size() == ownedTargets.size()) {
		return true;
	}

	if (!isState(EngineState::Playing)) {
		return false;
	}

	auto pendingTargets = pendingOwnedTargets(playedOwnedTargets);
	auto targetEngines = snapshotOwnedTargetEngines(pendingTargets);

	playLock.unlock();
	auto targetStates = queryOwnedTargetStates(targetEngines);
	playLock.lock();

	if (playedOwnedTargets.size() != ownedTargets.size()
		&& !cancelled
		&& isState(EngineState::Playing)) {
		std::vector<EnginePlayingMessage> errors;
		addOwnedTargetTimeoutMessage(errors, "Owned device PlayComplete timeout", "PlayComplete", targetStates);
		appendPlayMessages(errors);
		cancelled = true;
		return false;
	}

	return playedOwnedTargets.size() == ownedTargets.size();
}


void LocalEventEngine::trigger(const STI::Device::DeviceID& target)
{
	//Triggers a specific device (allows any device to act as the system trigger)
	if (target == localDeviceID) {
		triggerTarget->requestTrigger(engineID, lastParseID);	// optionally waits for hardware trigger; returns when received
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
		std::vector<EnginePlayingMessage> errors;
		addPlayMessage(errors, PlayingMessageType::Error, "Failed to enter Playing")
			<< "[LocalEventEngine:" << localDeviceID.getID()
			<< "] play cancel: failed to enter Playing (state=" << print(getState()) << ")\n";
		appendPlayMessages(errors);
		setState(EngineState::Error);
		cancelled = true;
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

		if (evt == nullptr) {
			continue;
		}

		if (waitUntil(playLock, evt->getTime())) {	//success if not interrupted
			evt->waitBeforePlay();
			evt->play();
		}

		if (appendPlayMessages(evt->getPlayMessages())) {
			// error message found
			setState(EngineState::Error);
			cancelled = true;
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

	if (!isState(EngineState::Playing)) {
		//The play loop exited early (error or stop request). Any event that was never
		//played would leave the measurement thread blocked forever in waitForPlayComplete,
		//deadlocking the join below and wedging the engine in the Error state.
		//Stopping the events (idempotent) wakes the measurement thread so it can drain.
		stopDeviceEvents();
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

	for (unsigned i = 0; i < rawEvents.size(); ++i) {
		if (localChannels->getChannel(rawEvents.at(i).channel(), channel)) {
			if (channel->getType() == STI::Device::ChannelType::Input
				&& channel->getOutputType() == STI::Utils::MixedValueType::Empty) {
				continue;
			}
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

bool LocalEventEngine::appendPlayMessages(const std::vector<EnginePlayingMessage>& messages)
{
	// return true if error message found

	if (messages.size() == 0) {
		return false;
	}

	std::unique_lock<std::mutex> messageLock(playMessageMutex);

	for (const auto& message : messages) {
		if (message.getSourceID().empty()) {
			EnginePlayingMessage withSource = message;
			withSource.setSourceID(localDeviceID);
			localPlayMessages.push_back(std::move(withSource));
		}
		else {
			localPlayMessages.push_back(message);
		}
	}
	localPlayMsgCounter.appendCounts(messages);
	if (activePlayJobPtr != nullptr) {
		activePlayJobPtr->addPlayMessages(messages);
	}

	return localPlayMsgCounter.errorCount > 0;
}

bool LocalEventEngine::hasPlayMessage(const std::string& name) const
{
	std::unique_lock<std::mutex> messageLock(playMessageMutex);

	return hasMessageNamed(playReadyMessages, name) || hasMessageNamed(localPlayMessages, name);
}

void LocalEventEngine::recordPlayCanceledMessage()
{
	if (!activePlayJob || hasPlayMessage("Play canceled")) {
		return;
	}

	if (hasErrorMessages(playReadyMessages) || localPlayMsgCounter.errorCount > 0) {
		return;
	}

	std::vector<EnginePlayingMessage> messages;
	auto& msg = addPlayMessage(messages, PlayingMessageType::Error, "Play canceled");
	msg << "[LocalEventEngine:" << localDeviceID.getID()
		<< "] play canceled by stop request.";
	appendPlayMessages(messages);
}

void LocalEventEngine::measureData()
{
	for (auto& evt : synchedEvents) {
		if (evt == nullptr) {
			continue;
		}

		evt->collectData();

		for (const auto& measurement : evt->getMeasurements()) {
			if (measurement == nullptr) {
				evt->addError("Missing Measurement Result")
					<< "A synchronous event at time " << STI::Utils::printTimeFormated(evt->getTime())
					<< " contains a null Measurement.";
				continue;
			}

			if (!measurement->dataReady()) {
				evt->addError("Missing Measurement Result")
					<< "No measurement result was produced for channel #" << measurement->channel()
					<< " at time " << STI::Utils::printTimeFormated(measurement->time()) << ".";
				continue;
			}

			std::shared_ptr<STI::Device::Channel> channel;
			if (!localChannels->getChannel(static_cast<short>(measurement->channel()), channel) || channel == nullptr) {
				continue;
			}

			if (channel->getType() != STI::Device::ChannelType::Input) {
				continue;
			}

			if (mixedValueMatchesType(measurement->data(), channel->getInputType())) {
				channel->saveLastMeasurement(measurement->data());
			}
			else {
				evt->addError("Incorrect Measurement Type")
					<< "Incorrect measurement type found for channel #" << measurement->channel()
					<< ". Expected type '" << STI::Utils::MixedValue::TypeToString(channel->getInputType())
					<< "', got type '" << STI::Utils::MixedValue::TypeToString(measurement->data().getType())
					<< "'.";
			}
		}

		if (appendPlayMessages(evt->getMeasureMessages())) {
			// error message found
			setState(EngineState::Error);
			// playDeviceEvents() owns playMutex while it joins this measurement
			// thread.  Setting Error is enough to make that thread stop the shot;
			// attempting to lock playMutex here would deadlock the engine.
		}

		if (!isState(EngineState::Playing))
			break;
	}
}

void LocalEventEngine::stop()
{
	EngineState state = getState();
	if (isPlayState(state)) {
		recordPlayCanceledMessage();
	}

	if (masterTrigger != nullptr) {
		masterTrigger->stop();
	}

	switch (state) {
	case EngineState::Error:
		cancelled = true;
		releaseTriggerLock();
		releasePlayLock();
		stopDeviceEvents();
		break;
	case EngineState::Parsing:
		parser.addParsingError("Parsing Aborted")
			<< "The parsing operation was aborted.";
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
	triggerTarget->cancelTrigger();
//	std::unique_lock<std::mutex> triggerLock(triggerMutex);
	triggerCondition.notify_all();			//releases waitForTrigger
}
	

void LocalEventEngine::stopOwnedDevices()
{
	for (auto& engine : engines) {
		if (engine.second == 0) {
			continue;
		}
		engine.second->stop();
	}
}

void LocalEventEngine::stopDeviceEvents()
{
	for (auto& evt : synchedEvents) {
		if (evt == nullptr) {
			continue;
		}
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
