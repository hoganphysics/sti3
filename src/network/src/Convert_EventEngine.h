#ifndef STI_NETWORK_CONVERT_EVENTENGINE_H
#define STI_NETWORK_CONVERT_EVENTENGINE_H

#include "NetworkConvert.h"
#include "deviceNet.h"

#include <sti/fwd/RawEvent_fwd.h>

#include <sti/engine/EngineState.h>
#include <sti/engine/EventEngineJob.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineJobStatus.h>
#include <sti/engine/RawEventTarget.h>
#include <sti/engine/RawEventTargetDevice.h>
#include <sti/engine/RawEventTargetChannel.h>

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
class ParsedDependencyTree;
class RawEvent;
class EventEngineJob;
class LocalEventEngineJob;
class EngineID;
class EngineJobID;
class Measurement;
class EngineParsingMessage;
enum class ParsingMessageType;
class ShotResultRecord;
enum class ShotType;
enum class RecordStatus;
enum class EventEngineJobList;
class ParseJobStatus;
class PlayJobStatus;
class AddSequenceStatus;

} //Engine


//EventEngine
template<>
bool Network::convert<TNetwork::TEventEngine_var, std::shared_ptr<Engine::EventEngine>>(
        const TNetwork::TEventEngine_var& tEventEngine, std::shared_ptr<Engine::EventEngine>& eventEngine);
template<>
bool Network::convert<std::shared_ptr<Engine::EventEngine>, TNetwork::TEventEngine_var>(
        const std::shared_ptr<Engine::EventEngine>& eventEngine, TNetwork::TEventEngine_var& tEventEngine);


//EventEngineDependencyTree
template<>
bool Network::convert<Engine::EventEngineDependencyTree, TNetwork::TEventEngineDependencyTree>(
            const Engine::EventEngineDependencyTree& tree, TNetwork::TEventEngineDependencyTree& tTree);
template<>
bool Network::convert<TNetwork::TEventEngineDependencyTree, Engine::EventEngineDependencyTree>(
            const TNetwork::TEventEngineDependencyTree& tTree, Engine::EventEngineDependencyTree& tree);

//ParsedDependencyTree
template<>
bool Network::convert<std::shared_ptr<Engine::ParsedDependencyTree>, TNetwork::TEventEngineDependencyTree>(
            const std::shared_ptr<Engine::ParsedDependencyTree>& tree, TNetwork::TEventEngineDependencyTree& tTree);
template<>
bool Network::convert<TNetwork::TEventEngineDependencyTree, std::shared_ptr<Engine::ParsedDependencyTree>>(
            const TNetwork::TEventEngineDependencyTree& tTree, std::shared_ptr<Engine::ParsedDependencyTree>& tree);



//EngineState
template<>
bool Network::convert<Engine::EngineState, TNetwork::TEngineState>(const Engine::EngineState& state, TNetwork::TEngineState& tState);
template<>
bool Network::convert<TNetwork::TEngineState, Engine::EngineState>(const TNetwork::TEngineState& tState, Engine::EngineState& state);
template<>
TNetwork::TEngineState Network::convert<Engine::EngineState, TNetwork::TEngineState>(const Engine::EngineState& state);
template<>
Engine::EngineState STI::Network::convert<TNetwork::TEngineState, Engine::EngineState>(const TNetwork::TEngineState& tState);


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
bool Network::convert<Engine::EngineJobStatus, TNetwork::TEngineJobStatus>(const Engine::EngineJobStatus& jobStatus, TNetwork::TEngineJobStatus& tJobStatus);
template<>
bool Network::convert<TNetwork::TEngineJobStatus, Engine::EngineJobStatus>(const TNetwork::TEngineJobStatus& tJobStatus, Engine::EngineJobStatus& jobStatus);

template<>
TNetwork::TEngineJobStatus Network::convert<Engine::EngineJobStatus, TNetwork::TEngineJobStatus>(const Engine::EngineJobStatus& jobStatus);
template<>
Engine::EngineJobStatus Network::convert<TNetwork::TEngineJobStatus, Engine::EngineJobStatus>(const TNetwork::TEngineJobStatus& tJobStatus);



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

template<>
TNetwork::TEventEngineJob Network::convert<std::shared_ptr<Engine::EventEngineJob>, TNetwork::TEventEngineJob>(const std::shared_ptr<Engine::EventEngineJob>& engineJob);
template<>
std::shared_ptr<Engine::EventEngineJob> Network::convert<TNetwork::TEventEngineJob, std::shared_ptr<Engine::EventEngineJob>>(const TNetwork::TEventEngineJob& tEngineJob);


//EventEngineJobList
template<>
TNetwork::TEventEngineJobList Network::convert<Engine::EventEngineJobList, TNetwork::TEventEngineJobList>(const Engine::EventEngineJobList& jobList);
template<>
Engine::EventEngineJobList Network::convert<TNetwork::TEventEngineJobList, Engine::EventEngineJobList>(const TNetwork::TEventEngineJobList& tJobList);



//RawEventTarget
template<>
TNetwork::TRawEventTarget Network::convert<Engine::RawEventTarget, TNetwork::TRawEventTarget>(const Engine::RawEventTarget& target);
template<>
Engine::RawEventTarget Network::convert<TNetwork::TRawEventTarget, Engine::RawEventTarget>(const TNetwork::TRawEventTarget& tTarget);

//RawEventTargetDevice
template<>
bool Network::convert<Engine::RawEventTargetDevice, TNetwork::TRawEventTargetDevice>(const Engine::RawEventTargetDevice& targetDevice, TNetwork::TRawEventTargetDevice& tTargetDevice);
template<>
Engine::RawEventTargetDevice Network::convert<TNetwork::TRawEventTargetDevice, Engine::RawEventTargetDevice>(const TNetwork::TRawEventTargetDevice& tTargetDevice);

//RawEventTargetChannel
template<>
bool Network::convert<Engine::RawEventTargetChannel, TNetwork::TRawEventTargetChannel>(const Engine::RawEventTargetChannel& targetChannel, TNetwork::TRawEventTargetChannel& tTargetChannel);
template<>
Engine::RawEventTargetChannel Network::convert<TNetwork::TRawEventTargetChannel, Engine::RawEventTargetChannel>(const TNetwork::TRawEventTargetChannel& tTargetChannel);


//RawEvent
template<>
bool Network::convert<Engine::RawEvent, TNetwork::TRawEvent>(const Engine::RawEvent& evt, TNetwork::TRawEvent& tEvent);
template<>
bool Network::convert<TNetwork::TRawEvent, Engine::RawEvent>(const TNetwork::TRawEvent& tEvent, Engine::RawEvent& evt);

template<>
TNetwork::TRawEvent Network::convert<Engine::RawEvent, TNetwork::TRawEvent>(const Engine::RawEvent& evt);
template<>
Engine::RawEvent Network::convert<TNetwork::TRawEvent, Engine::RawEvent>(const TNetwork::TRawEvent& tEvent);


// //STI::Engine::DeviceEventMap
// template<>
// bool Network::convert<Engine::DeviceEventMap, TNetwork::TDeviceEventsSeq>(const Engine::DeviceEventMap& deviceEvents, TNetwork::TDeviceEventsSeq& tDeviceEvents);
// template<>
// bool Network::convert<TNetwork::TDeviceEventsSeq, Engine::DeviceEventMap>(const TNetwork::TDeviceEventsSeq& tDeviceEvents, Engine::DeviceEventMap& deviceEvents);




//TShotConfig
template<>
TNetwork::TShotConfig Network::convert<Engine::ShotConfig, TNetwork::TShotConfig>(const Engine::ShotConfig& shotConfig);
template<>
Engine::ShotConfig Network::convert<TNetwork::TShotConfig, Engine::ShotConfig>(const TNetwork::TShotConfig& tShotConfig);

//TRecordStatus
template<>
TNetwork::TRecordStatus Network::convert<Engine::RecordStatus, TNetwork::TRecordStatus>(const Engine::RecordStatus& recordStatus);
template<>
Engine::RecordStatus Network::convert<TNetwork::TRecordStatus, Engine::RecordStatus>(const TNetwork::TRecordStatus& tRecordStatus);

//TShotResultRecord
template<>
TNetwork::TShotResultRecord Network::convert<Engine::ShotResultRecord, TNetwork::TShotResultRecord>(const Engine::ShotResultRecord& shotResultRecord);
template<>
Engine::ShotResultRecord Network::convert<TNetwork::TShotResultRecord, Engine::ShotResultRecord>(const TNetwork::TShotResultRecord& tShotResultRecord);

template<>
bool Network::convert<Engine::ShotResultRecord, TNetwork::TShotResultRecord>(const Engine::ShotResultRecord& shotResultRecord, TNetwork::TShotResultRecord& tShotResultRecord);
template<>
bool Network::convert<TNetwork::TShotResultRecord, Engine::ShotResultRecord>(const TNetwork::TShotResultRecord& tShotResultRecord, Engine::ShotResultRecord& shotResultRecord);


//ShotType
template<>
TNetwork::TShotType Network::convert<Engine::ShotType, TNetwork::TShotType>(const Engine::ShotType& shotType);
template<>
Engine::ShotType Network::convert<TNetwork::TShotType, Engine::ShotType>(const TNetwork::TShotType& tShotType);


//ParseID
template<>
TNetwork::TParseID Network::convert<Engine::ParseID, TNetwork::TParseID>(const Engine::ParseID& pid);
template<>
Engine::ParseID Network::convert<TNetwork::TParseID, Engine::ParseID>(const TNetwork::TParseID& tpid);

template<>
bool Network::convert<Engine::ParseID, TNetwork::TParseID>(const Engine::ParseID& pid, TNetwork::TParseID& tpid);
template<>
bool Network::convert<TNetwork::TParseID, Engine::ParseID>(const TNetwork::TParseID& tpid, Engine::ParseID& pid);


//ParseJobStatus
template<>
TNetwork::TParseJobStatus Network::convert<Engine::ParseJobStatus, TNetwork::TParseJobStatus>(const Engine::ParseJobStatus& parseJobStatus);
template<>
Engine::ParseJobStatus Network::convert<TNetwork::TParseJobStatus, Engine::ParseJobStatus>(const TNetwork::TParseJobStatus& tParseJobStatus);
template<>
bool Network::convert<Engine::ParseJobStatus, TNetwork::TParseJobStatus>(const Engine::ParseJobStatus& parseJobStatus, TNetwork::TParseJobStatus& tParseJobStatus);
template<>
bool Network::convert<TNetwork::TParseJobStatus, Engine::ParseJobStatus>(const TNetwork::TParseJobStatus& tParseJobStatus, Engine::ParseJobStatus& parseJobStatus);



//ShotID
template<>
TNetwork::TShotID Network::convert<Engine::ShotID, TNetwork::TShotID>(const Engine::ShotID& sid);
template<>
Engine::ShotID Network::convert<TNetwork::TShotID, Engine::ShotID>(const TNetwork::TShotID& tsid);

template<>
bool Network::convert<Engine::ShotID, TNetwork::TShotID>(const Engine::ShotID& sid, TNetwork::TShotID& tsid);
template<>
bool Network::convert<TNetwork::TShotID, Engine::ShotID>(const TNetwork::TShotID& tsid, Engine::ShotID& sid);


//PlayJobStatus
template<>
TNetwork::TPlayJobStatus Network::convert<Engine::PlayJobStatus, TNetwork::TPlayJobStatus>(const Engine::PlayJobStatus& playJobStatus);
template<>
Engine::PlayJobStatus Network::convert<TNetwork::TPlayJobStatus, Engine::PlayJobStatus>(const TNetwork::TPlayJobStatus& tPlayJobStatus);
template<>
bool Network::convert<Engine::PlayJobStatus, TNetwork::TPlayJobStatus>(const Engine::PlayJobStatus& playJobStatus, TNetwork::TPlayJobStatus& tPlayJobStatus);
template<>
bool Network::convert<TNetwork::TPlayJobStatus, Engine::PlayJobStatus>(const TNetwork::TPlayJobStatus& tPlayJobStatus, Engine::PlayJobStatus& playJobStatus);



//AddSequenceStatus
template<>
TNetwork::TAddSequenceStatus Network::convert<Engine::AddSequenceStatus, TNetwork::TAddSequenceStatus>(
                const Engine::AddSequenceStatus& addSequenceStatus);
template<>
Engine::AddSequenceStatus Network::convert<TNetwork::TAddSequenceStatus, Engine::AddSequenceStatus>(
                const TNetwork::TAddSequenceStatus& tAddSequenceStatus);
template<>
bool Network::convert<Engine::AddSequenceStatus, TNetwork::TAddSequenceStatus>(const Engine::AddSequenceStatus& addSequenceStatus, TNetwork::TAddSequenceStatus& tAddSequenceStatus);
template<>
bool Network::convert<TNetwork::TAddSequenceStatus, Engine::AddSequenceStatus>(const TNetwork::TAddSequenceStatus& tAddSequenceStatus, Engine::AddSequenceStatus& addSequenceStatus);


//Shot
template<>
bool Network::convert<TNetwork::TShot, std::shared_ptr<Engine::Shot>>(const TNetwork::TShot& tShot, std::shared_ptr<Engine::Shot>& shot);
template<>
bool Network::convert<std::shared_ptr<Engine::Shot>, TNetwork::TShot>(const std::shared_ptr<Engine::Shot>& shot, TNetwork::TShot& tShot);



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


//EngineParsingMessage
template<>
bool Network::convert<Engine::EngineParsingMessage, TNetwork::TEngineParsingMessage>(const Engine::EngineParsingMessage& parsingMessage, TNetwork::TEngineParsingMessage& tParsingMessage);
template<>
bool Network::convert<TNetwork::TEngineParsingMessage, Engine::EngineParsingMessage>(const TNetwork::TEngineParsingMessage& tParsingMessage, Engine::EngineParsingMessage& parsingMessage);

template<>
Engine::EngineParsingMessage Network::convert<TNetwork::TEngineParsingMessage, Engine::EngineParsingMessage>(const TNetwork::TEngineParsingMessage& tParsingMessage);



//ParsingMessageType
template<>
TNetwork::TParsingMessageType Network::convert<Engine::ParsingMessageType, TNetwork::TParsingMessageType>(const Engine::ParsingMessageType& messType);
template<>
Engine::ParsingMessageType Network::convert<TNetwork::TParsingMessageType, Engine::ParsingMessageType>(const TNetwork::TParsingMessageType& tMessType);


//Measurement
template<>
bool Network::convert<std::shared_ptr<Engine::Measurement>, TNetwork::TMeasurement>(
        const std::shared_ptr<Engine::Measurement>& measurement, TNetwork::TMeasurement& tMeasurement);
template<>
bool Network::convert<TNetwork::TMeasurement, std::shared_ptr<Engine::Measurement>>(
        const TNetwork::TMeasurement& tMeasurement, std::shared_ptr<Engine::Measurement>& measurement);

template<>
TNetwork::TMeasurement Network::convert<std::shared_ptr<Engine::Measurement>, TNetwork::TMeasurement>(
        const std::shared_ptr<Engine::Measurement>& measurement);
template<>
std::shared_ptr<Engine::Measurement> Network::convert<TNetwork::TMeasurement, std::shared_ptr<Engine::Measurement>>(
        const TNetwork::TMeasurement& tMeasurement);



} //STI

#endif

