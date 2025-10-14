#include "NetworkConvert.h"

#include "Convert_EventEngine.h"
#include "Convert_DeviceTrace.h"
#include "Convert_StackTrace.h"
#include "Convert_SequenceResult.h"

#include <sti/device/DeviceTrace.h>

#include <sti/engine/AddSequenceStatus.h>
#include <sti/engine/EngineJobID.h>
#include <sti/engine/EngineParsingMessage.h>
#include <sti/engine/EngineID.h>
#include <sti/engine/EventEngineJob.h>
#include <sti/engine/Measurement.h>
#include <sti/engine/ParsedDependencyTree.h>
#include <sti/engine/ParseID.h>
#include <sti/engine/ParseJobStatus.h>
#include <sti/engine/PlayJobStatus.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/Shot.h>
#include <sti/engine/ShotID.h>
#include <sti/engine/ShotConfig.h>
#include <sti/engine/ShotResultRecord.h>
#include <sti/utils/TimeStamp.h>

#include <sti/utils/GraphPathLabel.h>

#include "EventEngine.h"
#include "EventEngineDependencyTree.h"
#include "LocalEventEngineJob.h"
#include "NetworkEventEngine.h"
#include "NetworkShotWrapper.h"

#include "RemoteShot.h"
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
using STI::Utils::TimeStamp;
using STI::TNetwork::TTimeStamp;
using STI::Engine::EngineJobSourceID;
using STI::TNetwork::TEngineJobSourceID;
using STI::TNetwork::TShot;
using STI::Engine::Shot;
using STI::Engine::EngineParsingMessage;
using STI::TNetwork::TEngineParsingMessage;
using STI::Engine::EngineParsingMessageCount;
using STI::TNetwork::TEngineParsingMessageCount;
using STI::Engine::ParsingMessageType;
using STI::TNetwork::TParsingMessageType;
using STI::Engine::ParseID; 
using STI::TNetwork::TParseID;
using STI::Engine::ShotID;
using STI::TNetwork::TShotID;
using STI::TNetwork::TMeasurement;
using STI::Engine::Measurement;
using STI::TNetwork::TShotType;
using STI::Engine::ShotConfig; 
using STI::TNetwork::TShotConfig;
using STI::Engine::RecordStatus;
using STI::TNetwork::TRecordStatus;
using STI::Engine::ShotResultRecord;
using STI::TNetwork::TShotResultRecord;
using STI::Engine::ShotType;
using STI::TNetwork::TShotType;
using STI::TNetwork::TRawEventTarget;
using STI::Engine::RawEventTarget;
using STI::TNetwork::TRawEventTargetDevice;
using STI::Engine::RawEventTargetDevice;
using STI::TNetwork::TRawEventTargetChannel;
using STI::Engine::RawEventTargetChannel;
using STI::TNetwork::TEventEngineJobList;
using STI::Engine::EventEngineJobList;
using STI::Engine::ParseJobStatus;
using STI::TNetwork::TParseJobStatus;
using STI::Engine::PlayJobStatus;
using STI::TNetwork::TPlayJobStatus;
using STI::Engine::AddSequenceStatus;
using STI::TNetwork::TAddSequenceStatus;
using STI::Engine::SequenceID;
using STI::TNetwork::TSequenceID;
using STI::TNetwork::TDeviceIDMeasurementsTupleSeq;
using STI::Engine::MeasurementMap;
using STI::Engine::MeasurementVector;
using STI::Device::DeviceID;
using STI::TNetwork::TDeviceID;
using STI::Engine::SequenceEntryID;
using STI::TNetwork::TSequenceEntryID;


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

        outNodes.clear();
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
    tJobID.seqid = convert<SequenceID, TSequenceID>(jobID.seqid);

    return true;
}

template<>
bool STI::Network::convert<TEngineJobID, EngineJobID>(const TEngineJobID& tJobID, EngineJobID& jobID)
{
    jobID.type = convert<TEventEngineJobType, EventEngineJobType>(tJobID.type);
    jobID.pid = convert<TParseID, ParseID>(tJobID.pid);
    jobID.sid = convert<TShotID, ShotID>(tJobID.sid);
    jobID.seqid = convert<TSequenceID, SequenceID>(tJobID.seqid);

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
        case EventEngineJobType::Sequence:
            tJobType = TEventEngineJobType::EngineJobSequence;
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
        case TEventEngineJobType::EngineJobSequence:
            jobType = EventEngineJobType::Sequence;
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
        tJobStatus = TEngineJobStatus::JobNew;
        break;
    case EngineJobStatus::Running:
        tJobStatus = TEngineJobStatus::JobRunning;
        break;
    case EngineJobStatus::Completed:
        tJobStatus = TEngineJobStatus::JobCompleted;
        break;
    case EngineJobStatus::Canceled:
        tJobStatus = TEngineJobStatus::JobCanceled;
        break;
    case EngineJobStatus::NotFound:
        tJobStatus = TEngineJobStatus::JobNotFound;
        break;
    case EngineJobStatus::Archived:
        tJobStatus = TEngineJobStatus::JobArchived;
        break;
    case EngineJobStatus::Deferred:
        tJobStatus = TEngineJobStatus::JobDeferred;
        break;
    default:
        tJobStatus = TEngineJobStatus::JobNotFound;
        break;
    }
    return true;
}

template<>
bool STI::Network::convert<TEngineJobStatus, EngineJobStatus>(const TEngineJobStatus& tJobStatus, EngineJobStatus& jobStatus)
{
    
    switch (tJobStatus)
    {
    case TEngineJobStatus::JobNew:
        jobStatus = EngineJobStatus::New;
        break;
    case TEngineJobStatus::JobRunning:
        jobStatus = EngineJobStatus::Running;
        break;
    case TEngineJobStatus::JobCompleted:
        jobStatus = EngineJobStatus::Completed;
        break;
    case TEngineJobStatus::JobCanceled:
        jobStatus = EngineJobStatus::Canceled;
        break;
    case TEngineJobStatus::JobNotFound:
        jobStatus = EngineJobStatus::NotFound;
        break;
    case TEngineJobStatus::JobArchived:
        jobStatus = EngineJobStatus::Archived;
        break;
    case TEngineJobStatus::JobDeferred:
        jobStatus = EngineJobStatus::Deferred;
        break;
    default:
        jobStatus = EngineJobStatus::NotFound;
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
    STI::TNetwork::TShot tShot;

    ::STI::TNetwork::TShotCallback_ptr tShotCallback;
    
    bool shotMissing = engineJob.getStatus() == EngineJobStatus::NotFound || 
                       engineJob.getStatus() == EngineJobStatus::Archived;

    if (!shotMissing && engineJob.getShot(shot) && TShotRefInterface::getTShotReference(shot, tShotCallback)) {

        tShot.shotCallback = tShotCallback;
        tShot.shotConfig = convert<ShotConfig, TShotConfig>(shot->getShotConfig());
    }
    else {
        // tShot.shotConfig = convert<ShotConfig, TShotConfig>(engineJob.getJobID().pid.shotConfig);
        tShot.shotCallback = STI::TNetwork::TShotCallback::_nil();
    }

    tEngineJob.shot.shotConfig;
    tEngineJob.shot = tShot;

    std::shared_ptr<EventEngineDependencyTree> tree;
    if (!engineJob.getDependencies(tree)) {
        tree = std::make_shared<EventEngineDependencyTree>();
    }

    convert<EventEngineDependencyTree, TEventEngineDependencyTree>(*tree, tEngineJob.dependencies);

    convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(engineJob.getMissingTargetIDs(), tEngineJob.missingTargetIDs);

    convert<STI::Engine::EngineParsingMessage, STI::TNetwork::TEngineParsingMessage>(engineJob.getParsingMessages(), tEngineJob.messages);

    return true;
}

template<>
bool STI::Network::convert<TEventEngineJob, std::shared_ptr<EventEngineJob>>(const TEventEngineJob& tEngineJob, std::shared_ptr<EventEngineJob>& engineJob)
{
    bool success = false;

    EngineJobID jobID = convert<TEngineJobID, EngineJobID>(tEngineJob.jobID);

    std::shared_ptr<Shot> parsedShot;
    std::shared_ptr<EventEngineDependencyTree> tree;
    std::set<STI::Device::DeviceID> missingTargets;     //empty

    EngineJobStatus jobStatus = convert<TEngineJobStatus, EngineJobStatus>(tEngineJob.status);

    switch (jobID.type)
    {
    case EventEngineJobType::Parse:
        {
            convert<TShot, std::shared_ptr<Shot>>(tEngineJob.shot, parsedShot);

            // if (!CORBA::is_nil(tEngineJob.shot.shotCallback)) {

            //     parsedShot = std::make_shared<STI::Network::RemoteShot>(
            //         convert<TShotConfig, ShotConfig>(tEngineJob.shot.shotConfig), tEngineJob.shot.shotCallback);
            // }

            tree = std::make_shared<EventEngineDependencyTree>();
            convert<TEventEngineDependencyTree, EventEngineDependencyTree>(tEngineJob.dependencies, *tree);

            auto job = std::make_shared<LocalEventEngineJob>(jobID.pid, parsedShot,
                convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tEngineJob.jobOwner)
                );
        
            job->setDependencies(tree);
            job->setMissingTargets(missingTargets);

            job->markRunning(convert<TEngineID, EngineID>(tEngineJob.engineID));    // work around to set EngineID
            job->setStatus(jobStatus);  //set actual status

            engineJob = job;
            success = true;
        }
        break;
	case EventEngineJobType::Play:
        {
            convert<TShot, std::shared_ptr<Shot>>(tEngineJob.shot, parsedShot);

            auto job = std::make_shared<LocalEventEngineJob>(jobID, parsedShot,
                convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tEngineJob.jobOwner));
            job->setStatus(jobStatus);
            engineJob = job;
            success = true;
        }
		break;
    default:
        break;
    }

    std::vector<EngineParsingMessage> messages;
    convert<STI::TNetwork::TEngineParsingMessage, STI::Engine::EngineParsingMessage>(tEngineJob.messages, messages);
    engineJob->addMessages(messages);

    return success && (engineJob != 0);
}

template<>
TEventEngineJob STI::Network::convert<EventEngineJob, TEventEngineJob>(const EventEngineJob& engineJob)
{
    TEventEngineJob tJob;

    convert<EventEngineJob, TEventEngineJob>(engineJob, tJob);

    return tJob;
}


template<>
TEventEngineJob STI::Network::convert<std::shared_ptr<EventEngineJob>, TEventEngineJob>(const std::shared_ptr<EventEngineJob>& engineJob)
{
    TEventEngineJob tJob;

    convert<std::shared_ptr<EventEngineJob>, TEventEngineJob>(engineJob, tJob);

    return tJob;
}

template<>
std::shared_ptr<EventEngineJob> STI::Network::convert<TEventEngineJob, std::shared_ptr<EventEngineJob>>(const TEventEngineJob& tEngineJob)
{
    std::shared_ptr<EventEngineJob> job;

    convert<TEventEngineJob, std::shared_ptr<EventEngineJob>>(tEngineJob, job);

    return job;
}



//RawEventTarget
template<>
TRawEventTarget STI::Network::convert<RawEventTarget, TRawEventTarget>(const RawEventTarget& target)
{
    TRawEventTarget tTarget;
    convert<RawEventTargetDevice, TRawEventTargetDevice>(target.device(), tTarget.device);
    convert<RawEventTargetChannel, TRawEventTargetChannel>(target.channel(), tTarget.channel);
    return tTarget;
}

template<>
RawEventTarget STI::Network::convert<TRawEventTarget, RawEventTarget>(const TRawEventTarget& tTarget)
{
    RawEventTarget target(
        convert<TRawEventTargetDevice, RawEventTargetDevice>(tTarget.device),
        convert<TRawEventTargetChannel, RawEventTargetChannel>(tTarget.channel)
        );
    return target;
}

//RawEventTargetDevice
template<>
bool STI::Network::convert<RawEventTargetDevice, TRawEventTargetDevice>(const RawEventTargetDevice& targetDevice, TRawEventTargetDevice& tTargetDevice)
{
    tTargetDevice.isAbstract = static_cast<CORBA::Boolean>(targetDevice.isAbstract());
    
    if (targetDevice.isAbstract()) {
        convert<std::string, ::CORBA::String_member>(targetDevice.name(), tTargetDevice.name);
    }
    else {
        convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(targetDevice.deviceID(), tTargetDevice.targetDeviceID);
    }
    return true;
}

template<>
RawEventTargetDevice STI::Network::convert<TRawEventTargetDevice, RawEventTargetDevice>(const TRawEventTargetDevice& tTargetDevice)
{
    if (tTargetDevice.isAbstract) {
        RawEventTargetDevice targetDevice(
            convert<::CORBA::String_member, std::string>(tTargetDevice.name)
            );
        return targetDevice;
    }
    else {
        RawEventTargetDevice targetDevice(
            convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tTargetDevice.targetDeviceID)
            );
        return targetDevice;
    }
}

//RawEventTargetChannel
template<>
bool STI::Network::convert<RawEventTargetChannel, TRawEventTargetChannel>(const RawEventTargetChannel& targetChannel, TRawEventTargetChannel& tTargetChannel)
{
    tTargetChannel.isAbstract = static_cast<CORBA::Boolean>(targetChannel.isAbstract());
 
    if (targetChannel.isAbstract()) {
        convert<std::string, ::CORBA::String_member>(targetChannel.name(), tTargetChannel.name);
    }
    else {
        tTargetChannel.channel = static_cast<CORBA::UShort>(targetChannel.channel());
    }
    return true;
}

template<>
RawEventTargetChannel STI::Network::convert<TRawEventTargetChannel, RawEventTargetChannel>(const TRawEventTargetChannel& tTargetChannel)
{
    if (tTargetChannel.isAbstract) {
        RawEventTargetChannel targetChannel(
            convert<::CORBA::String_member, std::string>(tTargetChannel.name)
            );
        return targetChannel;
    }
    else {
        RawEventTargetChannel targetChannel(
            static_cast<unsigned short>(tTargetChannel.channel)
            );
        return targetChannel;
    }
}



//EventEngineJobList
template<>
TEventEngineJobList STI::Network::convert<EventEngineJobList, TEventEngineJobList>(const EventEngineJobList& jobList)
{
    TEventEngineJobList tJobList;

    switch (jobList)
    {
    case EventEngineJobList::Queued:
        tJobList = TEventEngineJobList::EngineJobListQueued;
        break;
    case EventEngineJobList::Running:
        tJobList = TEventEngineJobList::EngineJobListRunning;
        break;
    case EventEngineJobList::Completed:
        tJobList = TEventEngineJobList::EngineJobListCompleted;
        break;
    case EventEngineJobList::Archived:
        tJobList = TEventEngineJobList::EngineJobListArchived;
        break;
    default:
        tJobList = TEventEngineJobList::EngineJobListArchived;
        break;
    }

    return tJobList;
}

template<>
EventEngineJobList STI::Network::convert<TEventEngineJobList, EventEngineJobList>(const TEventEngineJobList& tJobList)
{
    EventEngineJobList jobList;

    switch (tJobList)
    {
    case TEventEngineJobList::EngineJobListQueued:
        jobList = EventEngineJobList::Queued;
        break;
    case TEventEngineJobList::EngineJobListRunning:
        jobList = EventEngineJobList::Running;
        break;
    case TEventEngineJobList::EngineJobListCompleted:
        jobList = EventEngineJobList::Completed;
        break;
    case TEventEngineJobList::EngineJobListArchived:
        jobList = EventEngineJobList::Archived;
        break;
    default:
        jobList = EventEngineJobList::Archived;
        break;
    }

    return jobList;
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
    tEvent.target = convert<RawEventTarget, TRawEventTarget>(evt.target());
    convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(evt.value(), tEvent.parsedValue.value);
    convert<std::string, ::CORBA::String_member>(evt.description(), tEvent.description);
    convert<std::string, ::CORBA::String_member>(evt.getGroupName(), tEvent.groupName);
    convert<STI::Engine::CompressedStackTrace, STI::TNetwork::TStackFrameSeq>(evt.getCompressedStackTrace(), tEvent.stackTrace);
    tEvent.isMeasurement = static_cast<CORBA::Boolean>(evt.isMeasurementEvent());
    tEvent.rawEventType = convert<RawEventType, TRawEventType>(evt.type());
    STI::Network::convertEventGraphPath(evt.getEventGraphPath(), tEvent.eventGraphPath);

    return true;
}

template<>
bool STI::Network::convert<TRawEvent, RawEvent>(const TRawEvent& tEvent, RawEvent& evt)
{
	evt.setTime(static_cast<double>(tEvent.time));
    evt.setTarget(convert<TRawEventTarget, RawEventTarget>(tEvent.target));
	evt.setValue(convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(tEvent.parsedValue.value));
	evt.setDescription(convert<::CORBA::String_member, std::string>(tEvent.description));
    evt.setGroupName(convert<::CORBA::String_member, std::string>(tEvent.groupName));
    convert<STI::TNetwork::TStackFrameSeq, STI::Engine::CompressedStackTrace>(tEvent.stackTrace, evt.getCompressedStackTrace());

	evt.setEventType(convert<TRawEventType, RawEventType>(tEvent.rawEventType));

    STI::Utils::GraphPathLabel gpl;
    STI::Network::convertEventGraphPath(tEvent.eventGraphPath, gpl);
	evt.setEventGraphPath(gpl);
    evt.setParentGroup(nullptr);

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



//TShotConfig
template<>
TShotConfig STI::Network::convert<ShotConfig, TShotConfig>(const ShotConfig& shotConfig)
{
    TShotConfig tShotConfig;

    tShotConfig.shotType = convert<ShotType, TShotType>(shotConfig.shotType);
    tShotConfig.jobSourceID = convert<EngineJobSourceID, TEngineJobSourceID>(shotConfig.jobSourceID);

    tShotConfig.targetEnginePool = static_cast<CORBA::Long>(shotConfig.targetEnginePool);

    tShotConfig.file = convert<std::string, ::CORBA::String_member>(shotConfig.file);
    tShotConfig.comment = convert<std::string, ::CORBA::String_member>(shotConfig.comment);

    return tShotConfig;
}

template<>
ShotConfig STI::Network::convert<TShotConfig, ShotConfig>(const TShotConfig& tShotConfig)
{
    ShotConfig shotConfig;

    shotConfig.shotType = convert<TShotType, ShotType>(tShotConfig.shotType);
    shotConfig.jobSourceID = convert<TEngineJobSourceID, EngineJobSourceID>(tShotConfig.jobSourceID);

    shotConfig.targetEnginePool = static_cast<int>(tShotConfig.targetEnginePool);

    shotConfig.file = convert<::CORBA::String_member, std::string>(tShotConfig.file);
    shotConfig.comment = convert<::CORBA::String_member, std::string>(tShotConfig.comment);

    return shotConfig;
}

//TRecordStatus
template<>
TRecordStatus STI::Network::convert<RecordStatus, TRecordStatus>(const RecordStatus& recordStatus)
{
    TRecordStatus tRecordStatus;

    switch (recordStatus)
    {
    case RecordStatus::Unqueried:
        tRecordStatus = TRecordStatus::TRecordUnqueried;
        break;
    case RecordStatus::Complete:
        tRecordStatus = TRecordStatus::TRecordComplete;
        break;
    case RecordStatus::MissingDevice:
        tRecordStatus = TRecordStatus::TRecordMissingDevice;
        break;
    case RecordStatus::MissingResults:
        tRecordStatus = TRecordStatus::TRecordMissingResults;
        break;
    case RecordStatus::Error:
        tRecordStatus = TRecordStatus::TRecordError;
        break;
    default:
        tRecordStatus = TRecordStatus::TRecordError;
        break;
    }

    return tRecordStatus;
}

template<>
RecordStatus STI::Network::convert<TRecordStatus, RecordStatus>(const TRecordStatus& tRecordStatus)
{
    RecordStatus recordStatus;

    switch (tRecordStatus)
    {
    case TRecordStatus::TRecordUnqueried:
        recordStatus = RecordStatus::Unqueried;
        break;
    case TRecordStatus::TRecordComplete:
        recordStatus = RecordStatus::Complete;
        break;
    case TRecordStatus::TRecordMissingDevice:
        recordStatus = RecordStatus::MissingDevice;
        break;
    case TRecordStatus::TRecordMissingResults:
        recordStatus = RecordStatus::MissingResults;
        break;
    case TRecordStatus::TRecordError:
        recordStatus = RecordStatus::Error;
        break;
    default:
        recordStatus = RecordStatus::Error;
        break;
    }

    return recordStatus;
}

//TShotResultRecord
template<>
TShotResultRecord STI::Network::convert<ShotResultRecord, TShotResultRecord>(const ShotResultRecord& shotResultRecord)
{
    TShotResultRecord tShotResultRecord;
    
    convert<ShotResultRecord, TShotResultRecord>(shotResultRecord, tShotResultRecord);

    return tShotResultRecord;

}

template<>
ShotResultRecord STI::Network::convert<TShotResultRecord, ShotResultRecord>(const TShotResultRecord& tShotResultRecord)
{
    ShotResultRecord shotResultRecord;
    
    convert<TShotResultRecord, ShotResultRecord>(tShotResultRecord, shotResultRecord);

    return shotResultRecord;
}

template<>
bool STI::Network::convert<ShotResultRecord, TShotResultRecord>(const ShotResultRecord& shotResultRecord, TShotResultRecord& tShotResultRecord)
{
    tShotResultRecord.deviceID = convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(shotResultRecord.deviceID);
    tShotResultRecord.recordStatus = convert<RecordStatus, TRecordStatus>(shotResultRecord.recordStatus);

    tShotResultRecord.dependencies.length(0);
    convert<ShotResultRecord, TShotResultRecord>(shotResultRecord.dependencies, tShotResultRecord.dependencies);

    return true;
}

template<>
bool STI::Network::convert<TShotResultRecord, ShotResultRecord>(const TShotResultRecord& tShotResultRecord, ShotResultRecord& shotResultRecord)
{   
    shotResultRecord.deviceID = convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tShotResultRecord.deviceID);
    shotResultRecord.recordStatus = convert<TRecordStatus, RecordStatus>(tShotResultRecord.recordStatus);

    convert<TShotResultRecord, ShotResultRecord>(tShotResultRecord.dependencies, shotResultRecord.dependencies);

    return true;
}


//ShotType
template<>
TShotType STI::Network::convert<ShotType, TShotType>(const ShotType& shotType)
{
    TShotType tShotType;

    switch (shotType)
    {
    case ShotType::Single:
        tShotType = TShotType::ShotTypeSingle;
        break;
    case ShotType::Sequence:
        tShotType = TShotType::ShotTypeSequence;
        break;
    case ShotType::SingleUndocumented:
        tShotType = TShotType::ShotTypeSingleUndocumented;
        break;
    case ShotType::SequenceEntry:
        tShotType = TShotType::ShotTypeSequenceEntry;
        break;
    default:
        tShotType = TShotType::ShotTypeSingle;
        break;
    }

    return tShotType;
}

template<>
ShotType STI::Network::convert<TShotType, ShotType>(const TShotType& tShotType)
{
    ShotType shotType;

    switch (tShotType)
    {
    case TShotType::ShotTypeSingle:
        shotType = ShotType::Single;
        break;
    case TShotType::ShotTypeSequence:
        shotType = ShotType::Sequence;
        break;
    case TShotType::ShotTypeSingleUndocumented:
        shotType = ShotType::SingleUndocumented;
        break;
    case TShotType::ShotTypeSequenceEntry:
        shotType = ShotType::SequenceEntry;
        break; 
    default:
        shotType = ShotType::Single;
        break;
    }

    return shotType;
}




//ParseID
template<>
TParseID STI::Network::convert<ParseID, TParseID>(const ParseID& pid)
{
    TParseID tParseID;

    tParseID.parseTimestamp = convert<TimeStamp, TTimeStamp>(pid.parseTimestamp);
    // tParseID.shotConfig = convert<ShotConfig, TShotConfig>(pid.shotConfig);
    tParseID.shotType = convert<ShotType, TShotType>(pid.shotType);
    tParseID.jobSourceID = convert<EngineJobSourceID, TEngineJobSourceID>(pid.jobSourceID);

    if (pid.shotType == ShotType::SequenceEntry) {
        tParseID.sequenceEntryID = convert<SequenceEntryID, TSequenceEntryID>(pid.sequenceEntryID);
    }

    return tParseID;
}

template<>
ParseID STI::Network::convert<TParseID, ParseID>(const TParseID& tpid)
{
    ParseID parseID;

    parseID.parseTimestamp = convert<TTimeStamp, TimeStamp>(tpid.parseTimestamp);
    parseID.shotType = convert<TShotType, ShotType>(tpid.shotType);
    parseID.jobSourceID = convert<TEngineJobSourceID, EngineJobSourceID>(tpid.jobSourceID);
    // parseID.shotConfig = convert<TShotConfig, ShotConfig>(tpid.shotConfig);
    // ShotType shotType;
	// EngineJobSourceID jobSourceID;
    
    if (parseID.shotType == ShotType::SequenceEntry) {
        parseID.sequenceEntryID = convert<TSequenceEntryID, SequenceEntryID>(tpid.sequenceEntryID);
    }
    
    return parseID;
}


template<>
bool STI::Network::convert<ParseID, TParseID>(const ParseID& pid, TParseID& tpid)
{
    tpid = convert<ParseID, TParseID>(pid);
    return true;
}

template<>
bool STI::Network::convert<TParseID, ParseID>(const TParseID& tpid, ParseID& pid)
{
    pid = convert<TParseID, ParseID>(tpid);
    return true;
}

//ParseJobStatus
template<>
TParseJobStatus STI::Network::convert<ParseJobStatus, TParseJobStatus>(const ParseJobStatus& parseJobStatus)
{
    TParseJobStatus tParseJobStatus;
    tParseJobStatus.pid = convert<ParseID, TParseID>(parseJobStatus.pid);
    tParseJobStatus.status = convert<EngineJobStatus, TEngineJobStatus>(parseJobStatus.status);
    return tParseJobStatus;
}

template<>
ParseJobStatus STI::Network::convert<TParseJobStatus, ParseJobStatus>(const TParseJobStatus& tParseJobStatus)
{
    ParseJobStatus parseJobStatus;
    parseJobStatus.pid = convert<TParseID, ParseID>(tParseJobStatus.pid);
    parseJobStatus.status = convert<TEngineJobStatus, EngineJobStatus>(tParseJobStatus.status);
    return parseJobStatus;
}

template<>
bool STI::Network::convert<ParseJobStatus, TParseJobStatus>(const ParseJobStatus& parseJobStatus, TParseJobStatus& tParseJobStatus)
{
    tParseJobStatus = convert<ParseJobStatus, TParseJobStatus>(parseJobStatus);
    return true;
}

template<>
bool STI::Network::convert<TParseJobStatus, ParseJobStatus>(const TParseJobStatus& tParseJobStatus, ParseJobStatus& parseJobStatus)
{
    parseJobStatus = convert<TParseJobStatus, ParseJobStatus>(tParseJobStatus);
    return true;
}



//ShotID
template<>
TShotID STI::Network::convert<ShotID, TShotID>(const ShotID& sid)
{
    TShotID tsid;
    convert<ShotID, TShotID>(sid, tsid);

    return tsid;
}

template<>
ShotID STI::Network::convert<TShotID, ShotID>(const TShotID& tsid)
{
    ShotID sid;
    convert<TShotID, ShotID>(tsid, sid);

    return sid;
}

template<>
bool STI::Network::convert<ShotID, TShotID>(const ShotID& sid, TShotID& tsid)
{
    tsid.parseID = convert<ParseID, TParseID>(sid.parseID);
    tsid.jobSourceID = convert<EngineJobSourceID, TEngineJobSourceID>(sid.jobSourceID);

    tsid.submissionTime = convert<TimeStamp, TTimeStamp>(sid.submissionTime);

    return true;
}

template<>
bool STI::Network::convert<TShotID, ShotID>(const TShotID& tsid, ShotID& sid)
{
    sid.parseID = convert<TParseID, ParseID>(tsid.parseID);
    sid.jobSourceID = convert<TEngineJobSourceID, EngineJobSourceID>(tsid.jobSourceID);

    sid.submissionTime = convert<TTimeStamp, TimeStamp>(tsid.submissionTime);

    return true;
}



//PlayJobStatus
template<>
TPlayJobStatus STI::Network::convert<PlayJobStatus, TPlayJobStatus>(const PlayJobStatus& playJobStatus)
{
    TPlayJobStatus tPlayJobStatus;
    tPlayJobStatus.sid = convert<ShotID, TShotID>(playJobStatus.sid);
    tPlayJobStatus.status = convert<EngineJobStatus, TEngineJobStatus>(playJobStatus.status);
    return tPlayJobStatus;
}

template<>
PlayJobStatus STI::Network::convert<TPlayJobStatus, PlayJobStatus>(const TPlayJobStatus& tPlayJobStatus)
{
    PlayJobStatus playJobStatus;
    playJobStatus.sid = convert<TShotID, ShotID>(tPlayJobStatus.sid);
    playJobStatus.status = convert<TEngineJobStatus, EngineJobStatus>(tPlayJobStatus.status);
    return playJobStatus;
}

template<>
bool STI::Network::convert<PlayJobStatus, TPlayJobStatus>(const PlayJobStatus& playJobStatus, TPlayJobStatus& tPlayJobStatus)
{
    tPlayJobStatus = convert<PlayJobStatus, TPlayJobStatus>(playJobStatus);
    return true;
}

template<>
bool STI::Network::convert<TPlayJobStatus, PlayJobStatus>(const TPlayJobStatus& tPlayJobStatus, PlayJobStatus& playJobStatus)
{
    playJobStatus = convert<TPlayJobStatus, PlayJobStatus>(tPlayJobStatus);
    return true;
}




//AddSequenceStatus
template<>
TAddSequenceStatus STI::Network::convert<AddSequenceStatus, TAddSequenceStatus>(
                const AddSequenceStatus& addSequenceStatus)
{
    TAddSequenceStatus tAddSequenceStatus;
    tAddSequenceStatus.seqid = convert<SequenceID, TSequenceID>(addSequenceStatus.seqid);
    tAddSequenceStatus.status = convert<EngineJobStatus, TEngineJobStatus>(addSequenceStatus.status);
    return tAddSequenceStatus;
}

template<>
AddSequenceStatus STI::Network::convert<TAddSequenceStatus, AddSequenceStatus>(
                const TAddSequenceStatus& tAddSequenceStatus)
{
    AddSequenceStatus addSequenceStatus;
    addSequenceStatus.seqid = convert<TSequenceID, SequenceID>(tAddSequenceStatus.seqid);
    addSequenceStatus.status = convert<TEngineJobStatus, EngineJobStatus>(tAddSequenceStatus.status);
    return addSequenceStatus;
}

template<>
bool STI::Network::convert<AddSequenceStatus, TAddSequenceStatus>(const AddSequenceStatus& addSequenceStatus, TAddSequenceStatus& tAddSequenceStatus)
{
    tAddSequenceStatus = convert<AddSequenceStatus, TAddSequenceStatus>(addSequenceStatus);
    return true;
}

template<>
bool STI::Network::convert<TAddSequenceStatus, AddSequenceStatus>(const TAddSequenceStatus& tAddSequenceStatus, AddSequenceStatus& addSequenceStatus)
{
    addSequenceStatus = convert<TAddSequenceStatus, AddSequenceStatus>(tAddSequenceStatus);
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
    tTime.min = timeStamp.minute();
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
bool STI::Network::convert<TShot, std::shared_ptr<Shot>>(const TShot& tShot, std::shared_ptr<Shot>& shot)
{
    bool success = false;

    if (!CORBA::is_nil(tShot.shotCallback)) {
        
        auto remoteShot = std::make_shared<STI::Network::RemoteShot>(
                    convert<TShotConfig, ShotConfig>(tShot.shotConfig), 
                    tShot.shotCallback);
        
        shot = remoteShot;
        success = (shot != 0);
    }
    return success;
}


template<>
bool STI::Network::convert<std::shared_ptr<Shot>, TShot>(const std::shared_ptr<Shot>& shot, TShot& tShot)
{
    ::STI::TNetwork::TShotCallback_ptr tShotCallback;

    if (shot != 0 && TShotRefInterface::getTShotReference(shot, tShotCallback)) {

        tShot.shotCallback = tShotCallback;
     
        tShot.shotConfig = convert<ShotConfig, TShotConfig>(shot->getShotConfig());

        return !CORBA::is_nil(tShot.shotCallback);
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


//EngineParsingMessageCount
template<>
bool STI::Network::convert<EngineParsingMessageCount, TEngineParsingMessageCount>(const EngineParsingMessageCount& parsingMessageCount, TEngineParsingMessageCount& tParsingMessageCount)
{
    tParsingMessageCount.errorCount = static_cast<CORBA::Short>(parsingMessageCount.errorCount);
    tParsingMessageCount.warningCount = static_cast<CORBA::Short>(parsingMessageCount.warningCount);
    tParsingMessageCount.infoCount = static_cast<CORBA::Short>(parsingMessageCount.infoCount);

    return true;
}

template<>
bool STI::Network::convert<TEngineParsingMessageCount, EngineParsingMessageCount>(const TEngineParsingMessageCount& tParsingMessageCount, EngineParsingMessageCount& parsingMessageCount)
{
    parsingMessageCount.errorCount = static_cast<unsigned>(tParsingMessageCount.errorCount);
    parsingMessageCount.warningCount = static_cast<unsigned>(tParsingMessageCount.warningCount);
    parsingMessageCount.infoCount = static_cast<unsigned>(tParsingMessageCount.infoCount);

    return true;
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
    STI::Network::convertEventGraphPath(measurement->getMeasurementGraphPath(), tMeasurement.measurementGraphPath);
    convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(measurement->device(), tMeasurement.device);
    convert<STI::Utils::MixedValue, STI::TNetwork::TMixedValue>(measurement->data(), tMeasurement.measurementResult);
    tMeasurement.fullGroupName = convert<std::string, CORBA::String_member>(measurement->groupName());

    return true;
}

template<>
bool STI::Network::convert<TMeasurement, std::shared_ptr<Measurement>>(
        const TMeasurement& tMeasurement, std::shared_ptr<Measurement>& measurement)
{
    STI::Utils::GraphPathLabel gpl;
    STI::Network::convertEventGraphPath(tMeasurement.measurementGraphPath, gpl);

    measurement = std::make_shared<Measurement>(
        static_cast<double>(tMeasurement.time),
        static_cast<unsigned short>(tMeasurement.channel),
        convert<STI::TNetwork::TDeviceID, STI::Device::DeviceID>(tMeasurement.device),
        gpl,
        convert<CORBA::String_member, std::string>(tMeasurement.fullGroupName)
        );

    auto data = convert<STI::TNetwork::TMixedValue, STI::Utils::MixedValue>(tMeasurement.measurementResult);
    measurement->setMeasurementResult(std::move(data));

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

//MeasurementMap
template<>
bool STI::Network::convert<TDeviceIDMeasurementsTupleSeq, std::shared_ptr<MeasurementMap>>(
        const TDeviceIDMeasurementsTupleSeq& tMeasurements, std::shared_ptr<MeasurementMap>& measurements)
{
    measurements = std::make_shared<MeasurementMap>();

    for (unsigned i = 0; i < tMeasurements.length(); ++i) {
        auto& newMeasurements = (*measurements)[convert<TDeviceID, DeviceID>(tMeasurements[i].id)];
        convert<TMeasurement, std::shared_ptr<Measurement>>(tMeasurements[i].measurements, newMeasurements);
    }
    return true;
}

template<>
bool STI::Network::convert<std::shared_ptr<MeasurementMap>, TDeviceIDMeasurementsTupleSeq>(
        const std::shared_ptr<MeasurementMap>& measurements, TDeviceIDMeasurementsTupleSeq& tMeasurements)
{
    if (measurements == 0) {
        tMeasurements.length(0);
        return true;
    }

    tMeasurements.length( measurements->size() );
    unsigned i = 0;
    for (auto& tuple : *measurements) { //tuple: {DeviceID, shared_ptr<MeasurementVector>}
        tMeasurements[i].id = convert<DeviceID, TDeviceID>(tuple.first);
        convert<std::shared_ptr<Measurement>, TMeasurement>(tuple.second, tMeasurements[i].measurements);
    }
    return true;
}


