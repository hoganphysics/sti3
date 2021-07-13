
#include "TEventEngine_i.h"

#include "ORBManager.h"
#include "DeviceID.h"
#include "Convert_EventEngine.h"
#include "EngineState.h"
#include "RemoteTriggerCallback.h"
#include "LocalEventEngineJob.h"
#include "RawEvent.h"
#include "RemoteResultsCollector.h"


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

//TEventEngine_i::TEventEngine_i(const std::shared_ptr<EventEngine>& engine)
//: eventEngine(engine)
//{
//}

TEventEngine_i::TEventEngine_i(STI::Engine::EventEngine* engine)
: eventEngine(engine)
{
}



TEventEngine_i::~TEventEngine_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

void TEventEngine_i::play(const ::STI::TNetwork::TEventEngineJob& job)
//void TEventEngine_i::play(::STI::TNetwork::TEventEngineJob_ptr job)
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
    
    remoteTriggerCB = std::make_shared<STI::Network::RemoteTriggerCallback>(triggerCB);

    if (eventEngine != 0) {

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


::CORBA::Boolean TEventEngine_i::getParsedEvents(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TDeviceEventsSeq_out events)
{
	bool success = false;

    if (eventEngine != 0) {

		STI::TNetwork::TDeviceEventsSeq_var tDeviceEventsSeq_var(new STI::TNetwork::TDeviceEventsSeq);
		STI::Engine::DeviceEventMap deviceEvents;

		success = eventEngine->getParsedEvents(convert<STI::TNetwork::TParseID, STI::Engine::ParseID>(parseID), deviceEvents);

		success &= convert<STI::Engine::DeviceEventMap, ::STI::TNetwork::TDeviceEventsSeq>(deviceEvents, tDeviceEventsSeq_var);	
		
		events = new STI::TNetwork::TDeviceEventsSeq();
		(*events) = tDeviceEventsSeq_var;
	}

	return success;
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

::CORBA::Boolean TEventEngine_i::getMeasurements(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TMeasurementSeq_out measurements)
{
	bool success = false;

    if (eventEngine != 0) {

		STI::TNetwork::TMeasurementSeq_var tMeasurementSeq_var(new STI::TNetwork::TMeasurementSeq);
		auto localMeasurements = std::make_shared<STI::Engine::MeasurementVector>();

		success = eventEngine->getMeasurements(convert<STI::TNetwork::TShotID, STI::Engine::ShotID>(sid), localMeasurements);

		success &= convert<std::shared_ptr<STI::Engine::Measurement>, STI::TNetwork::TMeasurement>(*localMeasurements,
					(_CORBA_Unbounded_Sequence<STI::TNetwork::TMeasurement>&) tMeasurementSeq_var);

		// success &= convert<STI::Engine::MeasurementVector, ::STI::TNetwork::TMeasurementSeq>(deviceEvents, tDeviceEventsSeq_var);	
		
		measurements = new STI::TNetwork::TMeasurementSeq();
		(*measurements) = tMeasurementSeq_var;
	}

	return success;
}

::CORBA::Boolean TEventEngine_i::transferMeasurements(::STI::TNetwork::TResultsCollector_ptr resultsCollector)
{
	bool success = false;

	auto remoteCollector = std::make_shared<STI::Network::RemoteResultsCollector>(resultsCollector);

	if (eventEngine != 0) {
		success = eventEngine->transferMeasurements(remoteCollector);
	}

	return success;
}

