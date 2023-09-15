#ifndef STI_TNETWORK_TEVENTENGINESCHEDULER_I_H
#define STI_TNETWORK_TEVENTENGINESCHEDULER_I_H

#include <sti/engine/EventEngineScheduler.h>
#include <sti/device/Device.h>
#include "TEventEngineDependencyParser_i.h"
#include "generated/deviceNet.h"

#include <memory>


namespace STI
{
namespace TNetwork
{

class TEventEngineScheduler_i : public POA_STI::TNetwork::TEventEngineScheduler
{
public:

	TEventEngineScheduler_i(const std::shared_ptr<STI::Device::Device>& device);
	~TEventEngineScheduler_i();

    TParseJobStatus* parse(const ::STI::TNetwork::TShot& shot);
    TPlayJobStatus* play(const ::STI::TNetwork::TParseID& parseID, const ::STI::TNetwork::TEngineJobSourceID& source);
    TAddSequenceStatus* addSequence(const ::STI::TNetwork::TSequence& tSequenceData, const ::STI::TNetwork::TEngineJobSourceID& source);
    TParseJobStatus* parseSeqEntry(const ::STI::TNetwork::TShot& shot, const ::STI::TNetwork::TSequenceEntryID& sequenceEntryID);
    
    TEngineJobStatus getStatusPID(const ::STI::TNetwork::TParseID& pid);
    TEngineJobStatus getStatusSID(const ::STI::TNetwork::TShotID& sid);

    TEventEngineDependencyParser_ptr getDependencyParser();

    ::CORBA::Boolean getJob(const ::STI::TNetwork::TEngineJobID& id, ::STI::TNetwork::TEventEngineJob_out job);
    void addJob(const ::STI::TNetwork::TEventEngineJob& newJob);
    void cancelJob(const ::STI::TNetwork::TEngineJobID& jobID);
    void cancelAll();

    TEngineJobIDSeq* getJobIDs(::STI::TNetwork::TEventEngineJobList jobListType);
    TEventEngineJobSeq* getJobs(::STI::TNetwork::TEventEngineJobList jobListType);

    ::CORBA::Boolean getParseResult(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TParseResult_out tParseResult);

    ::CORBA::Boolean ping();

private:

    std::shared_ptr<TEventEngineDependencyParser_i> dependencyParserServant;
    std::shared_ptr<STI::Engine::EventEngineScheduler> engineScheduler;
};

} //TNetwork
} //STI

#endif
