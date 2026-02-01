#include "RemoteEventEngine.h"
#include "convert/Convert_EventEngine.h"
#include "convert/Convert_ShotResult.h"

#include <sti/engine/RawEvent.h>
#include <sti/engine/ParsedDependencyTree.h>

#include "NetworkResultsCollector.h"


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


RemoteEventEngine::RemoteEventEngine(::STI::TNetwork::TEventEngine_var engine)
: STI::TNetwork::TReferenceHolder<STI::TNetwork::TEventEngine>(engine)
{
}

RemoteEventEngine::~RemoteEventEngine()
{
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

	triggerCallbackServantHolder.emplace(triggerCB);

	try {
		getTRef()->playCB(
			convert<STI::Engine::EngineJobID, STI::TNetwork::TEngineJobID>(jobID),
			triggerCallbackServantHolder.getRefPtr(),
			static_cast<CORBA::Boolean>(debug)
			);
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

void RemoteEventEngine::clear()
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	if (isDisabled()) return;

	try {
		getTRef()->clear();
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


bool RemoteEventEngine::getParseResult(const STI::Engine::ParseID& parseID, std::shared_ptr<STI::Engine::ParseResult>& parseResult) const
{
	std::unique_lock<std::mutex> engineLock(engineMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TParseResult_var tParseResult(new STI::TNetwork::TParseResult);

	bool success = false;

	try {
		success = getTRef()->getParseResult(convert<ParseID, TParseID>(parseID), tParseResult);	//remote call

		success &= convert<::STI::TNetwork::TParseResult, std::shared_ptr<STI::Engine::ParseResult>>(tParseResult, parseResult);
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

