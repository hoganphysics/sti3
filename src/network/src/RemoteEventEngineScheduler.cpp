
#include "RemoteEventEngineScheduler.h"
#include "DeviceTrace.h"
#include "EventEngineDependencyTree.h"
#include "LocalEventEngineJob.h"
#include "Convert_EventEngine.h"
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

RemoteEventEngineScheduler::RemoteEventEngineScheduler(::STI::TNetwork::TEventEngineScheduler_ptr scheduler)
	: tEventEngineScheduler(STI::TNetwork::TEventEngineScheduler::_duplicate(scheduler))
{
}

RemoteEventEngineScheduler::~RemoteEventEngineScheduler()
{
}

void RemoteEventEngineScheduler::parse(const STI::Engine::ParseID& parseID, const std::shared_ptr<Shot>& shot)
{
	if (CORBA::is_nil(tEventEngineScheduler)) return;

	STI::TNetwork::TShot_ptr tShot;

	if (!convert<std::shared_ptr<Shot>, STI::TNetwork::TShot_ptr>(shot, tShot)) {
	//	return;
	}

	// std::shared_ptr<std::vector<STI::Engine::RawEvent>> evts;
	// shot->getEvents(evts);

  	// std::cout << "RemoteEventEngineScheduler::parse " << evts->size() << std::endl;
 


	try {

		tEventEngineScheduler->parse(convert<ParseID, TParseID>(parseID), tShot);	//remote call
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
	if (CORBA::is_nil(tEventEngineScheduler)) return;

	try {
        
		tEventEngineScheduler->play(convert<STI::Engine::ShotID, STI::TNetwork::TShotID>(shotID));	//remote call
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
	if (CORBA::is_nil(tEventEngineScheduler)) return;

//	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages(new STI::TNetwork::TEngineParsingMessageSeq);
	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages;

    try {

        STI::TNetwork::TDeviceIDSeq_var tEvtTargets(new STI::TNetwork::TDeviceIDSeq);
        convert<DeviceID, TDeviceID>(evtTargets, tEvtTargets);

        STI::TNetwork::TEventEngineDependencyTree tTree;
        convert<EventEngineDependencyTree, TEventEngineDependencyTree>(tree, tTree);
    
        STI::TNetwork::TDeviceIDSeq_var tMissingTargets(new STI::TNetwork::TDeviceIDSeq);
        convert<DeviceID, TDeviceID>(missingTargets, tMissingTargets);

		tEventEngineScheduler->getDependants(tEvtTargets, tTree, tMissingTargets, tEngineParsingMessages,
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
	if (CORBA::is_nil(tEventEngineScheduler)) return;

//	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages(new STI::TNetwork::TEngineParsingMessageSeq);
	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages;

    try {
        STI::TNetwork::TEventEngineDependencyTree tTree;
        convert<EventEngineDependencyTree, TEventEngineDependencyTree>(tree, tTree);

		tEventEngineScheduler->addDeviceEventTargets(tTree, tEngineParsingMessages, 
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
	if (CORBA::is_nil(tEventEngineScheduler)) return;

	try {
		tEventEngineScheduler->addJob(convert<EventEngineJob, TEventEngineJob>(*newJob));	//remote call
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
	if (CORBA::is_nil(tEventEngineScheduler)) return;

	try {
		tEventEngineScheduler->cancelJob(convert<EngineJobID, TEngineJobID>(jobID));	//remote call
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
	if (CORBA::is_nil(tEventEngineScheduler)) return;

	try {
		tEventEngineScheduler->cancelAll();	//remote call
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
	if (CORBA::is_nil(tEventEngineScheduler)) return false;

	try {
//		tEventEngineScheduler->getParsedEvents(convert<ParseID, TParseID>(parseID), );	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
	return false;
}

bool RemoteEventEngineScheduler::getParsingMessages(const STI::Engine::ParseID& parseID, std::vector<STI::Engine::EngineParsingMessage>& messages) const
{
	if (CORBA::is_nil(tEventEngineScheduler)) return false;

	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages(new STI::TNetwork::TEngineParsingMessageSeq);
//	STI::TNetwork::TEngineParsingMessageSeq_var tEngineParsingMessages;

	bool success = false;

    try {

		success = tEventEngineScheduler->getParsingMessages(convert<ParseID, TParseID>(parseID), tEngineParsingMessages);	//remote call

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

bool RemoteEventEngineScheduler::getParsedTree(const STI::Engine::ParseID& parseID, std::shared_ptr<STI::Engine::EventEngineDependencyTree>& tree) const
{
	if (CORBA::is_nil(tEventEngineScheduler)) return false;

	STI::TNetwork::TEventEngineDependencyTree_var tTree(new STI::TNetwork::TEventEngineDependencyTree);
//	STI::TNetwork::TEventEngineDependencyTree_var tTree;
	
	tree = std::make_shared<STI::Engine::EventEngineDependencyTree>();

	bool success = false;

    try {
		success = tEventEngineScheduler->getParsedTree(convert<ParseID, TParseID>(parseID), tTree);	//remote call

        convert<TEventEngineDependencyTree, EventEngineDependencyTree>(tTree, *tree);
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

bool RemoteEventEngineScheduler::ping() const
{
	if (CORBA::is_nil(tEventEngineScheduler)) return false;

	bool success = false;

	try {
		success = tEventEngineScheduler->ping();	//remote call
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

