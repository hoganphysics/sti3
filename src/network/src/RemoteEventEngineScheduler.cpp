
#include "RemoteEventEngineScheduler.h"
#include "DeviceTrace.h"
#include "EventEngineDependencyTree.h"
#include "LocalEventEngineJob.h"
#include "Convert_EventEngine.h"
#include "deviceNet.h"
#include "orbTypes.h"
#include "NetworkParsedShotWrapper.h"
#include "EngineJobID.h"

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
using STI::Network::NetworkParsedShotWrapper;
using STI::TNetwork::TEventEngineJob;
using STI::Engine::Shot;
using STI::TNetwork::TParseID;
using STI::Engine::ParseID;

RemoteEventEngineScheduler::RemoteEventEngineScheduler(::STI::TNetwork::TEventEngineScheduler_ptr scheduler)
	: tEventEngineScheduler(STI::TNetwork::TEventEngineScheduler::_duplicate(scheduler))
{
}

RemoteEventEngineScheduler::~RemoteEventEngineScheduler()
{
}

void RemoteEventEngineScheduler::parse(const STI::Engine::ParseID& parseID, const std::shared_ptr<Shot>& shot)
{
	STI::TNetwork::TParsedShot_ptr tShot;

	if (!convert<std::shared_ptr<Shot>, STI::TNetwork::TParsedShot_ptr>(shot, tShot)) {
		return;
	}
  
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


void RemoteEventEngineScheduler::getDependants(const std::set<STI::Device::DeviceID>& evtTargets, STI::Engine::EventEngineDependencyTree& tree, 
                                std::set<STI::Device::DeviceID>& missingTargets, const STI::Device::DeviceTrace& trace)
{
    try {

        STI::TNetwork::TDeviceIDSeq_var tEvtTargets(new STI::TNetwork::TDeviceIDSeq);
        convert<DeviceID, TDeviceID>(evtTargets, tEvtTargets);

        STI::TNetwork::TEventEngineDependencyTree tTree;
        convert<EventEngineDependencyTree, TEventEngineDependencyTree>(tree, tTree);
    
        STI::TNetwork::TDeviceIDSeq_var tMissingTargets(new STI::TNetwork::TDeviceIDSeq);
        convert<DeviceID, TDeviceID>(missingTargets, tMissingTargets);

		tEventEngineScheduler->getDependants(tEvtTargets, tTree, tMissingTargets, 
                                             convert<DeviceTrace, TDeviceTrace>(trace));	//remote call

        convert<TEventEngineDependencyTree, EventEngineDependencyTree>(tTree, tree);
        convert<TDeviceID, DeviceID>(tMissingTargets, missingTargets);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

    
void RemoteEventEngineScheduler::addDeviceEventTargets(STI::Engine::EventEngineDependencyTree& tree, const DeviceTrace& trace)
{
    try {
        STI::TNetwork::TEventEngineDependencyTree tTree;
        convert<EventEngineDependencyTree, TEventEngineDependencyTree>(tree, tTree);

		tEventEngineScheduler->addDeviceEventTargets(tTree,
                                                     convert<DeviceTrace, TDeviceTrace>(trace));	//remote call

        convert<TEventEngineDependencyTree, EventEngineDependencyTree>(tTree, tree);
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


std::shared_ptr<STI::Engine::EventEngineJob> RemoteEventEngineScheduler::createJob(const STI::Engine::ParseID& parseID, 
                                          const std::shared_ptr<STI::Engine::Shot>& shot,
                                          const std::shared_ptr<STI::Engine::EventEngineDependencyTree>& tree, 
                                          const STI::Device::DeviceID& owner, 
                                          const std::set<STI::Device::DeviceID>& missingTargets)
{
    auto networkShot = std::make_shared<NetworkParsedShotWrapper>(shot);
    auto newJob = std::make_shared<STI::Engine::LocalEventEngineJob>(parseID, networkShot, tree, owner, missingTargets);
    return newJob;
}

