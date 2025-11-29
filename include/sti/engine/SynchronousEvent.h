#ifndef STI_ENGINE_SYNCHRONOUSEVENT_H
#define STI_ENGINE_SYNCHRONOUSEVENT_H

#include <sti/fwd/MixedValue_fwd.h>
#include <sti/utils/FileHolder.h>

#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>


namespace STI
{
namespace Engine
{

class RawEvent;
class Measurement;


class SynchronousEvent
{
public:

	SynchronousEvent(double time);
	virtual ~SynchronousEvent();

	double getTime() const { return _time; }
	void setTime(double time) { _time = time; }

	const std::vector<std::shared_ptr<Measurement>>& getMeasurements() const { return measurements; }
	void addMeasurement(const RawEvent& sourceEvent);
	bool setMeasurementResult(const STI::Utils::MixedValue& result);
	bool setMeasurementResult(unsigned index, const STI::Utils::MixedValue& result);
	bool attachFile(const std::shared_ptr<STI::Utils::FileHolder>& file);

	void load();
	void play();
	void collectData();
	void stop();
	void pause();
	void unpause(bool retrigger);

	//Custom waiters
	//Note: these wait functions *must* return/abort when stopEvent() is called!
	virtual void waitBeforePlay() {}		//adds extra wait before playEvent (after cpu waits for getTime())
	virtual void waitBeforeCollectData() {}	//adds extra wait before collectMeasurementData (after play())

	void reset();		//must call before playing again
	void unload();		//call to indicate that the event is no longer loaded

	virtual bool unloadEvent() { return true; }

	bool operator<(const SynchronousEvent& rhs) const { return getTime() < rhs.getTime(); }

private:

	virtual void loadEvent() = 0;
	virtual void playEvent() = 0;
	virtual void collectMeasurementData() = 0;
	virtual void stopEvent() = 0;
	virtual void pauseEvent() = 0;
	virtual void unpauseEvent(bool retrigger) = 0;

	void waitForPlayComplete();

	bool played;
	bool loaded;
	bool stopped;
	bool paused;

	double _time;
	std::vector<std::shared_ptr<Measurement>> measurements;

	mutable std::mutex evtMutex;
	mutable std::condition_variable condition;
};


class SynchronousEventAdapter : public SynchronousEvent
{
public:

	SynchronousEventAdapter(double time) : SynchronousEvent(time) {}
	virtual ~SynchronousEventAdapter() {}

	virtual void loadEvent() {}
	virtual void playEvent() {}
	virtual void collectMeasurementData() {}
	virtual void stopEvent() {}
	virtual void pauseEvent() {}
	virtual void unpauseEvent(bool retrigger) {}
	
};


} //Engine
} //STI

#endif

