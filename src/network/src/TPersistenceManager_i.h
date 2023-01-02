#ifndef STI_TNETWORK_TPERSISTENCEMANAGER_I_H
#define STI_TNETWORK_TPERSISTENCEMANAGER_I_H

#include "fwd/PersistenceManager_fwd.h"
#include "deviceNet.h"
#include <sti/device/Device.h>

#include <memory>


namespace STI
{
namespace TNetwork
{


class TPersistenceManager_i : public POA_STI::TNetwork::TPersistenceManager
{
public:

	TPersistenceManager_i(const std::shared_ptr<STI::Device::Device>& device);
	~TPersistenceManager_i();
    
    ::CORBA::Boolean findShot(const ::STI::TNetwork::TShotID& sid);
    ::CORBA::Boolean getParseResult(const ::STI::TNetwork::TParseID& pid, ::STI::TNetwork::TParseResult_out tParseResult);
    ::CORBA::Boolean getShotResult(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TShotResult_out tShotResult);
    ::CORBA::Boolean getSequenceResult(const ::STI::TNetwork::TSequenceID& seqid, ::STI::TNetwork::TSequenceResult_out tSequenceResult);
    ::CORBA::Boolean saveShot(const ::STI::TNetwork::TShotID& sid, const ::STI::TNetwork::TFullShotResult& tFullShotResult, ::CORBA::Boolean isOwner);
    TShotResultRecord* transferResults(::STI::TNetwork::TResultsCollector_ptr tResultsCollector);
    ::CORBA::Boolean getMeasurements(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TDeviceIDMeasurementsTupleSeq_out measurements);
    void addSequence(const ::STI::TNetwork::TSequenceResult& tSequenceResult);
    ::CORBA::Boolean updateSequence(const ::STI::TNetwork::TSequenceEntryID& id, const ::STI::TNetwork::TShotID& shotID, ::STI::TNetwork::TEngineJobStatus shotStatus, ::CORBA::Boolean isOwner);
    ::CORBA::Boolean saveSequence(const ::STI::TNetwork::TSequenceResult& tSequenceResult, ::CORBA::Boolean isOwner);
    ::CORBA::Boolean ping();

private:

    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;

};


} //TNetwork
} //STI

#endif




