#ifndef STI_TNETWORK_TEVENTENGINESCHEDULER_I_H
#define STI_TNETWORK_TEVENTENGINESCHEDULER_I_H

#include "EventEngineScheduler.h"
#include "Device.h"

#include "deviceNet.h"

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

    void parse(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TShot_ptr shot) ;
    void play(const ::STI::TNetwork::TShotID& shotID);

    void getDependants(const ::STI::TNetwork::TDeviceIDSeq& evtTargets, 
                        ::STI::TNetwork::TEventEngineDependencyTree& tree, 
                        ::STI::TNetwork::TDeviceIDSeq& missingTargets, 
                        ::STI::TNetwork::TEngineParsingMessageSeq_out messages, 
                        const ::STI::TNetwork::TDeviceTrace& trace);
    
    void addDeviceEventTargets(::STI::TNetwork::TEventEngineDependencyTree& tree, 
                                ::STI::TNetwork::TEngineParsingMessageSeq_out messages, 
                                const ::STI::TNetwork::TDeviceTrace& trace);



    //void addJob(::STI::TNetwork::TEventEngineJob_ptr newJob);
    void addJob(const ::STI::TNetwork::TEventEngineJob& newJob);
    void cancelJob(const ::STI::TNetwork::TEngineJobID& jobID);
    void cancelAll();

    ::CORBA::Boolean getParsedEvents(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TDeviceEventsSeq_out events);
    ::CORBA::Boolean getParsingMessages(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TEngineParsingMessageSeq_out messages);
    ::CORBA::Boolean getParsedTree(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TEventEngineDependencyTree_out tree);

//    ::CORBA::Boolean transferResults(::STI::TNetwork::TResultsCollector_ptr resultsCollector);
//    ::CORBA::Boolean getResults(const ::STI::TNetwork::TShotID& shotID, ::STI::TNetwork::TResultTicket_out results);

    ::CORBA::Boolean ping();

private:

    std::shared_ptr<STI::Engine::EventEngineScheduler> engineScheduler;

};

} //TNetwork
} //STI

#endif
