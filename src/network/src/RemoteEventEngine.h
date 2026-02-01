#ifndef STI_NETWORK_REMOTEEVENTENGINE_H
#define STI_NETWORK_REMOTEEVENTENGINE_H

#include "generated/deviceNet.h"

#include "EventEngine.h"
#include "TTriggerCallback_i.h"
#include "TReferenceHolder.h"
#include "ServantHolder.h"

#include <memory>
#include <mutex>


namespace STI
{
namespace Network
{

class RemoteEventEngine : public STI::Engine::EventEngine,
						  public STI::TNetwork::TReferenceHolder<STI::TNetwork::TEventEngine>	//mixin
{
public:

	RemoteEventEngine(::STI::TNetwork::TEventEngine_var engine);
    ~RemoteEventEngine();

	void play(STI::Engine::EventEngineJob& job);
	void play(const STI::Engine::EngineJobID& jobID, const std::shared_ptr<STI::Engine::TriggerCallback>& triggerCB, bool debug);

	void trigger();
	void trigger(const STI::Device::DeviceID& target);

	void stop();
	void pause();
	void unpause(bool retrigger);

	void clear();

    STI::Device::DeviceID getDeviceID() const;

	STI::Engine::EngineState getState() const;

	std::shared_ptr<STI::Engine::ParsedDependencyTree> getParsedTree() const;

	bool getParseResult(const STI::Engine::ParseID& parseID, std::shared_ptr<STI::Engine::ParseResult>& parseResult) const;

private:

    // std::shared_ptr<STI::TNetwork::TTriggerCallback_i> triggerCallbackServant;
	// ServantHolder<STI::TNetwork::TTriggerCallback_i, STI::TNetwork::TTriggerCallback> triggerCallbackServantHolder;
	ServantHolder<STI::TNetwork::TTriggerCallback_i, STI::TNetwork::TTriggerCallback> triggerCallbackServantHolder;

	mutable std::mutex engineMutex;
};


} //Network
} //STI


#endif

