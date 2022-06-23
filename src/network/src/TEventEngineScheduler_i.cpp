
#include "TEventEngineScheduler_i.h"
#include <sti/engine/EventEngineScheduler.h>
#include <sti/device/Device.h>
#include <sti/device/DeviceTrace.h>
#include <sti/device/DeviceID.h>
#include "NetworkConvert.h"
#include "ORBManager.h"
#include <sti/engine/EngineJobID.h>
#include "LocalEventEngineJob.h"
#include "Convert_EventEngine.h"
#include "Convert_DeviceTrace.h"
#include "Shot.h"
#include <sti/engine/RawEvent.h>
#include "RemoteResultsCollector.h"
#include <sti/engine/ShotID.h>

#include "EventEngineDependencyTree.h"
//#include "RemoteEventEngineJob.h"
#include <sti/engine/EngineParsingMessage.h>

#include <memory>


using STI::TNetwork::TEventEngineScheduler_i;
using ::STI::TNetwork::TDeviceIDSeq;
using ::STI::TNetwork::TEventEngineDependencyTree;
using ::STI::TNetwork::TDeviceTrace;
using STI::Device::DeviceTrace;
using STI::Device::DeviceID;
using STI::Engine::EventEngineScheduler;
using ::STI::TNetwork::TEventEngineDependencyTree;
using ::STI::Engine::EventEngineDependencyTree;
using STI::Network::convert;
//using STI::Network::RemoteEventEngineJob;
using STI::Engine::EngineJobID;
using ::STI::TNetwork::TEngineJobID;
using STI::Engine::EventEngineJobType;
using STI::Engine::EventEngineJob;
using STI::Engine::Shot;
using ::STI::TNetwork::TEngineJobStatus;
using ::STI::TNetwork::TParseID;
using ::STI::TNetwork::TShotID;
using STI::Engine::ParseID;
using STI::Engine::ShotID;


TEventEngineScheduler_i::TEventEngineScheduler_i(const std::shared_ptr<STI::Device::Device>& device)
{
	if (device != 0) {
        device->getEngineScheduler(engineScheduler);        
    }
}

TEventEngineScheduler_i::~TEventEngineScheduler_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

TParseID* TEventEngineScheduler_i::parse(const ::STI::TNetwork::TShot& shot)
{
	STI::TNetwork::TParseID_var tParseID(new STI::TNetwork::TParseID);

	std::shared_ptr<Shot> parsedShot;
	bool success = convert<::STI::TNetwork::TShot, std::shared_ptr<Shot>>(shot, parsedShot);
    
	if (engineScheduler != 0) {
		auto pid = engineScheduler->parse(parsedShot);
		convert<ParseID, TParseID>(pid, tParseID.inout());
	}

	return tParseID._retn();
}


TShotID* TEventEngineScheduler_i::play(const TParseID& parseID, const TEngineJobSourceID& source)
{
	STI::TNetwork::TShotID_var tShotID(new STI::TNetwork::TShotID);

    if (engineScheduler != 0) {
		auto sid = engineScheduler->play(convert<TParseID, STI::Engine::ParseID>(parseID), 
										 convert<TEngineJobSourceID, STI::Engine::EngineJobSourceID>(source));
		convert<ShotID, TShotID>(sid, tShotID.inout());
	}

	return tShotID._retn();
}


TEngineJobStatus TEventEngineScheduler_i::getStatusPID(const ::STI::TNetwork::TParseID& pid)
{
	TEngineJobStatus tStatus;

	if (engineScheduler != 0) {
		auto status = engineScheduler->getStatus(convert<TParseID, STI::Engine::ParseID>(pid));
		convert<STI::Engine::EngineJobStatus, TEngineJobStatus>(status, tStatus);
	}
	return tStatus;
}


TEngineJobStatus TEventEngineScheduler_i::getStatusSID(const ::STI::TNetwork::TShotID& sid)
{
	TEngineJobStatus tStatus;

	if (engineScheduler != 0) {
		auto status = engineScheduler->getStatus(convert<TShotID, STI::Engine::ShotID>(sid));
		convert<STI::Engine::EngineJobStatus, TEngineJobStatus>(status, tStatus);
	}
	return tStatus;
}


void TEventEngineScheduler_i::getDependants(const TDeviceIDSeq& evtTargets, 
                        TEventEngineDependencyTree& tree, 
                        TDeviceIDSeq& missingTargets, 
						::STI::TNetwork::TEngineParsingMessageSeq_out messages, 
                        const TDeviceTrace& trace)
{
    if (engineScheduler != 0) {

		//convert in values
		STI::Engine::EventEngineDependencyTree dependencyTree;
		convert<TEventEngineDependencyTree, STI::Engine::EventEngineDependencyTree>(tree, dependencyTree);

        std::set<DeviceID> missingIDs;
        std::set<DeviceID> targetIDs;
        convert<TDeviceID, DeviceID>(missingTargets, missingIDs);
        convert<TDeviceID, DeviceID>(evtTargets, targetIDs);

		std::vector<STI::Engine::EngineParsingMessage> generatedMessages;
		//convert<::STI::TNetwork::TEngineParsingMessage, STI::Engine::EngineParsingMessage>(messages, generatedMessages);

		engineScheduler->getDependants(targetIDs, dependencyTree, missingIDs, generatedMessages, convert<TDeviceTrace, DeviceTrace>(trace));

		//convert out values
		convert<STI::Engine::EventEngineDependencyTree, TEventEngineDependencyTree>(dependencyTree, tree);
        convert<DeviceID, TDeviceID>(missingIDs, missingTargets);
		
		
		STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessageSeq_var(new STI::TNetwork::TEngineParsingMessageSeq);

		if (convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(generatedMessages,
			(_CORBA_Unbounded_Sequence<STI::TNetwork::TEngineParsingMessage>&) tEngineParsingMessageSeq_var)) {

			messages = new STI::TNetwork::TEngineParsingMessageSeq();
			(*messages) = tEngineParsingMessageSeq_var;
		}
	}
}

void TEventEngineScheduler_i::addDeviceEventTargets(TEventEngineDependencyTree& tree, 
													::STI::TNetwork::TEngineParsingMessageSeq_out messages, 
                                                    const TDeviceTrace& trace)
{
    if (engineScheduler != 0) {

		//convert in values
		STI::Engine::EventEngineDependencyTree dependencyTree;
		convert<TEventEngineDependencyTree, STI::Engine::EventEngineDependencyTree>(tree, dependencyTree);

		std::vector<STI::Engine::EngineParsingMessage> generatedMessages;

		engineScheduler->addDeviceEventTargets(dependencyTree, generatedMessages, convert<TDeviceTrace, DeviceTrace>(trace));

        //convert out values
		convert<STI::Engine::EventEngineDependencyTree, TEventEngineDependencyTree>(dependencyTree, tree);

		STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessageSeq_var(new STI::TNetwork::TEngineParsingMessageSeq);
		
		if (convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(generatedMessages,
			(_CORBA_Unbounded_Sequence<STI::TNetwork::TEngineParsingMessage>&) tEngineParsingMessageSeq_var)) {

			messages = new STI::TNetwork::TEngineParsingMessageSeq();
			(*messages) = tEngineParsingMessageSeq_var;
		}
	}
}


void TEventEngineScheduler_i::addJob(const ::STI::TNetwork::TEventEngineJob& newJob)
{
	if (engineScheduler != 0) {

		std::shared_ptr<EventEngineJob> remoteJob;

		convert<TNetwork::TEventEngineJob, std::shared_ptr<EventEngineJob>>(newJob, remoteJob);

		engineScheduler->addJob(remoteJob);
	}
}

void TEventEngineScheduler_i::cancelJob(const ::STI::TNetwork::TEngineJobID& jobID)
{
    if (engineScheduler != 0) {

		engineScheduler->cancelJob(convert<TEngineJobID, STI::Engine::EngineJobID>(jobID));
	}
}

void TEventEngineScheduler_i::cancelAll()
{
    if (engineScheduler != 0) {

		engineScheduler->cancelAll();
	}
}

void TEventEngineScheduler_i::getQueuedJobs(::STI::TNetwork::TEngineJobIDSeq_out jobIDs)
{
    if (engineScheduler != 0) {

		STI::TNetwork::TEngineJobIDSeq_var tEngineJobIDSeq_var(new STI::TNetwork::TEngineJobIDSeq);
		std::set<STI::Engine::EngineJobID> ids;

		engineScheduler->getQueuedJobs(ids);

		convert<STI::Engine::EngineJobID, ::STI::TNetwork::TEngineJobID>(ids, tEngineJobIDSeq_var);

		jobIDs = new STI::TNetwork::TEngineJobIDSeq();
		(*jobIDs) = tEngineJobIDSeq_var;
	}
}

void TEventEngineScheduler_i::getRunningJobs(::STI::TNetwork::TEngineJobIDSeq_out jobIDs)
{
    if (engineScheduler != 0) {

		STI::TNetwork::TEngineJobIDSeq_var tEngineJobIDSeq_var(new STI::TNetwork::TEngineJobIDSeq);
		std::set<STI::Engine::EngineJobID> ids;

		engineScheduler->getRunningJobs(ids);

		convert<STI::Engine::EngineJobID, ::STI::TNetwork::TEngineJobID>(ids, tEngineJobIDSeq_var);

		jobIDs = new STI::TNetwork::TEngineJobIDSeq();
		(*jobIDs) = tEngineJobIDSeq_var;
	}
}

void TEventEngineScheduler_i::getCompletedJobs(::STI::TNetwork::TEngineJobIDSeq_out jobIDs)
{
    if (engineScheduler != 0) {

		STI::TNetwork::TEngineJobIDSeq_var tEngineJobIDSeq_var(new STI::TNetwork::TEngineJobIDSeq);
		std::set<STI::Engine::EngineJobID> ids;

		engineScheduler->getCompletedJobs(ids);

		convert<STI::Engine::EngineJobID, ::STI::TNetwork::TEngineJobID>(ids, tEngineJobIDSeq_var);

		jobIDs = new STI::TNetwork::TEngineJobIDSeq();
		(*jobIDs) = tEngineJobIDSeq_var;
	}
}


::CORBA::Boolean TEventEngineScheduler_i::getParsedEvents(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TDeviceEventsSeq_out events)
{
	bool success = false;

    if (engineScheduler != 0) {

		STI::TNetwork::TDeviceEventsSeq_var tDeviceEventsSeq_var(new STI::TNetwork::TDeviceEventsSeq);
		STI::Engine::DeviceEventMap deviceEvents;

		success = engineScheduler->getParsedEvents(
			convert<TParseID, STI::Engine::ParseID>(parseID),
			deviceEvents);

		success = convert<STI::Engine::DeviceEventMap, ::STI::TNetwork::TDeviceEventsSeq>(deviceEvents, tDeviceEventsSeq_var);

		events = new STI::TNetwork::TDeviceEventsSeq();
		(*events) = tDeviceEventsSeq_var;

	}

	return success;
}

::CORBA::Boolean TEventEngineScheduler_i::getParsingMessages(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TEngineParsingMessageSeq_out messages)
{
	bool success = false;

    if (engineScheduler != 0) {

		STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessageSeq_var(new STI::TNetwork::TEngineParsingMessageSeq);
		std::vector<STI::Engine::EngineParsingMessage> generatedMessages;

		success = engineScheduler->getParsingMessages(
					convert<TParseID, STI::Engine::ParseID>(parseID),
					generatedMessages);

		success = convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(generatedMessages,
			(_CORBA_Unbounded_Sequence<STI::TNetwork::TEngineParsingMessage>&) tEngineParsingMessageSeq_var);
		
		messages = new STI::TNetwork::TEngineParsingMessageSeq();
		(*messages) = tEngineParsingMessageSeq_var;
	}

	return success;
}

::CORBA::Boolean TEventEngineScheduler_i::getParsedTree(const ::STI::TNetwork::TParseID& parseID, 
															::STI::TNetwork::TEventEngineDependencyTree_out tree)
{
	bool success = false;

    if (engineScheduler != 0) {

		STI::TNetwork::TEventEngineDependencyTree_var tEventEngineDependencyTree_var(new STI::TNetwork::TEventEngineDependencyTree);
		std::shared_ptr<STI::Engine::ParsedDependencyTree> depTree;

		success = engineScheduler->getParsedTree(
					convert<TParseID, STI::Engine::ParseID>(parseID),
					depTree);
		success &= convert<std::shared_ptr<STI::Engine::ParsedDependencyTree>, TEventEngineDependencyTree>(
					depTree, tEventEngineDependencyTree_var);
		tree = new STI::TNetwork::TEventEngineDependencyTree();
		(*tree) = tEventEngineDependencyTree_var;		
	}

	return success;
}

// ::CORBA::Boolean TEventEngineScheduler_i::transferResults(::STI::TNetwork::TResultsCollector_ptr resultsCollector)
// {
// 	bool success = false;

// 	auto remoteCollector = std::make_shared<STI::Network::RemoteResultsCollector>(resultsCollector);

// 	if (engineScheduler != 0) {
// 		success = engineScheduler->transferResults(remoteCollector);
// 	}

// 	return success;
// }

// ::CORBA::Boolean TEventEngineScheduler_i::getResults(const ::STI::TNetwork::TShotID& shotID, ::STI::TNetwork::TResultTicket_out results)
// {
// 	bool success = false;

//     if (engineScheduler != 0) {

// 		std::shared_ptr<STI::Engine::ResultTicket> resultTicket;
// 		STI::TNetwork::TResultTicket_var tResultTicket_var(new STI::TNetwork::TResultTicket);

// 		success = engineScheduler->getResults(convert<TShotID, STI::Engine::ShotID>(shotID), resultTicket);

// 		success &= convert<std::shared_ptr<STI::Engine::ResultTicket>, TResultTicket>(
// 					resultTicket, tResultTicket_var);

// 		results = new STI::TNetwork::TResultTicket();
// 		(*results) = tResultTicket_var;		
// 	}

// 	return success;
// }

::CORBA::Boolean TEventEngineScheduler_i::ping()
{
	return true;
}

