#ifndef STI_TNETWORK_TPERSISTENCEMANAGER_I_H
#define STI_TNETWORK_TPERSISTENCEMANAGER_I_H

#include "fwd/PersistenceManager_fwd.h"
#include "deviceNet.h"
#include "Device.h"

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

    ::CORBA::Boolean getShot(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TShotResult_out tShotResult);
    ::CORBA::Boolean saveShot(const ::STI::TNetwork::TShotID& sid, const ::STI::TNetwork::TShotResult& tShotResult, ::CORBA::Boolean isOwner);
    TShotResultRecord* transferResults(::STI::TNetwork::TResultsCollector_ptr tResultsCollector);
    ::CORBA::Boolean getMeasurements(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TMeasurementSeq_out measurements);

    ::CORBA::Boolean ping();

private:

    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;

};


} //TNetwork
} //STI

#endif




