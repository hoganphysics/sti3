#ifndef STI_ENGINE_SYNCHRONOUSEVENT_H
#define STI_ENGINE_SYNCHRONOUSEVENT_H

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

	SynchronousEvent(double time);	//Time type?
	virtual ~SynchronousEvent();

	double getTime() const { return _time; }
	const std::vector<std::shared_ptr<Measurement>>& getMeasurements() const { return measurements; }
	void addMeasurement(const RawEvent& sourceEvent);

	void load();
	void play();
	void collectData();
	void stop();
	void pause();
	void unpause(bool retrigger);

	void reset();		//must call before playing again
	void unload();		//call to indicate that the event is no longer loaded

	bool operator<(const SynchronousEvent& rhs) const { return getTime() < rhs.getTime(); }

private:

	virtual void loadEvent() = 0;
	virtual void playEvent() = 0;
	virtual void collectMeasurementData() = 0;
	virtual void stopEvent() = 0;
	virtual void pauseEvent() = 0;
	virtual void unpauseEvent(bool retrigger) = 0;

	void wait();

	bool played;
	bool loaded;
	bool stopped;
	bool paused;

	double _time;
	std::vector<std::shared_ptr<Measurement>> measurements;

	mutable std::mutex evtMutex;
	mutable std::condition_variable condition;
};

} //Engine
} //STI

#endif

