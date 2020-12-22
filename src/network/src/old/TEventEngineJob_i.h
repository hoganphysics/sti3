#ifndef STI_TNETWORK_TEVENTENGINEJOB_I_H
#define STI_TNETWORK_TEVENTENGINEJOB_I_H

#include "deviceNet.h"

#include "EventEngineJob.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TEventEngineJob_i : public POA_STI::TNetwork::TEventEngineJob
{
public:

	TEventEngineJob_i(const std::shared_ptr<STI::Engine::EventEngineJob>& engineJob);
	~TEventEngineJob_i();
	
    TEngineJobID* getJobID();
    TDeviceID* getJobOwner();
    TEngineJobStatus getStatus();
    void markRunning(const ::STI::TNetwork::TEngineID& id);
    void markComplete();
    void markCancelled();
    TEngineID getEngineID();
    ::CORBA::Boolean getEngine(::STI::TNetwork::TEventEngine_out eventEngine);
    void setEventEngine(::STI::TNetwork::TEventEngine_ptr eventEngine);
    ::CORBA::Boolean getParsedShot(::STI::TNetwork::TParsedShot_out shot);
    ::CORBA::Boolean getDependencies(::STI::TNetwork::TEventEngineDependencyTree_out tree);
    TDeviceIDSeq* getMissingTargetIDs();

private:

    std::shared_ptr<STI::Engine::EventEngineJob> eventEngineJob;

};

} //TNetwork
} //STI

#endif
