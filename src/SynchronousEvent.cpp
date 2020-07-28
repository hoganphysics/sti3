
#include "SynchronousEvent.h"
#include "Measurement.h"
#include "RawEvent.h"


using STI::Engine::SynchronousEvent;
using STI::Engine::RawEvent;
using STI::Engine::Measurement;


SynchronousEvent::SynchronousEvent(double time) : _time(time)
{
	loaded = false;
	played = false;
	stopped = false;
	paused = false;
}

SynchronousEvent::~SynchronousEvent()
{
}

void SynchronousEvent::addMeasurement(const RawEvent& measurementEvent)
{
	//Accepts any RawEvent, even if not a measurement.
	//However if it's not a measurement event, it will cause an error during parsing.
	measurements.push_back(std::make_shared<Measurement>(measurementEvent));
}

void SynchronousEvent::load()
{
	std::unique_lock<std::mutex> loadLock(evtMutex);

	if (loaded)
		return;

	loadEvent();	//pure virtual

	loaded = true;
	stopped = false;
}

void SynchronousEvent::unload()
{
	std::unique_lock<std::mutex> loadLock(evtMutex);
	loaded = false;
}

void SynchronousEvent::play()
{
	std::unique_lock<std::mutex> playLock(evtMutex);

	if (played || stopped || paused)
		return;

	playEvent();	//pure virtual

	played = true;
	condition.notify_all();		//wake collectData()

}

void SynchronousEvent::collectData()
{
	wait();						//until play completes
	collectMeasurementData();	//pure virtual
}

void SynchronousEvent::stop()
{
	std::unique_lock<std::mutex> writeLock(evtMutex);
	
	stopEvent();

	stopped = true;
	condition.notify_all();
}

void SynchronousEvent::wait()
{
	std::unique_lock<std::mutex> playLock(evtMutex);

	while (!played && !stopped) {
		condition.wait(playLock);
	}
}

void SynchronousEvent::reset()
{
	std::unique_lock<std::mutex> writeLock(evtMutex);

	if (played) {
		//Replace all Measurement pointers with new Measurements in preparation for next shot.
		//References to the old Measurements are already stored elsewhere.
		//Uses the old Measurements to construct new ones (with empty data).
		for (auto& m : measurements) {
			m = std::make_shared<Measurement>(*m);
		}
	}

	played = false;
	stopped = false;
	paused = false;
}

void SynchronousEvent::pause()
{
	std::unique_lock<std::mutex> playLock(evtMutex);

	paused = true;

	pauseEvent();
}

void SynchronousEvent::unpause(bool retrigger)
{
	std::unique_lock<std::mutex> playLock(evtMutex);

	paused = false;

	unpauseEvent(retrigger);
}
