
#include "TEventEngineJob_i.h"

#include "NetworkConvert.h"
#include "EngineID.h"
#include "EventEngine.h"
#include "ORBManager.h"

using STI::TNetwork::TEventEngineJob_i;
using STI::TNetwork::TDeviceID;
using STI::TNetwork::TEngineJobID;
using STI::TNetwork::TEngineJobStatus;
using ::STI::TNetwork::TEngineID;
using STI::Engine::EventEngineJob;
using ::STI::TNetwork::TDeviceIDSeq;
using STI::Device::DeviceID;
using STI::Engine::EngineJobID;
using STI::Engine::EventEngineJob;
using STI::Engine::EventEngine;
using STI::Network::convert;
using STI::Engine::EngineJobStatus;


TEventEngineJob_i::TEventEngineJob_i(const std::shared_ptr<EventEngineJob>& engineJob)
: eventEngineJob(engineJob)
{
}

TEventEngineJob_i::~TEventEngineJob_i()
{
    STI::Network::ORBManager::ORBManager::deactivateServant(this);
}

TEngineJobID* TEventEngineJob_i::getJobID()
{
	STI::TNetwork::TEngineJobID_var tJobID(new STI::TNetwork::TEngineJobID);

	if(eventEngineJob != 0) {
		convert<EngineJobID, TEngineJobID>(eventEngineJob->getJobID(), tJobID);
	}

	return tJobID._retn();
}

TDeviceID* TEventEngineJob_i::getJobOwner()
{
    STI::TNetwork::TDeviceID_var tDevice(new STI::TNetwork::TDeviceID);

	if(eventEngineJob != 0) {
		convert<DeviceID, TDeviceID>(eventEngineJob->getJobOwner(), tDevice);
	}

	return tDevice._retn();
}

TEngineJobStatus TEventEngineJob_i::getStatus()
{
    TEngineJobStatus tStatus;

    if (eventEngineJob != 0) {
        convert<EngineJobStatus, TEngineJobStatus>(eventEngineJob->getStatus(), tStatus);
	}
    return tStatus;
}

void TEventEngineJob_i::markRunning(const TEngineID& id)
{
    if (eventEngineJob != 0) {
        eventEngineJob->markRunning( convert<STI::TNetwork::TEngineID, STI::Engine::EngineID>(id) );
	}
}

void TEventEngineJob_i::markComplete()
{
    if (eventEngineJob != 0) {
        eventEngineJob->markComplete();
	}
}

void TEventEngineJob_i::markCancelled()
{
    if (eventEngineJob != 0) {
        eventEngineJob->markCancelled();
	}
}

TEngineID TEventEngineJob_i::getEngineID()
{
    TEngineID tID;

    if (eventEngineJob != 0) {
        convert<STI::Engine::EngineID, TEngineID>(eventEngineJob->getEngineID(), tID);
	}
    return tID;
}

::CORBA::Boolean TEventEngineJob_i::getEngine(::STI::TNetwork::TEventEngine_out eventEngine)
{
    bool success = false;
    std::shared_ptr<EventEngine> engine;

    if (eventEngineJob != 0) {
        
        success = eventEngineJob->getEngine(engine) && engine != 0;
	}

	if (!success) {
		return false;
	}

	STI::TNetwork::TEventEngine_ptr tEngine;
	success = TEngineRefInterface::getTEngineReference(engine, tEngine);

	if (success) {
		
		STI::TNetwork::TEventEngine_var tEngineVar(tEngine);		//managed

		eventEngine = tEngineVar.out();
	}

    return success;
}

void TEventEngineJob_i::setEventEngine(::STI::TNetwork::TEventEngine_ptr eventEngine)
{
	if (deviceCollection != 0 && !CORBA::is_nil(device)) {
		
		//wrap the received TDevice reference in RemoteDevice
		std::shared_ptr<RemoteDevice> remoteDevice = std::make_shared<RemoteDevice>(device);
		
		return (remoteDevice != 0 &&
			deviceCollection->add(convert<TDeviceID, DeviceID>(deviceID), remoteDevice)
			);
	}
	return false;
}

::CORBA::Boolean TEventEngineJob_i::getParsedShot(::STI::TNetwork::TShot_out shot)
{
}

::CORBA::Boolean TEventEngineJob_i::getDependencies(::STI::TNetwork::TEventEngineDependencyTree_out tree)
{
}

TDeviceIDSeq* TEventEngineJob_i::getMissingTargetIDs()
{
    std::set<STI::Device::DeviceID> ids;

	if (eventEngineJob != 0) {
        ids = eventEngineJob->getMissingTargetIDs();
	}

	STI::TNetwork::TDeviceIDSeq_var tDeviceIDs(new STI::TNetwork::TDeviceIDSeq);
	convert<STI::Device::DeviceID, STI::TNetwork::TDeviceID>(ids, (_CORBA_Unbounded_Sequence<STI::TNetwork::TDeviceID>&) tDeviceIDs);

	return tDeviceIDs._retn();
}
