#ifndef STI_ENGINE_PSEUDOSYNCHRONOUSEVENT_H
#define STI_ENGINE_PSEUDOSYNCHRONOUSEVENT_H

#include <sti/device/Device.h>
#include <sti/engine/RawEvent.h>
#include <sti/engine/SynchronousEvent.h>


namespace STI
{
namespace Engine
{

class PseudoSynchronousEvent : public SynchronousEvent
{
public:

	PseudoSynchronousEvent(double time, const STI::Engine::RawEventVector& eventsIn, STI::Device::Device* device);
	~PseudoSynchronousEvent();

	void loadEvent();
	void playEvent();
	void collectMeasurementData();
	void stopEvent();
	void pauseEvent() {}
	void unpauseEvent(bool retrigger) {}

private:

	STI::Engine::RawEventVector events;
	STI::Device::Device* device;
};

} //Engine
} //STI

#endif

