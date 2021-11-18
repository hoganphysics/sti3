#ifndef STI_ENGINE_EVENTENGINE_H
#define STI_ENGINE_EVENTENGINE_H

#include "EngineState.h"
#include "DeviceID.h"
#include "fwd/RawEvent_fwd.h"
#include "fwd/Measurement_fwd.h"

#include <memory>

namespace STI
{
namespace Engine
{

class TriggerCallback;
class EventEngineJob;
class EngineJobID;
class ParseID;
class ShotID;
class ResultsCollector;
class ParsedDependencyTree;


class EventEngine
{
public:

	virtual ~EventEngine() {}

	virtual void play(EventEngineJob& job) = 0;
	virtual void play(const EngineJobID& jobID, const std::shared_ptr<TriggerCallback>& triggerCB, bool debug = false) = 0;

	virtual void trigger() = 0;
	virtual void trigger(const STI::Device::DeviceID& target) = 0;		//triggers just target

	virtual void stop() = 0;
	virtual void pause() = 0;
	virtual void unpause(bool retrigger) = 0;

    virtual STI::Device::DeviceID getDeviceID() const = 0;

	virtual STI::Engine::EngineState getState() const = 0;

	virtual std::shared_ptr<ParsedDependencyTree> getParsedTree() const = 0;
	virtual bool getParsedEvents(const ParseID& parseID, DeviceEventMap& parsedEvents) = 0;
	// virtual const DeviceEventMap& getParsedEvents() = 0;
	
	//virtual bool getMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements) = 0;
	// virtual bool transferMeasurements(const ShotID& sid, std::shared_ptr<MeasurementVector>& measurements) = 0;

	//virtual bool transferResults(const std::shared_ptr<ResultsCollector>& resultsCollector) = 0;
};

} //Engine
} //STI

#endif
