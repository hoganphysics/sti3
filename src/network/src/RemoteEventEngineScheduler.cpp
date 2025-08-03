#include "RemoteEventEngineScheduler.h"

#include "generated/deviceNet.h"
#include "generated/orbTypes.h"

#include <sti/device/DeviceTrace.h>

#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/ParseResult.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEvent.h>

#include "convert/Convert_DeviceTrace.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_ShotResult.h"
#include "convert/Convert_SequenceResult.h"

#include "EventEngineDependencyTree.h"
#include "LocalEventEngineJob.h"
#include "LocalShot.h"
#include "NetworkShotWrapper.h"
#include "NetworkResultsCollector.h"
#include "RemoteEventEngineDependencyParser.h"

#include <memory>


using STI::Network::RemoteEventEngineScheduler;
using STI::Engine::EngineJobID;
using STI::TNetwork::TEngineJobID;
using STI::Network::convert;
using STI::Engine::EventEngineJob;
using STI::Device::DeviceTrace;
using STI::TNetwork::TDeviceTrace;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;
using STI::Network::NetworkShotWrapper;
using STI::TNetwork::TEventEngineJob;
using STI::Engine::Shot;
using STI::TNetwork::TParseID;
using STI::Engine::ParseID;
using STI::TNetwork::TEngineParsingMessage;
using STI::TNetwork::TReferenceHolder;
using STI::Network::NetworkResultsCollector;
using STI::TNetwork::TShotID;
using STI::Engine::ShotID;
using STI::Engine::EngineJobSourceID;
using STI::TNetwork::TEngineJobSourceID;
using STI::Engine::ShotConfig;
using STI::Engine::EngineJobStatus;
using STI::TNetwork::TEngineJobStatus;
using STI::Engine::EventEngineJobList;
using STI::TNetwork::TEventEngineJobList;
using STI::Engine::EventEngineDependencyParser;
using STI::Network::RemoteEventEngineDependencyParser;
using STI::Engine::Sequence;
using ::STI::TNetwork::TSequence;
using STI::Engine::SequenceEntryID;
using ::STI::TNetwork::TSequenceEntryID;
using STI::Engine::SequenceID;
using ::STI::TNetwork::TSequenceID;
using STI::Engine::ParseJobStatus;
using STI::TNetwork::TParseJobStatus;
using STI::Engine::PlayJobStatus;
using STI::TNetwork::TPlayJobStatus;
using STI::Engine::AddSequenceStatus;
using STI::TNetwork::TAddSequenceStatus;


RemoteEventEngineScheduler::RemoteEventEngineScheduler(::STI::TNetwork::TEventEngineScheduler_ptr scheduler)
	: STI::TNetwork::TReferenceHolder<STI::TNetwork::TEventEngineScheduler>(scheduler, schedulerMutex)
{
	addDependent(remoteDependencyParser);
}

RemoteEventEngineScheduler::~RemoteEventEngineScheduler()
{
}

ParseJobStatus RemoteEventEngineScheduler::parse(const std::shared_ptr<Shot>& shot)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	ParseJobStatus parseJobStatus;
	parseJobStatus.status = EngineJobStatus::NotFound;

	STI::TNetwork::TShot tShot;

	if (isDisabled()) return parseJobStatus;

	if (!convert<std::shared_ptr<Shot>, STI::TNetwork::TShot>(shot, tShot)) {
		return parseJobStatus;
	}

	try {
		auto tParseJobStatus = getTRef()->parse(tShot);	//remote call

		if (tParseJobStatus != 0) {
			parseJobStatus = convert<TParseJobStatus, ParseJobStatus>(*tParseJobStatus);			
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return parseJobStatus;
}

PlayJobStatus RemoteEventEngineScheduler::play(const ParseID& parseID, const EngineJobSourceID& source)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	PlayJobStatus playJobStatus;
	playJobStatus.status = EngineJobStatus::NotFound;

	if (isDisabled()) return playJobStatus;

	try {
		auto tPlayJobStatus = getTRef()->play(
							convert<ParseID, TParseID>(parseID), 
							convert<EngineJobSourceID, TEngineJobSourceID>(source));	//remote call

		if (tPlayJobStatus != 0) {
			playJobStatus = convert<TPlayJobStatus, PlayJobStatus>(*tPlayJobStatus);
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return playJobStatus;
}

AddSequenceStatus RemoteEventEngineScheduler::addSequence(const std::shared_ptr<Sequence>& sequence, const EngineJobSourceID& source)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	AddSequenceStatus addSequenceStatus;
	addSequenceStatus.status = EngineJobStatus::NotFound;

	STI::TNetwork::TSequence tSequence;

	if (isDisabled()) return addSequenceStatus;

	if (!convert<std::shared_ptr<Sequence>, STI::TNetwork::TSequence>(sequence, tSequence)) {
		return addSequenceStatus;
	}

	try {
		auto tAddSequenceStatus = getTRef()->addSequence(tSequence,
				convert<EngineJobSourceID, TEngineJobSourceID>(source));	//remote call

		if (tAddSequenceStatus != 0) {
			addSequenceStatus = convert<TAddSequenceStatus, AddSequenceStatus>(*tAddSequenceStatus);			
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return addSequenceStatus;
}

void RemoteEventEngineScheduler::closeSequence(const SequenceID& seqid)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

	try {
		getTRef()->closeSequence(convert<SequenceID, TSequenceID>(seqid));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngineScheduler::cancelSequence(const SequenceID& seqid)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

	try {
		getTRef()->cancelSequence(convert<SequenceID, TSequenceID>(seqid));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

ParseJobStatus RemoteEventEngineScheduler::parse(const std::shared_ptr<Shot>& shot, const SequenceEntryID& sequenceEntryID)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	ParseJobStatus parseJobStatus;
	parseJobStatus.status = EngineJobStatus::NotFound;

	STI::TNetwork::TShot tShot;

	if (isDisabled()) return parseJobStatus;

	if (!convert<std::shared_ptr<Shot>, STI::TNetwork::TShot>(shot, tShot)) {
		return parseJobStatus;
	}

	try {
		auto tParseJobStatus = getTRef()->parseSeqEntry(tShot,
				convert<SequenceEntryID, TSequenceEntryID>(sequenceEntryID));	//remote call

		if (tParseJobStatus != 0) {
			parseJobStatus = convert<TParseJobStatus, ParseJobStatus>(*tParseJobStatus);			
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return parseJobStatus;
}

EngineJobStatus RemoteEventEngineScheduler::getStatus(const ParseID& pid)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	EngineJobStatus status = EngineJobStatus::NotFound;

	if (isDisabled()) return status;

	try {
		auto tStatus = getTRef()->getStatusPID(convert<ParseID, TParseID>(pid));	//remote call

		status = convert<TEngineJobStatus, EngineJobStatus>(tStatus);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return status;
}

EngineJobStatus RemoteEventEngineScheduler::getStatus(const STI::Engine::ShotID& sid)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	EngineJobStatus status = EngineJobStatus::NotFound;

	if (isDisabled()) return status;

	try {
		auto tStatus = getTRef()->getStatusSID(convert<ShotID, TShotID>(sid));	//remote call

		status = convert<TEngineJobStatus, EngineJobStatus>(tStatus);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return status;
}

EngineJobStatus RemoteEventEngineScheduler::getStatus(const SequenceID& seqID)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	EngineJobStatus status = EngineJobStatus::NotFound;

	if (isDisabled()) return status;

	try {
		auto tStatus = getTRef()->getStatusSeqID(convert<SequenceID, TSequenceID>(seqID));	//remote call

		status = convert<TEngineJobStatus, EngineJobStatus>(tStatus);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return status;
}

bool RemoteEventEngineScheduler::getDependencyParser(std::shared_ptr<EventEngineDependencyParser>& dependencyParser)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	// isLive already?
	if (remoteDependencyParser != 0 && !remoteDependencyParser->isDisabled() && remoteDependencyParser->ping()) {
		dependencyParser = remoteDependencyParser;
		return (dependencyParser != 0);
	}
	else if (remoteDependencyParser != 0) {
		//non-null but not live for some reason; disable
		remoteDependencyParser->disable();
	}

	if (isDisabled()) return false;

	::STI::TNetwork::TEventEngineDependencyParser_var tDependencyParser;	//remote reference
	
	try {
		tDependencyParser = getTRef()->getDependencyParser();	//remote call

		if (!CORBA::is_nil(tDependencyParser)) {
			remoteDependencyParser = std::make_shared<RemoteEventEngineDependencyParser>(tDependencyParser);
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	dependencyParser = remoteDependencyParser;
	return (dependencyParser != 0);
}


bool RemoteEventEngineScheduler::getJob(const EngineJobID& id, std::shared_ptr<EventEngineJob>& job) const
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return false;

	bool success = false;

	STI::TNetwork::TEventEngineJob_var tJob_var(new STI::TNetwork::TEventEngineJob);

	try {
		success = getTRef()->getJob(convert<EngineJobID, TEngineJobID>(id), tJob_var);	//remote call

		if (success) {
			success = convert<TEventEngineJob, std::shared_ptr<EventEngineJob>>(tJob_var, job);
		}
		
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
	return success && (job != 0);
}


void RemoteEventEngineScheduler::addJob(const std::shared_ptr<EventEngineJob>& newJob)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

	if (newJob == 0) return;

	try {
		getTRef()->addJob(convert<EventEngineJob, TEventEngineJob>(*newJob));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngineScheduler::cancelJob(const EngineJobID& jobID)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

	try {
		getTRef()->cancelJob(convert<EngineJobID, TEngineJobID>(jobID));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngineScheduler::cancelAll()
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

	try {
		getTRef()->cancelAll();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

std::set<EngineJobID> RemoteEventEngineScheduler::getJobIDs(const EventEngineJobList& jobListType) const
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	std::set<EngineJobID> jobIDs;

	if (isDisabled()) return jobIDs;

	STI::TNetwork::TEngineJobIDSeq_var tEngineJobIDs(new STI::TNetwork::TEngineJobIDSeq);

	try {

		tEngineJobIDs = getTRef()->getJobIDs(convert<EventEngineJobList, TEventEngineJobList>(jobListType));	//remote call

		convert<TEngineJobID, EngineJobID>(tEngineJobIDs, jobIDs);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
	return jobIDs;
}

std::vector<std::shared_ptr<EventEngineJob>> RemoteEventEngineScheduler::getJobs(const EventEngineJobList& jobListType) const
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	std::vector<std::shared_ptr<EventEngineJob>> jobs;

	if (isDisabled()) return jobs;

	STI::TNetwork::TEventEngineJobSeq_var tEngineJobs(new STI::TNetwork::TEventEngineJobSeq);

    try {

		tEngineJobs = getTRef()->getJobs(convert<EventEngineJobList, TEventEngineJobList>(jobListType));	//remote call

		convert<TEventEngineJob, std::shared_ptr<EventEngineJob>>(tEngineJobs, jobs);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
	return jobs;
}

std::shared_ptr<Shot> RemoteEventEngineScheduler::createShot(const ShotConfig& shotConfig, 
															 const std::shared_ptr<STI::Engine::RawEventGroup>& eventGroup)
{
    auto shot = std::make_shared<STI::Engine::LocalShot>(shotConfig, eventGroup);
    auto networkShot = std::make_shared<NetworkShotWrapper>(shot);

	return networkShot;
}

bool RemoteEventEngineScheduler::getParseResult(const ParseID& parseID, std::shared_ptr<STI::Engine::ParseResult>& parseResult) const
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TParseResult_var tParseResult(new STI::TNetwork::TParseResult);

	bool success = false;

    try {
		success = getTRef()->getParseResult(convert<ParseID, TParseID>(parseID), tParseResult);		//remote call

        success &= convert<STI::TNetwork::TParseResult, std::shared_ptr<STI::Engine::ParseResult>>(tParseResult, parseResult);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	if (!success) {
		parseResult = std::make_shared<STI::Engine::ParseResult>();
		parseResult->pid = parseID;
	}

	return success;
}


bool RemoteEventEngineScheduler::ping() const
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->ping();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}

