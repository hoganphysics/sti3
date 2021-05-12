#ifndef STI_NETWORK_REMOTEEVENTENGINE_H
#define STI_NETWORK_REMOTEEVENTENGINE_H

#include "deviceNet.h"

#include "EventEngine.h"
#include "TTriggerCallback_i.h"

#include <memory>

namespace STI
{
namespace Network
{

class RemoteEventEngine : public STI::Engine::EventEngine
{
public:

	RemoteEventEngine(::STI::TNetwork::TEventEngine_ptr engine);
    ~RemoteEventEngine();

	void play(STI::Engine::EventEngineJob& job);
	void play(const STI::Engine::EngineJobID& jobID, const std::shared_ptr<STI::Engine::TriggerCallback>& triggerCB, bool debug);

	void trigger();
	void trigger(const STI::Device::DeviceID& target);

	void stop();
	void pause();
	void unpause(bool retrigger);

    STI::Device::DeviceID getDeviceID() const;

	STI::Engine::EngineState getState() const;

	const STI::Engine::DeviceEventMap& getParsedEvents();

private:

//	bool getTDeviceRef(STI::TNetwork::TDevice_ptr& tDevice);

    std::shared_ptr<STI::TNetwork::TTriggerCallback_i> triggerCallbackServant;

    ::STI::TNetwork::TEventEngine_var _tEngine; //remote reference

	STI::Engine::DeviceEventMap events;
};


} //Network
} //STI


#endif

