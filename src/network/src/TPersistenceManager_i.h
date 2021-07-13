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

    ::CORBA::Boolean saveShot(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TEventEngine_ptr eventEngine);
    ::CORBA::Boolean transferMeasurements(::STI::TNetwork::TResultsCollector_ptr resultsCollector);
    ::CORBA::Boolean getResultTicket(const ::STI::TNetwork::TShotID& sid, ::STI::TNetwork::TResultTicket_out ticket);

    ::CORBA::Boolean ping();

private:

    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;

};


} //TNetwork
} //STI

#endif




