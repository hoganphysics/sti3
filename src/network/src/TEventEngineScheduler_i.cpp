#include "TEventEngineScheduler_i.h"

#include <sti/device/Device.h>
#include <sti/device/DeviceID.h>
#include <sti/device/DeviceTrace.h>

#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EventEngineDependencyParser.h>
#include <sti/engine/EventEngineScheduler.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/ShotResult.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/Shot.h>
#include <sti/engine/PostProcessRequest.h>

#include "convert/Convert_ShotResult.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_DeviceTrace.h"
#include "convert/Convert_SequenceResult.h"
#include "convert/Convert_DeviceMessage.h"
#include "convert/Convert_RawEventGroup.h"

#include "EventEngineDependencyTree.h"
#include "LocalEventEngineJob.h"
#include "NetworkConvert.h"
#include "RemoteResultsCollector.h"

#include <memory>


using STI::TNetwork::TEventEngineScheduler_i;
using ::STI::TNetwork::TDeviceIDSeq;
using ::STI::TNetwork::TEventEngineDependencyTree;
using ::STI::TNetwork::TDeviceTrace;
using STI::Device::DeviceTrace;
using STI::Device::DeviceID;
using STI::Engine::EventEngineScheduler;
using STI::Network::convert;
using STI::Engine::EngineJobID;
using ::STI::TNetwork::TEngineJobID;
using STI::Engine::EventEngineJobType;
using STI::Engine::EventEngineJob;
using STI::Engine::Shot;
using ::STI::TNetwork::TEngineJobStatus;
using ::STI::TNetwork::TParseID;
using ::STI::TNetwork::TShotID;
using ::STI::TNetwork::TDeviceID;
using ::STI::TNetwork::TPostProcessRequest;
using ::STI::TNetwork::TPostProcessRequestSeq;
using STI::Engine::ParseID;
using STI::Engine::ShotID;
using STI::Engine::PostProcessRequest;
using STI::Engine::EventEngineDependencyTree;
using ::STI::TNetwork::TEngineJobIDSeq;
using ::STI::TNetwork::TEventEngineJobSeq;
using ::STI::TNetwork::TEngineJobID;
using ::STI::TNetwork::TEventEngineJob;
using STI::Engine::ParseResult;
using ::STI::TNetwork::TEventEngineDependencyParser_ptr;
using STI::Engine::EventEngineDependencyParser;
using ::STI::TNetwork::TSequence;
using STI::Engine::Sequence;
using ::STI::TNetwork::TSequenceID;
using STI::Engine::SequenceID;
using ::STI::TNetwork::TShot;
using STI::Engine::Shot;
using ::STI::TNetwork::TSequenceEntryID;
using STI::Engine::SequenceEntryID;
using ::STI::TNetwork::TEngineJobSourceID;
using STI::Engine::EngineJobSourceID;
using STI::Engine::ParseJobStatus;
using STI::TNetwork::TParseJobStatus;
using STI::Engine::PlayJobStatus;
using STI::TNetwork::TPlayJobStatus;
using STI::Engine::AddSequenceStatus;
using STI::TNetwork::TAddSequenceStatus;
using STI::TNetwork::TEngineState;
using STI::TNetwork::TEngineID;


TEventEngineScheduler_i::TEventEngineScheduler_i(const std::shared_ptr<STI::Device::Device>& device)
{
	std::shared_ptr<EventEngineDependencyParser> dependencyParser;

	if (device != 0 
		&& device->getEngineScheduler(engineScheduler) 
		&& engineScheduler->getDependencyParser(dependencyParser)) {
	
		dependencyParserServantHolder.emplace(dependencyParser);
    }
}

TEventEngineScheduler_i::~TEventEngineScheduler_i()
{
}


TParseJobStatus* TEventEngineScheduler_i::parse(const ::STI::TNetwork::TShot& shot)
{
	STI::TNetwork::TParseJobStatus_var tParseJobStatus(new STI::TNetwork::TParseJobStatus);

	std::shared_ptr<Shot> parsedShot;
	bool success = convert<::STI::TNetwork::TShot, std::shared_ptr<Shot>>(shot, parsedShot);
    
	if (engineScheduler != 0) {
		auto parseJobStatus = engineScheduler->parse(parsedShot);
		convert<ParseJobStatus, TParseJobStatus>(parseJobStatus, tParseJobStatus.inout());
	}

	return tParseJobStatus._retn();
}

TPlayJobStatus* TEventEngineScheduler_i::play(const TParseID& parseID, const TEngineJobSourceID& source)
{
	STI::TNetwork::TPlayJobStatus_var tPlayJobStatus(new STI::TNetwork::TPlayJobStatus);

    if (engineScheduler != 0) {
		auto playJobStatus = engineScheduler->play(convert<TParseID, STI::Engine::ParseID>(parseID), 
										 		   convert<TEngineJobSourceID, STI::Engine::EngineJobSourceID>(source));
		convert<PlayJobStatus, TPlayJobStatus>(playJobStatus, tPlayJobStatus.inout());
	}

	return tPlayJobStatus._retn();
}

TAddSequenceStatus* TEventEngineScheduler_i::addSequence(const TSequence& tSequenceData, const TEngineJobSourceID& source)
{
	STI::TNetwork::TAddSequenceStatus_var tAddSequenceStatus(new STI::TNetwork::TAddSequenceStatus);

	std::shared_ptr<Sequence> sequenceData;
	bool success = convert<TSequence, std::shared_ptr<Sequence>>(tSequenceData, sequenceData);
    
	if (engineScheduler != 0) {
		auto addSequenceStatus = engineScheduler->addSequence(sequenceData, 
												  convert<TEngineJobSourceID, EngineJobSourceID>(source));
		convert<AddSequenceStatus, TAddSequenceStatus>(addSequenceStatus, tAddSequenceStatus.inout());
	}

	return tAddSequenceStatus._retn();
}

void TEventEngineScheduler_i::closeSequence(const ::STI::TNetwork::TSequenceID& seqid)
{
    if (engineScheduler != 0) {
		engineScheduler->closeSequence(convert<TSequenceID, STI::Engine::SequenceID>(seqid));
	}
}


void TEventEngineScheduler_i::cancelSequence(const ::STI::TNetwork::TSequenceID& seqid)
{
    if (engineScheduler != 0) {
		engineScheduler->cancelSequence(convert<TSequenceID, STI::Engine::SequenceID>(seqid));
	}
}


TParseJobStatus* TEventEngineScheduler_i::parseSeqEntry(const TShot& shot, const TSequenceEntryID& sequenceEntryID)
{
	STI::TNetwork::TParseJobStatus_var tParseJobStatus(new STI::TNetwork::TParseJobStatus);

	std::shared_ptr<Shot> parsedShot;
	bool success = convert<::STI::TNetwork::TShot, std::shared_ptr<Shot>>(shot, parsedShot);
    
	if (engineScheduler != 0) {
		auto parseJobStatus = engineScheduler->parse(parsedShot, 
										  convert<TSequenceEntryID, SequenceEntryID>(sequenceEntryID));
		convert<ParseJobStatus, TParseJobStatus>(parseJobStatus, tParseJobStatus.inout());
	}

	return tParseJobStatus._retn();
}

TParseJobStatus* TEventEngineScheduler_i::parseSeq(const TShot& shot, const ::STI::TNetwork::TSequenceID& sequenceID)
{
	STI::TNetwork::TParseJobStatus_var tParseJobStatus(new STI::TNetwork::TParseJobStatus);

	std::shared_ptr<Shot> parsedShot;
	bool success = convert<::STI::TNetwork::TShot, std::shared_ptr<Shot>>(shot, parsedShot);
    
	if (engineScheduler != 0) {
		auto parseJobStatus = engineScheduler->parse(parsedShot, 
										  convert<TSequenceID, SequenceID>(sequenceID));
		convert<ParseJobStatus, TParseJobStatus>(parseJobStatus, tParseJobStatus.inout());
	}

	return tParseJobStatus._retn();
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

TEngineJobStatus TEventEngineScheduler_i::getStatusSeqID(const ::STI::TNetwork::TSequenceID& seqID)
{
	TEngineJobStatus tStatus;

	if (engineScheduler != 0) {
		auto status = engineScheduler->getStatus(convert<TSequenceID, STI::Engine::SequenceID>(seqID));
		convert<STI::Engine::EngineJobStatus, TEngineJobStatus>(status, tStatus);
	}
	return tStatus;
}

TEventEngineDependencyParser_ptr TEventEngineScheduler_i::getDependencyParser()
{
	return dependencyParserServantHolder.getRefPtr();
}

::CORBA::Boolean TEventEngineScheduler_i::getJob(const ::STI::TNetwork::TEngineJobID& id, ::STI::TNetwork::TEventEngineJob_out job)
{
	bool success = false;
	job = new STI::TNetwork::TEventEngineJob();

	if (engineScheduler != 0) {

		std::shared_ptr<EventEngineJob> localJob;

		if (engineScheduler->getJob(convert<TEngineJobID, STI::Engine::EngineJobID>(id), localJob) && localJob != 0) {

			STI::TNetwork::TEventEngineJob_var tJob_var(new STI::TNetwork::TEventEngineJob);
			convert<std::shared_ptr<EventEngineJob>, TNetwork::TEventEngineJob>(localJob, tJob_var);

			(*job) = tJob_var;
			success = true;
		}
	}
	return success;
}

void TEventEngineScheduler_i::addJob(const ::STI::TNetwork::TEventEngineJob& newJob)
{
	if (engineScheduler != 0) {

		std::shared_ptr<EventEngineJob> remoteJob;

		convert<TNetwork::TEventEngineJob, std::shared_ptr<EventEngineJob>>(newJob, remoteJob);

		engineScheduler->addJob(remoteJob);
	}
}

void TEventEngineScheduler_i::distributePostProcessing(const TPostProcessRequestSeq& requests, const TEventEngineDependencyTree& tree,
													   const TShotID& shotID, const TDeviceID& jobOwnerID)
{
	if (engineScheduler != 0) {

		std::vector<PostProcessRequest> localRequests;
		convert<TPostProcessRequest, PostProcessRequest>(requests, localRequests);

		auto localTree = std::make_shared<EventEngineDependencyTree>();
		convert<TEventEngineDependencyTree, EventEngineDependencyTree>(tree, *localTree);

		engineScheduler->distributePostProcessing(localRequests, localTree,
			convert<TShotID, ShotID>(shotID), convert<TDeviceID, DeviceID>(jobOwnerID));
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

TEngineJobIDSeq* TEventEngineScheduler_i::getJobIDs(::STI::TNetwork::TEventEngineJobList jobListType)
{
	using STI::Engine::EventEngineJobList;
	using STI::TNetwork::TEventEngineJobList;

	STI::TNetwork::TEngineJobIDSeq_var tJobIDs(new STI::TNetwork::TEngineJobIDSeq);

    if (engineScheduler != 0) {

		std::set<STI::Engine::EngineJobID> ids;

		ids = engineScheduler->getJobIDs( convert<TEventEngineJobList, EventEngineJobList>(jobListType) );

		convert<STI::Engine::EngineJobID, ::STI::TNetwork::TEngineJobID>(ids, tJobIDs);
	}

	return tJobIDs._retn();
}


TEventEngineJobSeq* TEventEngineScheduler_i::getJobs(::STI::TNetwork::TEventEngineJobList jobListType)
{
	using STI::Engine::EventEngineJobList;
	using STI::TNetwork::TEventEngineJobList;

	STI::TNetwork::TEventEngineJobSeq_var tJobs(new STI::TNetwork::TEventEngineJobSeq);

    if (engineScheduler != 0) {

		std::vector<std::shared_ptr<STI::Engine::EventEngineJob>> jobs;

		jobs = engineScheduler->getJobs( convert<TEventEngineJobList, EventEngineJobList>(jobListType) );

		convert<std::shared_ptr<STI::Engine::EventEngineJob>, ::STI::TNetwork::TEventEngineJob>(jobs, tJobs);
	}

	return tJobs._retn();
}


void TEventEngineScheduler_i::getEngineIDs(::STI::TNetwork::TEngineIDSeq_out engineIDs)
{
	std::set<STI::Engine::EngineID> ids;
	engineIDs = new STI::TNetwork::TEngineIDSeq();

	if (engineScheduler != 0) {
		engineScheduler->getEngineIDs(ids);

		STI::TNetwork::TEngineIDSeq_var tEngineIDseq_var(new STI::TNetwork::TEngineIDSeq);

		convert<STI::Engine::EngineID, STI::TNetwork::TEngineID>(ids,
			(_CORBA_Unbounded_Sequence<STI::TNetwork::TEngineID>&) tEngineIDseq_var);

		(*engineIDs) = tEngineIDseq_var;
	}
}


TEngineState TEventEngineScheduler_i::getEngineState(const ::STI::TNetwork::TEngineID& engineID)
{
	TEngineState tState = STI::TNetwork::TEngineState::EngineUnknown;

	if (engineScheduler != 0) {
		auto state = engineScheduler->getEngineState(convert<TEngineID, STI::Engine::EngineID>(engineID));
		convert<STI::Engine::EngineState, TEngineState>(state, tState);
	}
	return tState;
}

void TEventEngineScheduler_i::getEngineStates(::STI::TNetwork::TEngineStateTupleSeq_out engineStates)
{
	engineStates = new STI::TNetwork::TEngineStateTupleSeq();

	if (engineScheduler != 0) {
		std::map<STI::Engine::EngineID, STI::Engine::EngineState> states;
		engineScheduler->getEngineStates(states);

		STI::TNetwork::TEngineStateTupleSeq_var tEngineStateTupleSeq_var(new STI::TNetwork::TEngineStateTupleSeq);

		convert<std::map<STI::Engine::EngineID, STI::Engine::EngineState>, TEngineStateTupleSeq>(states, tEngineStateTupleSeq_var);

		(*engineStates) = tEngineStateTupleSeq_var;
	}
}

void TEventEngineScheduler_i::clearEngine(const ::STI::TNetwork::TEngineID& engineID) 
{
	if (engineScheduler != 0) {
		engineScheduler->clearEngine(convert<TEngineID, STI::Engine::EngineID>(engineID));
	}
}

void TEventEngineScheduler_i::stopEngine(const ::STI::TNetwork::TEngineID& engineID) 
{
	if (engineScheduler != 0) {
		engineScheduler->stopEngine(convert<TEngineID, STI::Engine::EngineID>(engineID));
	}
}

::CORBA::Boolean TEventEngineScheduler_i::getParseResult(const ::STI::TNetwork::TParseID& parseID, 
															::STI::TNetwork::TParseResult_out tParseResult)
{
	bool success = false;
	tParseResult = new STI::TNetwork::TParseResult();

    if (engineScheduler != 0) {

		STI::TNetwork::TParseResult_var tParseResult_var(new STI::TNetwork::TParseResult);
		std::shared_ptr<ParseResult> parseResult;

		success = engineScheduler->getParseResult(
			convert<TParseID, STI::Engine::ParseID>(parseID),
			parseResult);

		success = convert<std::shared_ptr<ParseResult>, ::STI::TNetwork::TParseResult>(parseResult, tParseResult_var);

		(*tParseResult) = tParseResult_var;

	}

	return success;
}

::CORBA::Boolean TEventEngineScheduler_i::getShotResult(const ::STI::TNetwork::TShotID& shotID, ::STI::TNetwork::TShotResult_out shotResult)
{
	bool success = false;
	shotResult = new STI::TNetwork::TShotResult();

	if (engineScheduler != 0) {

		STI::TNetwork::TShotResult_var tShotResult_var(new STI::TNetwork::TShotResult);
		std::shared_ptr<STI::Engine::ShotResult> localShotResult;

		success = engineScheduler->getShotResult(
			convert<TShotID, STI::Engine::ShotID>(shotID),
			localShotResult);

		success = convert<std::shared_ptr<STI::Engine::ShotResult>, ::STI::TNetwork::TShotResult>(localShotResult, tShotResult_var);

		(*shotResult) = tShotResult_var;

	}

	return success;
}

::CORBA::Boolean TEventEngineScheduler_i::getLastParseResult(const ::STI::TNetwork::TEngineID& engineID,
															 ::STI::TNetwork::TParseResult_out tParseResult)
{
	bool success = false;
	tParseResult = new STI::TNetwork::TParseResult();

	if (engineScheduler != 0) {

		STI::TNetwork::TParseResult_var tParseResult_var(new STI::TNetwork::TParseResult);
		std::shared_ptr<ParseResult> parseResult;

		success = engineScheduler->getLastParseResult(
			convert<TEngineID, STI::Engine::EngineID>(engineID),
			parseResult);

		if (success) {
			success = convert<std::shared_ptr<ParseResult>, ::STI::TNetwork::TParseResult>(parseResult, tParseResult_var);
			(*tParseResult) = tParseResult_var;
		}
	}

	return success;
}

::CORBA::Boolean TEventEngineScheduler_i::getLastShotResult(const ::STI::TNetwork::TEngineID& engineID,
															::STI::TNetwork::TShotResult_out shotResult)
{
	bool success = false;
	shotResult = new STI::TNetwork::TShotResult();

	if (engineScheduler != 0) {

		STI::TNetwork::TShotResult_var tShotResult_var(new STI::TNetwork::TShotResult);
		std::shared_ptr<STI::Engine::ShotResult> localShotResult;

		success = engineScheduler->getLastShotResult(
			convert<TEngineID, STI::Engine::EngineID>(engineID),
			localShotResult);

		if (success) {
			success = convert<std::shared_ptr<STI::Engine::ShotResult>, ::STI::TNetwork::TShotResult>(localShotResult, tShotResult_var);
			(*shotResult) = tShotResult_var;
		}
	}

	return success;
}

::CORBA::Boolean TEventEngineScheduler_i::ping()
{
	return true;
}
