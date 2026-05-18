#ifndef STI_TNETWORK_TEVENTENGINESCHEDULER_I_H
#define STI_TNETWORK_TEVENTENGINESCHEDULER_I_H

#include <sti/engine/EventEngineScheduler.h>
#include <sti/device/Device.h>
#include "TEventEngineDependencyParser_i.h"
#include "generated/deviceNet.h"
#include "ServantHolder.h"

#include <memory>


namespace STI
{
namespace TNetwork
{

class TEventEngineScheduler_i : public POA_STI::TNetwork::TEventEngineScheduler,
                                public PortableServer::RefCountServantBase
{
public:

	TEventEngineScheduler_i(const std::shared_ptr<STI::Device::Device>& device);
	~TEventEngineScheduler_i();

    TParseJobStatus* parse(const ::STI::TNetwork::TShot& shot);
    TParseJobStatus* parseSeqEntry(const ::STI::TNetwork::TShot& shot, const ::STI::TNetwork::TSequenceEntryID& sequenceEntryID);
    TParseJobStatus* parseSeq(const ::STI::TNetwork::TShot& shot, const ::STI::TNetwork::TSequenceID& sequenceID);
    TPlayJobStatus* play(const ::STI::TNetwork::TParseID& parseID, const ::STI::TNetwork::TEngineJobSourceID& source);
    TAddSequenceStatus* addSequence(const ::STI::TNetwork::TSequence& tSequenceData, const ::STI::TNetwork::TEngineJobSourceID& source);
    void closeSequence(const ::STI::TNetwork::TSequenceID& seqid);
    void cancelSequence(const ::STI::TNetwork::TSequenceID& seqid);

    TEngineJobStatus getStatusPID(const ::STI::TNetwork::TParseID& pid);
    TEngineJobStatus getStatusSID(const ::STI::TNetwork::TShotID& sid);
    TEngineJobStatus getStatusSeqID(const ::STI::TNetwork::TSequenceID& seqID);

    TEventEngineDependencyParser_ptr getDependencyParser();

    ::CORBA::Boolean getJob(const ::STI::TNetwork::TEngineJobID& id, ::STI::TNetwork::TEventEngineJob_out job);
    void addJob(const ::STI::TNetwork::TEventEngineJob& newJob);
    void cancelJob(const ::STI::TNetwork::TEngineJobID& jobID);
    void cancelAll();

    TEngineJobIDSeq* getJobIDs(::STI::TNetwork::TEventEngineJobList jobListType);
    TEventEngineJobSeq* getJobs(::STI::TNetwork::TEventEngineJobList jobListType);

    void getEngineIDs(::STI::TNetwork::TEngineIDSeq_out engineIDs);
    TEngineState getEngineState(const ::STI::TNetwork::TEngineID& engineID);
    void getEngineStates(::STI::TNetwork::TEngineStateTupleSeq_out engineStates);

    void clearEngine(const ::STI::TNetwork::TEngineID& engineID);
    void stopEngine(const ::STI::TNetwork::TEngineID& engineID);

    ::CORBA::Boolean getParseResult(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TParseResult_out tParseResult);
    ::CORBA::Boolean getShotResult(const ::STI::TNetwork::TShotID& shotID, ::STI::TNetwork::TShotResult_out shotResult);
    ::CORBA::Boolean getLastParseResult(const ::STI::TNetwork::TEngineID& engineID, ::STI::TNetwork::TParseResult_out tParseResult);
    ::CORBA::Boolean getLastShotResult(const ::STI::TNetwork::TEngineID& engineID, ::STI::TNetwork::TShotResult_out shotResult);

    ::CORBA::Boolean ping();

private:

    // std::shared_ptr<TEventEngineDependencyParser_i> dependencyParserServant;
    STI::Network::ServantHolder<TEventEngineDependencyParser_i, TEventEngineDependencyParser> dependencyParserServantHolder;
    std::shared_ptr<STI::Engine::EventEngineScheduler> engineScheduler;
};

} //TNetwork
} //STI

#endif
