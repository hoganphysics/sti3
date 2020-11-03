#ifndef STI_ENGINE_EVENTENGINE_H
#define STI_ENGINE_EVENTENGINE_H

#include "EngineState.h"
#include "DeviceID.h"

namespace STI
{
namespace Engine
{

class TriggerCallback;
class EventEngineJob;
class EngineJobID;


class EventEngine
{
public:

	virtual ~EventEngine() {}

	virtual void play(const EventEngineJob& job) = 0;
	virtual void play(const EngineJobID& jobID, TriggerCallback& triggerCB, bool debug = false) = 0;

	virtual void trigger() = 0;
	virtual void trigger(STI::Device::DeviceID& target) = 0;		//triggers just target

	virtual void stop() = 0;
	virtual void pause() = 0;
	virtual void unpause(bool retrigger) = 0;

    virtual STI::Device::DeviceID getDeviceID() const = 0;

	virtual STI::Engine::EngineState getState() const = 0;
};

} //Engine
} //STI

#endif
