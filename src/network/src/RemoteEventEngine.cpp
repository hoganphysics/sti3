
#include "RemoteEventEngine.h"
#include "Convert_EventEngine.h"
#include <sti/engine/RawEvent.h>
#include "ParsedDependencyTree.h"
#include "NetworkResultsCollector.h"
#include "ORBManager.h"


using STI::Network::RemoteEventEngine;
using STI::Engine::EventEngineJob;
using STI::TNetwork::TEventEngineJob;
using STI::Network::convert;
using STI::Engine::TriggerCallback;
using STI::TNetwork::TParseID;
using STI::Engine::ParseID;
using STI::Engine::ParsedDependencyTree;
using STI::TNetwork::TShotID;
using STI::Engine::ShotID;

using STI::Network::NetworkResultsCollector;


RemoteEventEngine::RemoteEventEngine(::STI::TNetwork::TEventEngine_ptr engine)
: STI::TNetwork::TReferenceHolder<STI::TNetwork::TEventEngine>(engine, engineMutex)
//	: _tEngine(STI::TNetwork::TEventEngine::_duplicate(engine))
{
}

RemoteEventEngine::~RemoteEventEngine()
{
	disable();
}


void RemoteEventEngine::play(EventEngineJob& job)
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	if (isDisabled()) return;

	try {
		getTRef()->play(convert<EventEngineJob, TEventEngineJob>(job));
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngine::play(const STI::Engine::EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug)
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	if (isDisabled()) return;

    triggerCallbackServant = std::make_shared<STI::TNetwork::TTriggerCallback_i>(triggerCB);

	if (triggerCallbackServant != 0) {
		STI::Network::ORBManager::ORBManager::activateServant(*triggerCallbackServant);
	}

	try {
		if (triggerCallbackServant != 0) {
			
			getTRef()->playCB(
				convert<STI::Engine::EngineJobID, STI::TNetwork::TEngineJobID>(jobID),
				(*triggerCallbackServant)._this(),
				static_cast<CORBA::Boolean>(debug)
				);			
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}


void RemoteEventEngine::trigger()
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	if (isDisabled()) return;

	try {
		getTRef()->trigger();
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngine::trigger(const STI::Device::DeviceID& target)
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	if (isDisabled()) return;

	try {
		getTRef()->triggerTarget(convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(target));
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}


void RemoteEventEngine::stop()
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	if (isDisabled()) return;

	try {
		getTRef()->stop();
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngine::pause()
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	if (isDisabled()) return;

	try {
		getTRef()->pause();
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}

void RemoteEventEngine::unpause(bool retrigger)
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	if (isDisabled()) return;

	try {
		getTRef()->unpause(static_cast<CORBA::Boolean>(retrigger));
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}
}


STI::Device::DeviceID RemoteEventEngine::getDeviceID() const
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	::STI::TNetwork::TDeviceID_var tDeviceID;

	bool success = false;

	try {
		if (!isDisabled()) {
			tDeviceID = getTRef()->getDeviceID();
			success = true;
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	STI::Device::DeviceID deviceID;

	if(success) {
		deviceID = convert<::STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tDeviceID);
	}

	return deviceID;
}


STI::Engine::EngineState RemoteEventEngine::getState() const
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	::STI::TNetwork::TEngineState tState = ::STI::TNetwork::EngineUnknown;

	bool success = false;

	try {
		if (!isDisabled()) {
			tState = getTRef()->getState();
			success = true;			
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	STI::Engine::EngineState state = STI::Engine::EngineState::Missing;

	if(success) {
		convert<::STI::TNetwork::TEngineState, STI::Engine::EngineState>(tState, state);
	}

	return state;
}

std::shared_ptr<STI::Engine::ParsedDependencyTree> RemoteEventEngine::getParsedTree() const
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	::STI::TNetwork::TEventEngineDependencyTree_var tTree(new ::STI::TNetwork::TEventEngineDependencyTree);

	bool success = false;

	try {
		if (!isDisabled()) {
			tTree = getTRef()->getParsedTree();
			success = true;			
		}
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	std::shared_ptr<STI::Engine::ParsedDependencyTree> tree;

	if(success) {
		convert<::STI::TNetwork::TEventEngineDependencyTree, std::shared_ptr<STI::Engine::ParsedDependencyTree>>(tTree, tree);
	}

	return tree;
}

// const STI::Engine::DeviceEventMap& RemoteEventEngine::getParsedEvents()
bool RemoteEventEngine::getParsedEvents(const STI::Engine::ParseID& parseID, STI::Engine::DeviceEventMap& parsedEvents)
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TDeviceEventsSeq_var tEngineParsedEvents(new STI::TNetwork::TDeviceEventsSeq);

	bool success = false;

	try {
		success = getTRef()->getParsedEvents(convert<ParseID, TParseID>(parseID), tEngineParsedEvents);	//remote call

		success &= convert<::STI::TNetwork::TDeviceEventsSeq, STI::Engine::DeviceEventMap>(tEngineParsedEvents, parsedEvents);
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

// bool RemoteEventEngine::getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementVector>& measurements)
// {
// 	std::unique_lock<std::mutex> engineLock(engineMutex);

// 	if (isDisabled()) return false;

// 	STI::TNetwork::TMeasurementSeq_var tMeasurements(new STI::TNetwork::TMeasurementSeq);
// 	measurements = std::make_shared<STI::Engine::MeasurementVector>();

// 	bool success = false;

// 	try {
// 		success = getTRef()->getMeasurements(convert<ShotID, TShotID>(sid), tMeasurements);	//remote call

// 		//success &= convert<::STI::TNetwork::TMeasurementSeq, STI::Engine::MeasurementVector>(tMeasurements, *measurements); (_CORBA_Unbounded_Sequence<::STI::TNetwork::TMeasurement>) 
// 		success &= convert<::STI::TNetwork::TMeasurement, std::shared_ptr<STI::Engine::Measurement>>(tMeasurements, *measurements);
// 	}
// 	catch (CORBA::TRANSIENT&) {
// 	}
// 	catch (CORBA::SystemException&) {
// 	}
// 	catch (CORBA::Exception&)
// 	{
// 	}
// 	return success;
// }

// bool RemoteEventEngine::transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector)
// {
// 	std::unique_lock<std::mutex> engineLock(engineMutex);

// 	if (isDisabled()) return false;

// 	STI::TNetwork::TResultsCollector_var tResultsCollector;

// 	bool success = false;

// 	try {

// 		if (NetworkResultsCollector::getTResultsCollector(resultsCollector, tResultsCollector)) {
// 			success = getTRef()->transferResults(tResultsCollector);	//remote call
// 		}
// 	}
// 	catch (CORBA::TRANSIENT&) {
// 	}
// 	catch (CORBA::SystemException&) {
// 	}
// 	catch (CORBA::Exception&) {
// 	}
// 	return success;
// }

