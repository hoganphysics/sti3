#ifndef STI_TNETWORK_TEVENTENGINE_I_H
#define STI_TNETWORK_TEVENTENGINE_I_H

#include "generated/deviceNet.h"

#include "EventEngine.h"
#include "RemoteTriggerCallback.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TEventEngine_i : public POA_STI::TNetwork::TEventEngine,
                       public PortableServer::RefCountServantBase
{
public:

	TEventEngine_i(STI::Engine::EventEngine* engine);
	~TEventEngine_i();

    void play(const ::STI::TNetwork::TEventEngineJob& job);
    void playCB(const ::STI::TNetwork::TEngineJobID& jobID, 
                ::STI::TNetwork::TTriggerCallback_ptr triggerCB, ::CORBA::Boolean debug);
    void trigger();
    void triggerTarget(const ::STI::TNetwork::TDeviceID& target);
    void stop();
    void pause();
    void unpause(::CORBA::Boolean retrigger);
    TDeviceID* getDeviceID();
    TEngineState getState();
    TEventEngineDependencyTree* getParsedTree();
    ::CORBA::Boolean getParseResult(const ::STI::TNetwork::TParseID& parseID, ::STI::TNetwork::TParseResult_out tParseResult);

private:

	STI::Engine::EventEngine* eventEngine;
    std::shared_ptr<STI::Network::RemoteTriggerCallback> remoteTriggerCB;

};

} //TNetwork
} //STI

#endif
