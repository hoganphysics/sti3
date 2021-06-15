
#ifndef STI_ENGINE_JEVENTENGINE_H
#define STI_ENGINE_JEVENTENGINE_H

#include "DeviceID.h"

#include <memory>


namespace STI
{
namespace Engine
{

class TriggerCallback;
class JEventEngineJob;
class EngineJobID;
class ParseID;
class EventEngine;
enum class EngineState;

class JEventEngine
{
public:
    
    JEventEngine(const std::shared_ptr<EventEngine>& engine);
	~JEventEngine();

	void play(const std::shared_ptr<JEventEngineJob>& job);

	void stop();
	void pause();
	void unpause(bool retrigger);

    STI::Device::DeviceID getDeviceID() const;

	STI::Engine::EngineState getState() const;

	// bool getParsedEvents(const ParseID& parseID, DeviceEventMap& parsedEvents);
	// virtual const DeviceEventMap& getParsedEvents() = 0;
private:

    std::shared_ptr<EventEngine> engine;

};

} //Engine
} //STI

#endif
