#ifndef STI_NETWORK_CONVERT_EVENTENGINE_H
#define STI_NETWORK_CONVERT_EVENTENGINE_H

#include "NetworkConvert.h"
#include "deviceNet.h"
#include "EngineState.h"
#include "EventEngineJob.h"
#include "EngineJobID.h"
#include "fwd/RawEvent_fwd.h"

#include <memory>

namespace STI
{

namespace Device
{

class DeviceTrace;

} //Device


namespace Engine
{

class EventEngineDependencyTree;
class RawEvent;
class EventEngineJob;
class LocalEventEngineJob;
class EngineID;
class EngineJobID;
//class EventEngineJobType;

} //Engine

//EventEngineDependencyTree
template<>
bool Network::convert<Engine::EventEngineDependencyTree, TNetwork::TEventEngineDependencyTree>(const Engine::EventEngineDependencyTree& tree, TNetwork::TEventEngineDependencyTree& tTree);
template<>
bool Network::convert<TNetwork::TEventEngineDependencyTree, Engine::EventEngineDependencyTree>(const TNetwork::TEventEngineDependencyTree& tTree, Engine::EventEngineDependencyTree& tree);


//DeviceTrace
template<> 
Device::DeviceTrace Network::convert<TNetwork::TDeviceTrace, Device::DeviceTrace>(const TNetwork::TDeviceTrace& tDeviceTrace);
template<>
TNetwork::TDeviceTrace Network::convert<Device::DeviceTrace, TNetwork::TDeviceTrace>(const Device::DeviceTrace& deviceTrace);


//EngineState
template<>
bool Network::convert<Engine::EngineState, TNetwork::TEngineState>(const Engine::EngineState& state, TNetwork::TEngineState& tState);
template<>
bool Network::convert<TNetwork::TEngineState, Engine::EngineState>(const TNetwork::TEngineState& tState, Engine::EngineState& state);


//EngineJobID
template<>
bool Network::convert<Engine::EngineJobID, TNetwork::TEngineJobID>(const Engine::EngineJobID& jobID, TNetwork::TEngineJobID& tJobID);
template<>
bool Network::convert<TNetwork::TEngineJobID, Engine::EngineJobID>(const TNetwork::TEngineJobID& tJobID, Engine::EngineJobID& jobID);

template<>
TNetwork::TEngineJobID Network::convert<Engine::EngineJobID, TNetwork::TEngineJobID>(const Engine::EngineJobID& jobID);
template<>
Engine::EngineJobID Network::convert<TNetwork::TEngineJobID, Engine::EngineJobID>(const TNetwork::TEngineJobID& tJobID);


//EventEngineJobType
template<>
TNetwork::TEventEngineJobType Network::convert<Engine::EventEngineJobType, TNetwork::TEventEngineJobType>(const Engine::EventEngineJobType& jobType);
template<>
Engine::EventEngineJobType Network::convert<TNetwork::TEventEngineJobType, Engine::EventEngineJobType>(const TNetwork::TEventEngineJobType& tJobType);


//EngineID
template<>
bool Network::convert<Engine::EngineID, TNetwork::TEngineID>(const Engine::EngineID& engineID, TNetwork::TEngineID& tEngineID);
template<>
bool Network::convert<TNetwork::TEngineID, Engine::EngineID>(const TNetwork::TEngineID& tEngineID, Engine::EngineID& engineID);

template<>
TNetwork::TEngineID Network::convert<Engine::EngineID, TNetwork::TEngineID>(const Engine::EngineID& engineID);
template<>
Engine::EngineID Network::convert<TNetwork::TEngineID, Engine::EngineID>(const TNetwork::TEngineID& tEngineID);



//EngineJobStatus
template<>
bool Network::convert<Engine::EventEngineJob::EngineJobStatus, TNetwork::TEngineJobStatus>(const Engine::EventEngineJob::EngineJobStatus& jobStatus, TNetwork::TEngineJobStatus& tJobStatus);
template<>
bool Network::convert<TNetwork::TEngineJobStatus, Engine::EventEngineJob::EngineJobStatus>(const TNetwork::TEngineJobStatus& tJobStatus, Engine::EventEngineJob::EngineJobStatus& jobStatus);

template<>
TNetwork::TEngineJobStatus Network::convert<Engine::EventEngineJob::EngineJobStatus, TNetwork::TEngineJobStatus>(const Engine::EventEngineJob::EngineJobStatus& jobStatus);
template<>
Engine::EventEngineJob::EngineJobStatus Network::convert<TNetwork::TEngineJobStatus, Engine::EventEngineJob::EngineJobStatus>(const TNetwork::TEngineJobStatus& tJobStatus);



//EventEngineJob

//Note: when converting Shot, this must get the T referene (from the servant) using a dynamic cast; the servant is hosted locally
//Also, engineJob->setEventEngine(...);
// template<> 
// Engine::LocalEventEngineJob Network::convert<TNetwork::TEventEngineJob, Engine::LocalEventEngineJob>(const TNetwork::TEventEngineJob& tEngineJob);
template<>
TNetwork::TEventEngineJob Network::convert<Engine::EventEngineJob, TNetwork::TEventEngineJob>(const Engine::EventEngineJob& engineJob);

template<>
bool Network::convert<Engine::EventEngineJob, TNetwork::TEventEngineJob>(const Engine::EventEngineJob& engineJob, TNetwork::TEventEngineJob& tEngineJob);

template<>
bool Network::convert<std::shared_ptr<Engine::EventEngineJob>, TNetwork::TEventEngineJob>(const std::shared_ptr<Engine::EventEngineJob>& engineJob, TNetwork::TEventEngineJob& tEngineJob);
template<>
bool Network::convert<TNetwork::TEventEngineJob, std::shared_ptr<Engine::EventEngineJob>>(const TNetwork::TEventEngineJob& tEngineJob, std::shared_ptr<Engine::EventEngineJob>& engineJob);


//RawEvent
template<>
bool Network::convert<Engine::RawEvent, TNetwork::TRawEvent>(const Engine::RawEvent& evt, TNetwork::TRawEvent& tEvent);
template<>
bool Network::convert<TNetwork::TRawEvent, Engine::RawEvent>(const TNetwork::TRawEvent& tEvent, Engine::RawEvent& evt);

template<>
TNetwork::TRawEvent Network::convert<Engine::RawEvent, TNetwork::TRawEvent>(const Engine::RawEvent& evt);
template<>
Engine::RawEvent Network::convert<TNetwork::TRawEvent, Engine::RawEvent>(const TNetwork::TRawEvent& tEvent);


//ParseID
template<>
TNetwork::TParseID Network::convert<Engine::ParseID, TNetwork::TParseID>(const Engine::ParseID& pid);
template<>
Engine::ParseID Network::convert<TNetwork::TParseID, Engine::ParseID>(const TNetwork::TParseID& tpid);

//ShotID
template<>
TNetwork::TShotID Network::convert<Engine::ShotID, TNetwork::TShotID>(const Engine::ShotID& sid);
template<>
Engine::ShotID Network::convert<TNetwork::TShotID, Engine::ShotID>(const TNetwork::TShotID& tsid);


//Shot
template<>
bool Network::convert<TNetwork::TParsedShot_ptr, std::shared_ptr<Engine::Shot>>(const TNetwork::TParsedShot_ptr& tShot, std::shared_ptr<Engine::Shot>& shot);
template<>
bool Network::convert<std::shared_ptr<Engine::Shot>, TNetwork::TParsedShot_ptr>(const std::shared_ptr<Engine::Shot>& shot, TNetwork::TParsedShot_ptr& tShot);



//RawEventType
template<>
TNetwork::TRawEventType Network::convert<Engine::RawEventType, TNetwork::TRawEventType>(const Engine::RawEventType& evtType);
template<>
Engine::RawEventType Network::convert<TNetwork::TRawEventType, Engine::RawEventType>(const TNetwork::TRawEventType& tEvtType);


//TimeStamp
template<>
TNetwork::TTimeStamp Network::convert<Engine::TimeStamp, TNetwork::TTimeStamp>(const Engine::TimeStamp& timeStamp);
template<>
Engine::TimeStamp Network::convert<TNetwork::TTimeStamp, Engine::TimeStamp>(const TNetwork::TTimeStamp& tTime);


//EngineJobSourceID
template<>
TNetwork::TEngineJobSourceID Network::convert<Engine::EngineJobSourceID, TNetwork::TEngineJobSourceID>(const Engine::EngineJobSourceID& jobSourceID);
template<>
Engine::EngineJobSourceID Network::convert<TNetwork::TEngineJobSourceID, Engine::EngineJobSourceID>(const TNetwork::TEngineJobSourceID& tJobSourceID);



} //STI

#endif

