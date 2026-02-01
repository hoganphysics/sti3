#include "TEventEngine_i.h"

#include "convert/Convert_EventEngine.h"
#include "convert/Convert_ShotResult.h"

#include <sti/device/DeviceID.h>
#include <sti/engine/EngineState.h>
#include <sti/engine/RawEvent.h>

#include "LocalEventEngineJob.h"
#include "RemoteResultsCollector.h"
#include "RemoteTriggerCallback.h"

using STI::TNetwork::TEventEngine_i;
using STI::Engine::EventEngine;
using ::STI::TNetwork::TEngineJobID;
using ::STI::TNetwork::TDeviceID;
using STI::Device::DeviceID;
using ::STI::TNetwork::TEngineState;
using STI::Network::convert;
using STI::Engine::EngineState;
using STI::Network::RemoteTriggerCallback;
using STI::Engine::EventEngineJob;
using STI::Engine::EngineJobID;
using STI::TNetwork::TEventEngineJob;
using ::STI::TNetwork::TEventEngineDependencyTree;


TEventEngine_i::TEventEngine_i(STI::Engine::EventEngine* engine)
: eventEngine(engine)
{
}

TEventEngine_i::~TEventEngine_i()
{
}

void TEventEngine_i::play(const ::STI::TNetwork::TEventEngineJob& job)
{
    std::shared_ptr<EventEngineJob> engineJob;
    convert<TEventEngineJob, std::shared_ptr<EventEngineJob>>(job, engineJob);

    if (eventEngine != 0 && engineJob != 0) {
		eventEngine->play(*engineJob);
	}
}

void TEventEngine_i::playCB(const TEngineJobID& jobID, 
                ::STI::TNetwork::TTriggerCallback_ptr triggerCB, ::CORBA::Boolean debug)
{
    if (eventEngine != 0 && !CORBA::is_nil(triggerCB)) {
		
		STI::TNetwork::TTriggerCallback_var triggerCB_var = STI::TNetwork::TTriggerCallback::_duplicate(triggerCB);
    	remoteTriggerCB = std::make_shared<STI::Network::RemoteTriggerCallback>(triggerCB_var);
		
		eventEngine->play(
                convert<TEngineJobID, EngineJobID>(jobID),
                remoteTriggerCB,
                static_cast<bool>(debug)
                );
	}
}

void TEventEngine_i::trigger()
{
    if (eventEngine != 0) {
		eventEngine->trigger();
	}
}

void TEventEngine_i::triggerTarget(const TDeviceID& target)
{
    if (eventEngine != 0) {
		eventEngine->trigger(convert<TDeviceID, DeviceID>(target));
	}
}

void TEventEngine_i::stop()
{
    if (eventEngine != 0) {
		eventEngine->stop();
	}
}

void TEventEngine_i::pause()
{
    if (eventEngine != 0) {
		eventEngine->pause();
	}
}

void TEventEngine_i::unpause(::CORBA::Boolean retrigger)
{
    if (eventEngine != 0) {
		eventEngine->unpause(static_cast<bool>(retrigger));
	}
}

void TEventEngine_i::clear()
{
	if (eventEngine != 0) {
		eventEngine->clear();
	}
}

TDeviceID* TEventEngine_i::getDeviceID()
{
	STI::TNetwork::TDeviceID_var tDevice(new STI::TNetwork::TDeviceID);

	if(eventEngine != 0) {
		convert<DeviceID, TDeviceID>(eventEngine->getDeviceID(), tDevice);
	}

	return tDevice._retn();
}

TEngineState TEventEngine_i::getState()
{
    TEngineState tState;
    tState = ::STI::TNetwork::EngineMissing;

    if (eventEngine != 0) {
		convert<EngineState, TEngineState>(eventEngine->getState(), tState);
	}

    return tState;
}


TEventEngineDependencyTree* TEventEngine_i::getParsedTree()
{
	STI::TNetwork::TEventEngineDependencyTree_var tEventEngineDependencyTree_var(new STI::TNetwork::TEventEngineDependencyTree);
	
	if (eventEngine != 0) {
		convert<std::shared_ptr<STI::Engine::ParsedDependencyTree>, TEventEngineDependencyTree>(
									eventEngine->getParsedTree(), tEventEngineDependencyTree_var);
	}

	return tEventEngineDependencyTree_var._retn();
}


::CORBA::Boolean TEventEngine_i::getParseResult(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TParseResult_out tParseResult)
{
	bool success = false;
	tParseResult = new STI::TNetwork::TParseResult();

    if (eventEngine != 0) {

		STI::TNetwork::TParseResult_var tParseResult_var(new STI::TNetwork::TParseResult);
		std::shared_ptr<STI::Engine::ParseResult> parseResult;

		success = eventEngine->getParseResult(convert<STI::TNetwork::TParseID, STI::Engine::ParseID>(parseID), parseResult);

		success &= convert<std::shared_ptr<STI::Engine::ParseResult>, ::STI::TNetwork::TParseResult>(parseResult, tParseResult_var);	
		
		(*tParseResult) = tParseResult_var;
	}

	return success;
}

