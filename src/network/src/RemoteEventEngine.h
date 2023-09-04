#ifndef STI_NETWORK_REMOTEEVENTENGINE_H
#define STI_NETWORK_REMOTEEVENTENGINE_H

#include "generated/deviceNet.h"

#include "EventEngine.h"
#include "TTriggerCallback_i.h"
#include "TReferenceHolder.h"

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

	std::shared_ptr<STI::Engine::ParsedDependencyTree> getParsedTree() const;
	// bool getParsedEvents(const STI::Engine::ParseID& parseID, STI::Engine::DeviceEventMap& parsedEvents);

	bool getParseResult(const STI::Engine::ParseID& parseID, std::shared_ptr<STI::Engine::ParseResult>& parseResult) const;

	//bool getMeasurements(const STI::Engine::ShotID& sid, std::shared_ptr<STI::Engine::MeasurementVector>& measurements);
	//bool transferResults(const std::shared_ptr<STI::Engine::ResultsCollector>& resultsCollector);

private:

//	bool getTDeviceRef(STI::TNetwork::TDevice_ptr& tDevice);

    std::shared_ptr<STI::TNetwork::TTriggerCallback_i> triggerCallbackServant;

    //::STI::TNetwork::TEventEngine_var _tEngine; //remote reference

	// STI::Engine::DeviceEventMap events;

	mutable std::mutex engineMutex;
};


} //Network
} //STI


#endif

