#ifndef STI_TNETWORK_TEVENTENGINE_I_H
#define STI_TNETWORK_TEVENTENGINE_I_H

#include "deviceNet.h"

#include "EventEngine.h"
#include "RemoteTriggerCallback.h"

#include <memory>

namespace STI
{
namespace TNetwork
{

class TEventEngine_i : public POA_STI::TNetwork::TEventEngine
{
public:

	//TEventEngine_i(const std::shared_ptr<STI::Engine::EventEngine>& engine);
	TEventEngine_i(STI::Engine::EventEngine* engine);
	~TEventEngine_i();

    void play(const ::STI::TNetwork::TEventEngineJob& job);
    //void play(::STI::TNetwork::TEventEngineJob_ptr job);
    void playCB(const ::STI::TNetwork::TEngineJobID& jobID, 
                ::STI::TNetwork::TTriggerCallback_ptr triggerCB, ::CORBA::Boolean debug);
    void trigger();
    void triggerTarget(const ::STI::TNetwork::TDeviceID& target);
    void stop();
    void pause();
    void unpause(::CORBA::Boolean retrigger);
    TDeviceID* getDeviceID();
    TEngineState getState();

private:

	//std::shared_ptr<STI::Engine::EventEngine> eventEngine;
	STI::Engine::EventEngine* eventEngine;

    std::shared_ptr<STI::Network::RemoteTriggerCallback> remoteTriggerCB;

};

} //TNetwork
} //STI

#endif
