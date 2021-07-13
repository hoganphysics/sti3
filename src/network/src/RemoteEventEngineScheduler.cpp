
#include "RemoteEventEngineScheduler.h"
#include "DeviceTrace.h"
#include "EventEngineDependencyTree.h"
#include "ParsedDependencyTree.h"
#include "LocalEventEngineJob.h"
#include "Convert_EventEngine.h"

#include "Convert_DeviceTrace.h"

#include "deviceNet.h"
#include "orbTypes.h"
#include "NetworkShotWrapper.h"
#include "EngineJobID.h"
#include "EngineParsingMessage.h"
#include "RawEvent.h"
#include "LocalShot.h"


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


RemoteEventEngineScheduler::RemoteEventEngineScheduler(::STI::TNetwork::TEventEngineScheduler_ptr scheduler)
	: STI::TNetwork::TReferenceHolder<STI::TNetwork::TEventEngineScheduler>(scheduler, schedulerMutex)
	//: tEventEngineScheduler(STI::TNetwork::TEventEngineScheduler::_duplicate(scheduler))
{
}

RemoteEventEngineScheduler::~RemoteEventEngineScheduler()
{
}

void RemoteEventEngineScheduler::parse(const STI::Engine::ParseID& parseID, const std::shared_ptr<Shot>& shot)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

	STI::TNetwork::TShot_ptr tShot;

	if (!convert<std::shared_ptr<Shot>, STI::TNetwork::TShot_ptr>(shot, tShot)) {
	//	return;
	}

	// std::shared_ptr<std::vector<STI::Engine::RawEvent>> evts;
	// shot->getEvents(evts);

  	// std::cout << "RemoteEventEngineScheduler::parse " << evts->size() << std::endl;
 


	try {

		getTRef()->parse(convert<ParseID, TParseID>(parseID), tShot);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngineScheduler::play(const STI::Engine::ShotID& shotID)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

	try {
        
		getTRef()->play(convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(shotID));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

///////////////////////*******************
void RemoteEventEngineScheduler::getDependants(const std::set<STI::Device::DeviceID>& evtTargets, STI::Engine::EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, std::vector<STI::Engine::EngineParsingMessage>& messages, 
								const STI::Device::DeviceTrace& trace)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

//	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages(new STI::TNetwork::TEngineParsingMessageSeq);
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
														std::vector<EngineParsingMessage>& messages, const DeviceTrace& trace)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

//	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages(new STI::TNetwork::TEngineParsingMessageSeq);
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

    
void RemoteEventEngineScheduler::addJob(const std::shared_ptr<EventEngineJob>& newJob)
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return;

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

void  RemoteEventEngineScheduler::cancelAll()
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

// std::shared_ptr<STI::Engine::EventEngineJob> RemoteEventEngineScheduler::createJob(const STI::Engine::ParseID& parseID, 
//                                           const std::shared_ptr<STI::Engine::Shot>& shot,
//                                           const std::shared_ptr<STI::Engine::EventEngineDependencyTree>& tree, 
//                                           const STI::Device::DeviceID& owner, 
//                                           const std::set<STI::Device::DeviceID>& missingTargets)
// {
// //    auto networkShot = std::make_shared<NetworkShotWrapper>(shot);
    
// 	auto newJob = std::make_shared<STI::Engine::LocalEventEngineJob>(parseID, shot, owner);
//     newJob->setDependencies(tree);
//     newJob->setMissingTargets(missingTargets);

//     return newJob;
// }

std::shared_ptr<Shot> RemoteEventEngineScheduler::createShot(const std::shared_ptr<STI::Engine::RawEventVector>& events)
{
    auto shot = std::make_shared<STI::Engine::LocalShot>();
	shot->setEvents(events);

    auto networkShot = std::make_shared<NetworkShotWrapper>(shot);

	return networkShot;
}

bool RemoteEventEngineScheduler::getParsedEvents(const STI::Engine::ParseID& parseID, STI::Engine::DeviceEventMap& events) const
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TDeviceEventsSeq_var tEngineParsedEvents(new STI::TNetwork::TDeviceEventsSeq);

	bool success = false;

	try {

		success = getTRef()->getParsedEvents(convert<ParseID, TParseID>(parseID), tEngineParsedEvents);	//remote call

		convert<::STI::TNetwork::TDeviceEventsSeq, STI::Engine::DeviceEventMap>(tEngineParsedEvents, events);
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

bool RemoteEventEngineScheduler::getParsingMessages(const STI::Engine::ParseID& parseID, std::vector<STI::Engine::EngineParsingMessage>& messages) const
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages(new STI::TNetwork::TEngineParsingMessageSeq);

	bool success = false;

    try {

		success = getTRef()->getParsingMessages(convert<ParseID, TParseID>(parseID), tEngineParsingMessages);	//remote call

		convert<TEngineParsingMessage, EngineParsingMessage>(tEngineParsingMessages, messages);
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

bool RemoteEventEngineScheduler::getParsedTree(const STI::Engine::ParseID& parseID, 
									std::shared_ptr<STI::Engine::ParsedDependencyTree>& tree) const
{
	std::unique_lock<std::mutex> schedulerLock(schedulerMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TEventEngineDependencyTree_var tTree(new STI::TNetwork::TEventEngineDependencyTree);

	bool success = false;

    try {
		success = getTRef()->getParsedTree(convert<ParseID, TParseID>(parseID), tTree);	//remote call

        success &= convert<TEventEngineDependencyTree, std::shared_ptr<STI::Engine::ParsedDependencyTree>>(tTree, tree);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	if (!success) {
		auto emptyTree = std::make_shared<STI::Engine::EventEngineDependencyTree>();
		tree = std::make_shared<STI::Engine::ParsedDependencyTree>(emptyTree);
	}

	return success;
}

    bool transferMeasurements(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector);
    bool getResults(const STI::Engine::ShotID& shotID, std::shared_ptr<STI::Engine::ResultTicket>& results);

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

