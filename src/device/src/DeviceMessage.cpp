#include <sti/device/DeviceMessage.h>
#include <sti/device/DeviceID.h>
#include <sti/engine/Shot.h>
#include <sti/engine/RawEventGroup.h>
#include <sti/engine/SequenceJob.h>


using STI::Device::DeviceMessage;
using STI::Device::DeviceMessageType;
using STI::Device::RefreshDeviceMessage;
using STI::Device::EngineJobUpdateDeviceMessage;


DeviceMessage::DeviceMessage(const STI::Device::DeviceID& source, DeviceMessageType type)
: _trace(source), _type(type)
{
}

DeviceMessage::DeviceMessage(const STI::Device::DeviceTrace& trace, DeviceMessageType type)
: _trace(trace), _type(type)
{
}

DeviceMessage::~DeviceMessage()
{
}

void DeviceMessage::addRelayingID(const STI::Device::DeviceID& relayingID)
{
	_trace.addID(relayingID);
}

const STI::Device::DeviceID DeviceMessage::sourceID() const
{ 
	return _trace.last();
}

const STI::Device::DeviceID DeviceMessage::originalSourceID() const
{
	return _trace.first();
}

DeviceMessageType DeviceMessage::getType() const 
{ 
	return _type;
}

const STI::Device::DeviceTrace& DeviceMessage::getDeviceTrace() const
{
	return _trace;
}

std::string DeviceMessage::typeToString(const DeviceMessageType& type)
{
	std::string name;

	switch (type)
	{
	case DeviceMessageType::Refresh:
		name = "Refresh";
		break;
	case DeviceMessageType::CollectionUpdate:
		name = "CollectionUpdate";
		break;
	case DeviceMessageType::ChannelUpdate:
		name = "ChannelUpdate";
		break;
	case DeviceMessageType::ChannelsRefresh:
		name = "ChannelsRefresh";
		break;
	case DeviceMessageType::AttributeUpdate:
		name = "AttributeUpdate";
		break;
	case DeviceMessageType::AttributesRefresh:
		name = "AttributesRefresh";
		break;
	case DeviceMessageType::MonitorUpdate:
		name = "MonitorUpdate";
		break;
	case DeviceMessageType::EngineScheduler:
		name = "EngineScheduler";
		break;
	case DeviceMessageType::EngineParser:
		name = "EngineParser";
		break;
	case DeviceMessageType::EngineStatus:
		name = "EngineStatus";
		break;
	case DeviceMessageType::EngineJobUpdate:
		name = "EngineJobUpdate";
		break;
	case DeviceMessageType::Unknown:
		name = "Unknown";
		break;
	default:
		name = "Unknown";
		break;
	}

	return name;
}



void EngineJobUpdateDeviceMessage::setJob(const std::shared_ptr<STI::Engine::EventEngineJob>& engineJob)
{
	jobID = engineJob->getJobID();
	jobOwner = engineJob->getJobOwner();
	jobStatus = engineJob->getStatus();
	engineID = engineJob->getEngineID();

	std::shared_ptr<STI::Engine::Shot> shot;
	if (engineJob->getShot(shot)) {
		shotConfig = shot->getShotConfig();

		std::shared_ptr<STI::Engine::RawEventGroup> rootGroup;
		shot->getRootEventGroup(rootGroup);

		if (shot != 0) {
			overwrittenVars =rootGroup->getOverwrittenVars();
		}
	}

	parsingMessageCount.setCounts(engineJob->getParsingMessages());
}

void EngineJobUpdateDeviceMessage::setSequenceJob(const std::shared_ptr<STI::Engine::SequenceJob>& job)
{
	jobID = job->jobID;
	jobOwner = job->jobOwner;
	jobStatus = job->getJobStatus();
	
	// engineID = job->;
}