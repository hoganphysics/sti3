#include "RemoteEventEngineScheduler.h"
#include <sti/device/DeviceTrace.h>
#include "EventEngineDependencyTree.h"
#include "ParsedDependencyTree.h"
#include "LocalEventEngineJob.h"
#include "Convert_EventEngine.h"
#include "Convert_DeviceTrace.h"
#include "Convert_ShotResult.h"

#include "deviceNet.h"
#include "orbTypes.h"
#include "NetworkShotWrapper.h"
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/RawEvent.h>
#include "LocalShot.h"
#include "NetworkResultsCollector.h"
#include <sti/engine/ParseResult.h>

#include <memory>

using STI::Network::RemoteEventEngineScheduler;
using STI::Engine::EngineJobID;
using STI::TNetwork::TEngineJobID;
using STI::Network::convert;
using STI::Engine::EventEngineJob;
using STI::Engine::EventEngineDependencyTree;
using STI::Engine::ParsedDependencyTree;
using STI::TNetwork::TEventEngineDependencyTree;
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
using STI::Engine::EngineParsingMessage;
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


RemoteEventEngineScheduler::RemoteEventEngineScheduler(::STI::TNetwork::TEventEngineScheduler_ptr scheduler)
	: STI::TNetwork::TReferenceHolder<STI::TNetwork::TEventEngineScheduler>(scheduler, schedulerMutex)
{
}

RemoteEventEngineScheduler::~RemoteEventEngineScheduler()
{
}

ParseID RemoteEventEngineScheduler::parse(const std::shared_ptr<Shot>& shot)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	ParseID pid;
	STI::TNetwork::TShot tShot;

	if (isDisabled()) return pid;

	if (!convert<std::shared_ptr<Shot>, STI::TNetwork::TShot>(shot, tShot)) {
		return pid;
	}

	try {
		auto tParseID = getTRef()->parse(tShot);	//remote call

		if (tParseID != 0) {
			pid = convert<TParseID, ParseID>(*tParseID);			
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return pid;
}

ShotID RemoteEventEngineScheduler::play(const ParseID& parseID, const EngineJobSourceID& source)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	ShotID sid;

	if (isDisabled()) return sid;

	try {
		auto tShotID = getTRef()->play(
							convert<ParseID, TParseID>(parseID), 
							convert<EngineJobSourceID, TEngineJobSourceID>(source));	//remote call

		if (tShotID != 0) {
			sid = convert<TShotID, ShotID>(*tShotID);
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return sid;
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

void RemoteEventEngineScheduler::getDependants(const std::set<STI::Device::DeviceID>& evtTargets, 
											   STI::Engine::EventEngineDependencyTree& tree, 
                                			   std::set<STI::Device::DeviceID>& missingTargets, 
											   std::vector<STI::Engine::EngineParsingMessage>& messages, 
											   const STI::Device::DeviceTrace& trace)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages;

    try {

        STI::TNetwork::TDeviceIDSeq_var tEvtTargets(new STI::TNetwork::TDeviceIDSeq);
        convert<DeviceID, TDeviceID>(evtTargets, tEvtTargets);

        STI::TNetwork::TEventEngineDependencyTree tTree;
        convert<EventEngineDependencyTree, TEventEngineDependencyTree>(tree, tTree);
    
        STI::TNetwork::TDeviceIDSeq_var tMissingTargets(new STI::TNetwork::TDeviceIDSeq);
        convert<DeviceID, TDeviceID>(missingTargets, tMissingTargets);

		getTRef()->getDependants(tEvtTargets, tTree, tMissingTargets, tEngineParsingMessages,
                                             convert<DeviceTrace, TDeviceTrace>(trace));	//remote call

        convert<TEventEngineDependencyTree, EventEngineDependencyTree>(tTree, tree);
        convert<TDeviceID, DeviceID>(tMissingTargets, missingTargets);
		convert<TEngineParsingMessage, EngineParsingMessage>(tEngineParsingMessages, messages);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngineScheduler::addDeviceEventTargets(EventEngineDependencyTree& tree, 
													   std::vector<EngineParsingMessage>& messages, 
													   const DeviceTrace& trace)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages;

    try {
        STI::TNetwork::TEventEngineDependencyTree tTree;
        convert<EventEngineDependencyTree, TEventEngineDependencyTree>(tree, tTree);

		getTRef()->addDeviceEventTargets(tTree, tEngineParsingMessages, 
                                                     convert<DeviceTrace, TDeviceTrace>(trace));	//remote call

        convert<TEventEngineDependencyTree, EventEngineDependencyTree>(tTree, tree);
		convert<TEngineParsingMessage, EngineParsingMessage>(tEngineParsingMessages, messages);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
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

