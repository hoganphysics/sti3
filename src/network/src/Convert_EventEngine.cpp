
#include "NetworkConvert.h"
#include "Convert_EventEngine.h"
#include "Convert_DeviceTrace.h"

#include "DeviceTrace.h"
#include "EventEngineDependencyTree.h"
#include "EngineJobID.h"

#include "EngineID.h"
#include "EventEngineJob.h"
#include "LocalEventEngineJob.h"
#include "RawEvent.h"
#include "ParseID.h"
#include "ShotID.h"
#include "Shot.h"
#include "NetworkShotWrapper.h"
#include "RemoteShot.h"
#include "utils/GraphPathLabel.h"
#include "EventEngine.h"
#include "TimeStamp.h"
#include "EngineParsingMessage.h"
#include "ParsedDependencyTree.h"
#include "Measurement.h"
#include "NetworkEventEngine.h"
#include "RemoteEventEngine.h"


#include <map>
#include <memory>

using STI::TNetwork::TEventEngine_var;
using STI::Engine::EventEngine;
using STI::Engine::ParsedDependencyTree;
using STI::Engine::EventEngineDependencyTree;
using STI::TNetwork::TEventEngineDependencyTree;
using STI::Engine::EngineState;
using STI::TNetwork::TEngineState;
using STI::Engine::EngineJobID;
using STI::TNetwork::TEngineJobID;
using STI::Engine::EventEngineJobType;
using STI::TNetwork::TEventEngineJobType;
using STI::Engine::EngineID;
using STI::TNetwork::TEngineID;
using STI::Engine::EventEngineJob;
using STI::Engine::EngineJobStatus;
using STI::TNetwork::TEngineJobStatus;
using STI::Engine::EventEngineJob;
using STI::TNetwork::TEventEngineJob;
using STI::Engine::RawEvent;
using STI::TNetwork::TRawEvent;
using STI::Engine::ParseID;
using STI::TNetwork::TParseID;
using STI::Engine::ShotID;
using STI::TNetwork::TShotID;
using STI::Network::NetworkShotWrapper;
using STI::Engine::EventEngine;
using STI::Engine::LocalEventEngineJob;
using STI::Engine::RawEventType;
using STI::TNetwork::TRawEventType;
using STI::Engine::TimeStamp;
using STI::TNetwork::TTimeStamp;
using STI::Engine::EngineJobSourceID;
using STI::TNetwork::TEngineJobSourceID;
using STI::TNetwork::TShot_ptr;
using STI::Engine::Shot;
using STI::Engine::EngineParsingMessage;
using STI::TNetwork::TEngineParsingMessage;
using STI::Engine::ParsingMessageType;
using STI::TNetwork::TParsingMessageType;
using STI::Engine::DeviceEventMap;
using STI::TNetwork::TDeviceEventsSeq;
using STI::Engine::ParseID; 
using STI::TNetwork::TParseID;
using STI::Engine::ShotID;
using STI::TNetwork::TShotID;
using STI::TNetwork::TMeasurement;
using STI::Engine::Measurement;


bool convertEventGraphPath(const STI::Utils::GraphPathLabel& graphPath, ::STI::TNetwork::TGraphPathLabel& tGraphPath);
bool convertEventGraphPath(const ::STI::TNetwork::TGraphPathLabel& tGraphPath, STI::Utils::GraphPathLabel& graphPath);




//EventEngine
template<>
bool STI::Network::convert<TEventEngine_var, std::shared_ptr<EventEngine>>(
        const TEventEngine_var& tEventEngine, std::shared_ptr<EventEngine>& eventEngine)
{
    if (!CORBA::is_nil(tEventEngine)) {
        eventEngine = std::make_shared<STI::Network::RemoteEventEngine>(tEventEngine);
        return (eventEngine != 0);
    }
    return false;
}

template<>
bool STI::Network::convert<std::shared_ptr<EventEngine>, TEventEngine_var>(
        const std::shared_ptr<EventEngine>& eventEngine, TEventEngine_var& tEventEngine)
{
	return STI::Network::NetworkEventEngine::getTEventEngineReference(eventEngine, tEventEngine);
}



//EventEngineDependencyTree
template<>
bool STI::Network::convert<EventEngineDependencyTree, TEventEngineDependencyTree>(const EventEngineDependencyTree& tree, TEventEngineDependencyTree& tTree)
{
    std::vector<STI::Device::DeviceID> nodes;
    tree.getNodes(nodes);

    //setup vertexMap
    std::map<STI::Device::DeviceID, unsigned> vertexMap;
    for (unsigned i = 0; i < nodes.size(); ++i) {
        vertexMap[nodes.at(i)] = i;
    }
    
    std::vector<STI::Device::DeviceID> outNodes;

    tTree.vertices.length(static_cast<CORBA::ULong>(nodes.size()));

    for (unsigned i = 0; i < nodes.size(); ++i) {
        convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(nodes.at(i), tTree.vertices[i].id);
        
        tree.getDependedentNodes(nodes.at(i), outNodes);

        tTree.vertices[i].outConnections.length(static_cast<CORBA::ULong>(outNodes.size()));

        for (unsigned j = 0; j < outNodes.size(); ++j) {
            tTree.vertices[i].outConnections[j] = vertexMap[outNodes.at(j)];
        }
    }

    return true;
}

template<>
bool STI::Network::convert<TEventEngineDependencyTree, EventEngineDependencyTree>(
            const TEventEngineDependencyTree& tTree, EventEngineDependencyTree& tree)
{
    std::vector<STI::Device::DeviceID> nodes;
    
    for (unsigned i = 0; i < tTree.vertices.length(); ++i) {
        
        STI::Device::DeviceID deviceID = convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tTree.vertices[i].id);
        
        nodes.push_back(deviceID);      
    }

    for (unsigned i = 0; i < nodes.size(); ++i) {

        tree.addVertex(nodes.at(i));

        for (unsigned j = 0; j < tTree.vertices[i].outConnections.length(); ++j) {
            tree.addEdge(nodes.at(i), nodes.at(tTree.vertices[i].outConnections[j]));
        }
    }

    return true;
}

//ParsedDependencyTree
template<>
bool STI::Network::convert<std::shared_ptr<ParsedDependencyTree>, TEventEngineDependencyTree>(
                    const std::shared_ptr<ParsedDependencyTree>& tree, TEventEngineDependencyTree& tTree)
{
    if (tree == 0) return false;

    std::vector<STI::Device::DeviceID> nodes;
    tree->getNodes(nodes);

    //setup vertexMap
    std::map<STI::Device::DeviceID, unsigned> vertexMap;
    for (unsigned i = 0; i < nodes.size(); ++i) {
        vertexMap[nodes.at(i)] = i;
    }
    
    std::vector<STI::Device::DeviceID> outNodes;

    tTree.vertices.length(static_cast<CORBA::ULong>(nodes.size()));

    for (unsigned i = 0; i < nodes.size(); ++i) {
        convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(nodes.at(i), tTree.vertices[i].id);
        
        tree->getDependedentNodes(nodes.at(i), outNodes);

        tTree.vertices[i].outConnections.length(static_cast<CORBA::ULong>(outNodes.size()));

        for (unsigned j = 0; j < outNodes.size(); ++j) {
            tTree.vertices[i].outConnections[j] = vertexMap[outNodes.at(j)];
        }
    }

    return true;
}

template<>
bool STI::Network::convert<TEventEngineDependencyTree, std::shared_ptr<ParsedDependencyTree>>(
                    const TEventEngineDependencyTree& tTree, std::shared_ptr<ParsedDependencyTree>& tree)
{
    auto depTree = std::make_shared<EventEngineDependencyTree>();

    if (convert<TEventEngineDependencyTree, EventEngineDependencyTree>(tTree, *depTree) ) {
        tree = std::make_shared<ParsedDependencyTree>(depTree);
        return (tree != 0);
    }

    return false;
}


//EngineState
template<>
bool STI::Network::convert<EngineState, TEngineState>(const EngineState& state, TEngineState& tState)
{
    switch (state)
    {
		case EngineState::Idle:
			tState = TEngineState::EngineIdle;
			break;
		case EngineState::Parsing:
			tState = TEngineState::EngineParsing;
			break;
		case EngineState::Parsed:
			tState = TEngineState::EngineParsed;
			break;
		case EngineState::PreparingPlay:
			tState = TEngineState::EnginePreparingPlay;
			break;
		case EngineState::PlayReady:
			tState = TEngineState::EnginePlayReady;
			break;
		case EngineState::WaitingForTrigger:
			tState = TEngineState::EngineWaitingForTrigger;
			break;
		case EngineState::Playing:
			tState = TEngineState::EnginePlaying;
			break;
		case EngineState::Paused:
			tState = TEngineState::EnginePaused;
			break;
		case EngineState::Missing:
			tState = TEngineState::EngineMissing;
			break;
		case EngineState::Error:
			tState = TEngineState::EngineError;
			break;
        default:
			tState = TEngineState::EngineUnknown;
			break;
        }

    return true;
}

template<>
bool STI::Network::convert<TEngineState, EngineState>(const TEngineState& tState, EngineState& state)
{
    switch (tState)
    {
		case TEngineState::EngineIdle:
			state = EngineState::Idle;
			break;
		case TEngineState::EngineParsing:
			state = EngineState::Parsing;
			break;
		case TEngineState::EngineParsed:
			state = EngineState::Parsed;
			break;
		case TEngineState::EnginePreparingPlay:
			state = EngineState::PreparingPlay;
			break;
		case TEngineState::EnginePlayReady:
			state = EngineState::PlayReady;
			break;
		case TEngineState::EngineWaitingForTrigger:
			state = EngineState::WaitingForTrigger;
			break;
		case TEngineState::EnginePlaying:
			state = EngineState::Playing;
			break;
		case TEngineState::EnginePaused:
			state = EngineState::Paused;
			break;
		case TEngineState::EngineMissing:
			state = EngineState::Missing;
			break;
		case TEngineState::EngineError:
			state = EngineState::Error;
			break;
        default:
			state = EngineState::Unknown;
			break;
        }

    return true;
}

template<>
TEngineState STI::Network::convert<EngineState, TEngineState>(const EngineState& state)
{
    TEngineState tState;
    convert<EngineState, TEngineState>(state, tState);
    return tState;
}

template<>
EngineState STI::Network::convert<TEngineState, EngineState>(const TEngineState& tState)
{
    EngineState state;
    convert<TEngineState, EngineState>(tState, state);
    return state;
}


//EngineJobID
template<>
bool STI::Network::convert<EngineJobID, TEngineJobID>(const EngineJobID& jobID, TEngineJobID& tJobID)
{
    tJobID.type = convert<EventEngineJobType, TEventEngineJobType>(jobID.type);
    tJobID.pid = convert<ParseID, TParseID>(jobID.pid);
    tJobID.sid = convert<ShotID, TShotID>(jobID.sid);

    return true;
}

template<>
bool STI::Network::convert<TEngineJobID, EngineJobID>(const TEngineJobID& tJobID, EngineJobID& jobID)
{
    jobID.type = convert<TEventEngineJobType, EventEngineJobType>(tJobID.type);
    jobID.pid = convert<TParseID, ParseID>(tJobID.pid);
    jobID.sid = convert<TShotID, ShotID>(tJobID.sid);

    return true;
}


template<>
TEngineJobID STI::Network::convert<EngineJobID, TEngineJobID>(const EngineJobID& jobID)
{
	TEngineJobID tJobID;

	convert<EngineJobID, TEngineJobID>(jobID, tJobID);

	return tJobID;
}

template<>
EngineJobID STI::Network::convert<TEngineJobID, EngineJobID>(const TNetwork::TEngineJobID& tJobID)
{
	EngineJobID jobID;

	convert<TEngineJobID, EngineJobID>(tJobID, jobID);

	return jobID;
}


//EventEngineJobType
template<>
TEventEngineJobType STI::Network::convert<EventEngineJobType, TEventEngineJobType>(const EventEngineJobType& jobType)
{
    TEventEngineJobType tJobType;

    switch (jobType) 
    {
        case EventEngineJobType::Parse:
            tJobType = TEventEngineJobType::EngineJobParse;
            break;
        case EventEngineJobType::Play:
            tJobType = TEventEngineJobType::EngineJobPlay;
            break;
        default:
            tJobType = TEventEngineJobType::EngineJobParse;
            break;
    }

    return tJobType;
}

template<>
EventEngineJobType STI::Network::convert<TEventEngineJobType, EventEngineJobType>(const TEventEngineJobType& tJobType)
{
    EventEngineJobType jobType;

    switch (tJobType) 
    {
        case TEventEngineJobType::EngineJobParse:
            jobType = EventEngineJobType::Parse;
            break;
        case TEventEngineJobType::EngineJobPlay:
            jobType = EventEngineJobType::Play;
            break;
        default:
            jobType = EventEngineJobType::Parse;
            break;
    }

    return jobType;
}


//EngineID
template<>
bool STI::Network::convert<EngineID, TEngineID>(const EngineID& engineID, TEngineID& tEngineID)
{
    tEngineID.engineNumber = static_cast<CORBA::Long>(engineID.getNumber());

    return true;
}

template<>
bool STI::Network::convert<TEngineID, EngineID>(const TEngineID& tEngineID, EngineID& engineID)
{
    EngineID id(tEngineID.engineNumber);
    //engineID.setNumber( static_cast<short>(tEngineID.engineNumber) );
    engineID = id;

    return true;
}

template<>
TEngineID STI::Network::convert<EngineID, TEngineID>(const EngineID& engineID)
{
	TEngineID tEngineID;
	convert<EngineID, TEngineID>(engineID, tEngineID);
	return tEngineID;
}

template<>
EngineID STI::Network::convert<TEngineID, EngineID>(const TEngineID& tEngineID)
{
	EngineID engineID;
	convert<TEngineID, EngineID>(tEngineID, engineID);
	return engineID;
}



//EngineJobStatus
template<>
bool STI::Network::convert<EngineJobStatus, TEngineJobStatus>(const EngineJobStatus& jobStatus, TEngineJobStatus& tJobStatus)
{
    switch (jobStatus)
    {
    case EngineJobStatus::New:
        tJobStatus = TEngineJobStatus::New;
        break;
    case EngineJobStatus::Running:
        tJobStatus = TEngineJobStatus::Running;
        break;
    case EngineJobStatus::Completed:
        tJobStatus = TEngineJobStatus::Completed;
        break;
    case EngineJobStatus::Canceled:
        tJobStatus = TEngineJobStatus::Canceled;
        break;
    default:
        tJobStatus = TEngineJobStatus::New;
        break;
    }
    return true;
}

template<>
bool STI::Network::convert<TEngineJobStatus, EngineJobStatus>(const TEngineJobStatus& tJobStatus, EngineJobStatus& jobStatus)
{
    
    switch (tJobStatus)
    {
    case TEngineJobStatus::New:
        jobStatus = EngineJobStatus::New;
        break;
    case TEngineJobStatus::Running:
        jobStatus = EngineJobStatus::Running;
        break;
    case TEngineJobStatus::Completed:
        jobStatus = EngineJobStatus::Completed;
        break;
    case TEngineJobStatus::Canceled:
        jobStatus = EngineJobStatus::Canceled;
        break;
    default:
        jobStatus = EngineJobStatus::New;
        break;
    }
    return true;
}

template<>
TEngineJobStatus STI::Network::convert<EngineJobStatus, TEngineJobStatus>(const EngineJobStatus& jobStatus)
{
	TEngineJobStatus tStatus;

	convert<EngineJobStatus, TEngineJobStatus>(jobStatus, tStatus);

	return tStatus;
}

template<>
EngineJobStatus STI::Network::convert<TEngineJobStatus, EngineJobStatus>(const TEngineJobStatus& tJobStatus)
{
	EngineJobStatus status;

	convert<TEngineJobStatus, EngineJobStatus>(tJobStatus, status);

	return status;
}




//EventEngineJob
template<>
bool STI::Network::convert<std::shared_ptr<EventEngineJob>, TEventEngineJob>(const std::shared_ptr<EventEngineJob>& engineJob, TEventEngineJob& tEngineJob)
{
	if (engineJob != 0) {
		return convert<EventEngineJob, TEventEngineJob>(*engineJob, tEngineJob);
	}

	return false;
}

template<>
bool STI::Network::convert<EventEngineJob, TEventEngineJob>(const EventEngineJob& engineJob, TEventEngineJob& tEngineJob)
{
    tEngineJob.jobID = convert<EngineJobID, TEngineJobID>(engineJob.getJobID());
    tEngineJob.jobOwner = convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(engineJob.getJobOwner());
    tEngineJob.status = convert<EngineJobStatus, TEngineJobStatus>(engineJob.getStatus());
    tEngineJob.engineID = convert<EngineID, TEngineID>(engineJob.getEngineID());
    
    //Play events do not have a parsedShot or a EventEngineDependencyTree, so these will be null

    std::shared_ptr<Shot> shot;
    STI::TNetwork::TShot_ptr tShot;
    
    if (engineJob.getShot(shot) && TShotRefInterface::getTShotReference(shot, tShot)) {

        tEngineJob.shot = tShot;
    }

    std::shared_ptr<EventEngineDependencyTree> tree;
    
    if (engineJob.getDependencies(tree)) {
        convert<EventEngineDependencyTree, TEventEngineDependencyTree>(*tree, tEngineJob.dependencies);
    }

    convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(engineJob.getMissingTargetIDs(), tEngineJob.missingTargetIDs);

    // tEngineJob.eventEngine;
    // std::shared_ptr<EventEngine> engine;
    // if (engineJob->getEngine(engine)) {
    // }

    return true;
}

template<>
bool STI::Network::convert<TEventEngineJob, std::shared_ptr<EventEngineJob>>(const TEventEngineJob& tEngineJob, std::shared_ptr<EventEngineJob>& engineJob)
{

    EngineJobID jobID = convert<TEngineJobID, EngineJobID>(tEngineJob.jobID);

    std::shared_ptr<Shot> parsedShot;
    std::shared_ptr<EventEngineDependencyTree> tree;
    std::set<STI::Device::DeviceID> missingTargets;     //empty

    switch (jobID.type)
    {
    case EventEngineJobType::Parse:
        {
        if (!CORBA::is_nil(tEngineJob.shot)) {
            parsedShot = std::make_shared<STI::Network::RemoteShot>(tEngineJob.shot);
        }

        tree = std::make_shared<EventEngineDependencyTree>();
        convert<TEventEngineDependencyTree, EventEngineDependencyTree>(tEngineJob.dependencies, *tree);

		auto job = std::make_shared<LocalEventEngineJob>(jobID.pid, parsedShot,
			convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tEngineJob.jobOwner)
			);
        
        job->setDependencies(tree);
        job->setMissingTargets(missingTargets);
        engineJob = job;
        }
        break;
	case EventEngineJobType::Play:
		engineJob = std::make_shared<LocalEventEngineJob>(jobID,
			convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tEngineJob.jobOwner));
		break;
    default:
        break;
    }
    
    return true;
}

template<>
TEventEngineJob STI::Network::convert<EventEngineJob, TEventEngineJob>(const EventEngineJob& engineJob)
{
    TEventEngineJob tJob;

    convert<EventEngineJob, TEventEngineJob>(engineJob, tJob);

    return tJob;
}

//GraphPathLabel
bool convertEventGraphPath(const STI::Utils::GraphPathLabel& graphPath, ::STI::TNetwork::TGraphPathLabel& tGraphPath)
{
    tGraphPath.length(static_cast<CORBA::ULong>(graphPath.size()));

    for (unsigned i = 0; i < graphPath.size(); ++i) {
        tGraphPath[i] = static_cast<CORBA::ULong>(graphPath.at(i));
    }
    return true;
}

bool convertEventGraphPath(const ::STI::TNetwork::TGraphPathLabel& tGraphPath, STI::Utils::GraphPathLabel& graphPath)
{
    for (unsigned i = 0; i < tGraphPath.length(); ++i) {
         graphPath.push_back( static_cast<unsigned>(tGraphPath[i]) );
    }

    return true;
}

//RawEvent
template<>
bool STI::Network::convert<RawEvent, TRawEvent>(const RawEvent& evt, TRawEvent& tEvent)
{
    tEvent.time = static_cast<CORBA::Double>(evt.time());
    tEvent.channel = static_cast<CORBA::UShort>(evt.channel());
    convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(evt.value(), tEvent.value);
    convert<std::string, ::CORBA::String_member>(evt.description(), tEvent.description);
    //trace
    tEvent.isMeasurement = static_cast<CORBA::Boolean>(evt.isMeasurementEvent());
    tEvent.rawEventType = convert<RawEventType, TRawEventType>(evt.type());
    convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(evt.targetDevice(), tEvent.targetDeviceID);
    convertEventGraphPath(evt.getEventGraphPath(), tEvent.eventGraphPath);

    return true;
}


template<>
bool STI::Network::convert<TRawEvent, RawEvent>(const TRawEvent& tEvent, RawEvent& evt)
{
    evt.setTargetID(convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tEvent.targetDeviceID));
	evt.setTime(static_cast<double>(tEvent.time));
	evt.setChannel(static_cast<unsigned short>(tEvent.channel));
	evt.setValue(convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(tEvent.value));
	evt.setDescription(convert<::CORBA::String_member, std::string>(tEvent.description));
	evt.setEventType(convert<TRawEventType, RawEventType>(tEvent.rawEventType));

    STI::Utils::GraphPathLabel gpl;
    convertEventGraphPath(tEvent.eventGraphPath, gpl);
	evt.setEventGraphPath(gpl);

    return true;
}

template<>
TRawEvent STI::Network::convert<RawEvent, TRawEvent>(const RawEvent& evt)
{
    TRawEvent tEvent;

    convert<RawEvent, TRawEvent>(evt, tEvent);

    return tEvent;
}

template<>
RawEvent STI::Network::convert<TRawEvent, RawEvent>(const TRawEvent& tEvent)
{
    RawEvent evt;

    convert<TRawEvent, RawEvent>(tEvent, evt);

    return evt;
}


//STI::Engine::DeviceEventMap
template<>
bool STI::Network::convert<DeviceEventMap, TDeviceEventsSeq>(const DeviceEventMap& deviceEvents, TDeviceEventsSeq& tDeviceEvents)
{
    tDeviceEvents.length(deviceEvents.size());

    unsigned i = 0;

    for (auto& targetEvents : deviceEvents) {
        tDeviceEvents[i].targetDeviceID = convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(targetEvents.first);
        
        convert<RawEvent, TRawEvent>(targetEvents.second, tDeviceEvents[i].events);

        i++;
    }
    return true;
}


template<>
bool STI::Network::convert<TDeviceEventsSeq, DeviceEventMap>(const TDeviceEventsSeq& tDeviceEvents, DeviceEventMap& deviceEvents)
{
    deviceEvents.clear();

    for (unsigned i = 0; i < tDeviceEvents.length(); ++i) {
        //get RawEventVector for this DeviceID
        auto& evts = deviceEvents[convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tDeviceEvents[i].targetDeviceID)];
        
        //populate vector with converted events
        convert<TRawEvent, RawEvent>(tDeviceEvents[i].events, evts);
    }
    return true;
}



//ParseID
template<>
TParseID STI::Network::convert<ParseID, TParseID>(const ParseID& pid)
{
    TParseID tParseID;

    tParseID.parseTimestamp = convert<TimeStamp, TTimeStamp>(pid.parseTimestamp);
    tParseID.jobSourceID = convert<EngineJobSourceID, TEngineJobSourceID>(pid.jobSourceID);
    tParseID.file = convert<std::string, ::CORBA::String_member>(pid.file);
    tParseID.comment = convert<std::string, ::CORBA::String_member>(pid.comment);

    return tParseID;
}

template<>
ParseID STI::Network::convert<TParseID, ParseID>(const TParseID& tpid)
{
    ParseID parseID;

    parseID.parseTimestamp = convert<TTimeStamp, TimeStamp>(tpid.parseTimestamp);
    parseID.jobSourceID = convert<TEngineJobSourceID, EngineJobSourceID>(tpid.jobSourceID);
    parseID.file = convert<::CORBA::String_member, std::string>(tpid.file);
    parseID.comment = convert<::CORBA::String_member, std::string>(tpid.comment);

    return parseID;
}


//ShotID
template<>
TShotID STI::Network::convert<ShotID, TShotID>(const ShotID& sid)
{
    TShotID tsid;
    convert<ShotID, TShotID>(sid, tsid);

    // tShot.parseID = convert<ParseID, TParseID>(sid.parseID);

    // tShot.submissionTime = convert<TimeStamp, TTimeStamp>(sid.submissionTime);
    // tShot.playTime = convert<TimeStamp, TTimeStamp>(sid.playTime);
    
    // tShot.jobSourceID = convert<EngineJobSourceID, TEngineJobSourceID>(sid.jobSourceID);

    return tsid;
}

template<>
ShotID STI::Network::convert<TShotID, ShotID>(const TShotID& tsid)
{
    ShotID sid;
    convert<TShotID, ShotID>(tsid, sid);

    // shot.parseID = convert<TParseID, ParseID>(tsid.parseID);

    // shot.submissionTime = convert<TTimeStamp, TimeStamp>(tsid.submissionTime);
    // shot.playTime = convert<TTimeStamp, TimeStamp>(tsid.playTime);
    
    // shot.jobSourceID = convert<TEngineJobSourceID, EngineJobSourceID>(tsid.jobSourceID);

    return sid;
}

template<>
bool STI::Network::convert<ShotID, TShotID>(const ShotID& sid, TShotID& tsid)
{
    tsid.parseID = convert<ParseID, TParseID>(sid.parseID);

    tsid.submissionTime = convert<TimeStamp, TTimeStamp>(sid.submissionTime);
    tsid.playTime = convert<TimeStamp, TTimeStamp>(sid.playTime);
    
    tsid.jobSourceID = convert<EngineJobSourceID, TEngineJobSourceID>(sid.jobSourceID);

    return true;
}

template<>
bool STI::Network::convert<TShotID, ShotID>(const TShotID& tsid, ShotID& sid)
{
    sid.parseID = convert<TParseID, ParseID>(tsid.parseID);

    sid.submissionTime = convert<TTimeStamp, TimeStamp>(tsid.submissionTime);
    sid.playTime = convert<TTimeStamp, TimeStamp>(tsid.playTime);
    
    sid.jobSourceID = convert<TEngineJobSourceID, EngineJobSourceID>(tsid.jobSourceID);

    return true;
}



//TimeStamp
template<>
TTimeStamp STI::Network::convert<TimeStamp, TTimeStamp>(const TimeStamp& timeStamp)
{
    TTimeStamp tTime;

    tTime.year = timeStamp.year();
    tTime.month = timeStamp.month();
    tTime.day = timeStamp.day();
    tTime.hour = timeStamp.hour();
    tTime.min = timeStamp.min();
    tTime.sec = timeStamp.sec();
    tTime.millis = timeStamp.millis();
    tTime.micros = timeStamp.micros();
    tTime.nanos = timeStamp.nanos();

    return tTime;
}

template<>
TimeStamp STI::Network::convert<TTimeStamp, TimeStamp>(const TTimeStamp& tTime)
{
    TimeStamp time(tTime.year, tTime.month, tTime.day, tTime.hour, tTime.min, tTime.sec,
                    tTime.millis, tTime.micros, tTime.nanos);

    return time;
}



//EngineJobSourceID
template<>
TEngineJobSourceID STI::Network::convert<EngineJobSourceID, TEngineJobSourceID>(const EngineJobSourceID& jobSourceID)
{
    TEngineJobSourceID tSourceID;

    tSourceID.user = convert<std::string, ::CORBA::String_member>(jobSourceID.user);
    tSourceID.machine = convert<std::string, ::CORBA::String_member>(jobSourceID.machine);

    return tSourceID;
}

template<>
EngineJobSourceID STI::Network::convert<TEngineJobSourceID, EngineJobSourceID>(const TEngineJobSourceID& tJobSourceID)
{
    EngineJobSourceID sourceID;

    sourceID.user = convert<::CORBA::String_member, std::string>(tJobSourceID.user);
    sourceID.machine = convert<::CORBA::String_member, std::string>(tJobSourceID.machine);

    return sourceID;
}




template<>
bool STI::Network::convert<TShot_ptr, std::shared_ptr<Shot>>(const TShot_ptr& tShot, std::shared_ptr<Shot>& shot)
{
    bool success = false;

    std::shared_ptr<Shot> parsedShot;

    if (!CORBA::is_nil(tShot)) {
        shot = std::make_shared<STI::Network::RemoteShot>(tShot);
        success = (shot != 0);
    }

    return success;
}


template<>
bool STI::Network::convert<std::shared_ptr<Shot>, TShot_ptr>(const std::shared_ptr<Shot>& shot, TShot_ptr& tShot)
{
    //std::shared_ptr<Shot> parsedShot;
//    STI::TNetwork::TShot_ptr tShot;
    
    if (shot != 0 && TShotRefInterface::getTShotReference(shot, tShot)) {
        return !CORBA::is_nil(tShot);
    }
    return false;
}



//RawEventType
template<>
TRawEventType STI::Network::convert<RawEventType, TRawEventType>(const RawEventType& evtType)
{
    TRawEventType tEvtType;

    switch (evtType)
    {
    case RawEventType::Play:
        tEvtType = TRawEventType::RawEventPlay;
        break;
    case RawEventType::Measurement:
        tEvtType = TRawEventType::RawEventMeasurement;
        break;
    case RawEventType::Waveform:
        tEvtType = TRawEventType::RawEventWaveform;
        break;
    case RawEventType::Pause:
        tEvtType = TRawEventType::RawEventPause;
        break;
    case RawEventType::Jump:
        tEvtType = TRawEventType::RawEventJump;
        break;        
    default:
        tEvtType = TRawEventType::RawEventPlay;
        break;
    }

    return tEvtType;
}

template<>
RawEventType STI::Network::convert<TRawEventType, RawEventType>(const TRawEventType& tEvtType)
{
    RawEventType evtType;

    switch (tEvtType)
    {
    case TRawEventType::RawEventPlay:
        evtType = RawEventType::Play;
        break;
    case TRawEventType::RawEventMeasurement:
        evtType = RawEventType::Measurement;
        break;
    case TRawEventType::RawEventWaveform:
        evtType = RawEventType::Waveform;
        break;
    case TRawEventType::RawEventPause:
        evtType = RawEventType::Pause;
        break;
    case TRawEventType::RawEventJump:
        evtType = RawEventType::Jump;
        break;        
    default:
        evtType = RawEventType::Play;
        break;
    }

    return evtType;
}


//EngineParsingMessage
template<>
bool STI::Network::convert<EngineParsingMessage, TEngineParsingMessage>(const EngineParsingMessage& parsingMessage, TEngineParsingMessage& tParsingMessage)
{
    tParsingMessage.type = convert<ParsingMessageType, TParsingMessageType>(parsingMessage.getType());
    tParsingMessage.id_code = static_cast<::CORBA::Short>(parsingMessage.getIDCode());
    tParsingMessage.sourceID = convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(parsingMessage.getSourceID());
    convert<std::string, ::CORBA::String_member>(parsingMessage.getName(), tParsingMessage.name);
    convert<std::string, ::CORBA::String_member>(parsingMessage.getMessage(), tParsingMessage.message);
    convert<RawEvent, TRawEvent>(parsingMessage.getEvents(), tParsingMessage.events);
    return true;
}



template<>
bool STI::Network::convert<TEngineParsingMessage, EngineParsingMessage>(const TEngineParsingMessage& tParsingMessage, EngineParsingMessage& parsingMessage)
{
    parsingMessage = convert<TEngineParsingMessage, EngineParsingMessage>(tParsingMessage);

    return true;
}

template<>
EngineParsingMessage STI::Network::convert<TEngineParsingMessage, EngineParsingMessage>(const TEngineParsingMessage& tParsingMessage)
{
    EngineParsingMessage parsingMessage(
            convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tParsingMessage.sourceID),
            convert<TParsingMessageType, ParsingMessageType>(tParsingMessage.type),
            static_cast<unsigned>(tParsingMessage.id_code),
            convert<::CORBA::String_member, std::string>(tParsingMessage.name)
            );

    parsingMessage.appendMessage( convert<::CORBA::String_member, std::string>(tParsingMessage.message) );

    convert<TRawEvent, RawEvent>(tParsingMessage.events, parsingMessage.getEventVector());

    return parsingMessage;
}


//ParsingMessageType
template<>
TParsingMessageType STI::Network::convert<ParsingMessageType, TParsingMessageType>(const Engine::ParsingMessageType& messType)
{
    TParsingMessageType tMessType;

    switch (messType)
    {
    case ParsingMessageType::Error:
        tMessType = TParsingMessageType::ParsingError;
        break;
    case ParsingMessageType::Warning:
        tMessType = TParsingMessageType::ParsingWarning;
        break;
    case ParsingMessageType::Information:
        tMessType = TParsingMessageType::ParsingInformation;
        break;
    default:
        tMessType = TParsingMessageType::ParsingError;
        break;
    }

    return tMessType;
}

template<>
ParsingMessageType STI::Network::convert<TParsingMessageType, ParsingMessageType>(const TNetwork::TParsingMessageType& tMessType)
{
    ParsingMessageType messType;

    switch (tMessType)
    {
    case TParsingMessageType::ParsingError:
        messType = ParsingMessageType::Error;
        break;
    case TParsingMessageType::ParsingWarning:
        messType = ParsingMessageType::Warning;
        break;
    case TParsingMessageType::ParsingInformation:
        messType = ParsingMessageType::Information;
        break;
    default:
        messType = ParsingMessageType::Error;
        break;
    }

    return messType;
}


//Measurement
template<>
bool STI::Network::convert<std::shared_ptr<Measurement>, TMeasurement>(
        const std::shared_ptr<Measurement>& measurement, TMeasurement& tMeasurement)
{
    if (measurement == 0) return false;

    tMeasurement.time = static_cast<CORBA::Double>(measurement->time());
    tMeasurement.channel = static_cast<CORBA::UShort>(measurement->channel());
    convertEventGraphPath(measurement->getMeasurementGraphPath(), tMeasurement.measurementGraphPath);
    convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(measurement->device(), tMeasurement.device);
    convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(measurement->data(), tMeasurement.measurementResult);
    return true;
}

template<>
bool STI::Network::convert<TMeasurement, std::shared_ptr<Measurement>>(
        const TMeasurement& tMeasurement, std::shared_ptr<Measurement>& measurement)
{
    STI::Utils::GraphPathLabel gpl;
    convertEventGraphPath(tMeasurement.measurementGraphPath, gpl);

    measurement = std::make_shared<Measurement>(
        static_cast<double>(tMeasurement.time),
        static_cast<unsigned short>(tMeasurement.channel),
        convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tMeasurement.device),
        gpl);

    auto data = convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(tMeasurement.measurementResult);
    measurement->setMeasurementResult(data);

    return true;
}

template<>
TMeasurement STI::Network::convert<std::shared_ptr<Measurement>, TMeasurement>(
        const std::shared_ptr<Measurement>& measurement)
{
    TMeasurement tMeasurement;
    convert<std::shared_ptr<Measurement>, TMeasurement>(measurement, tMeasurement);
    return tMeasurement;
}

template<>
std::shared_ptr<Measurement> STI::Network::convert<TMeasurement, std::shared_ptr<Measurement>>(
        const TMeasurement& tMeasurement)
{
    std::shared_ptr<Measurement> measurement;
    convert<TMeasurement, std::shared_ptr<Measurement>>(tMeasurement, measurement);
    return measurement;
}

