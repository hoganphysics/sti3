#ifndef STI_ENGINE_SYNCHRONOUSEVENT_H
#define STI_ENGINE_SYNCHRONOUSEVENT_H

#include <sti/fwd/MixedValue_fwd.h>
#include <sti/engine/EnginePlayingMessage.h>
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

	EnginePlayingMessage& addError(const std::string& name);
	EnginePlayingMessage& addWarning(const std::string& name);
	EnginePlayingMessage& addInfoMessage(const std::string& name);

	const std::vector<EnginePlayingMessage>& getLoadMessages() const { return loadMessages; }
	const std::vector<EnginePlayingMessage>& getPlayMessages() const { return playMessages; }
	const std::vector<EnginePlayingMessage>& getMeasureMessages() const { return measureMessages; }
	std::vector<EnginePlayingMessage>& getMessages();

private:

	virtual void loadEvent() = 0;
	virtual void playEvent() = 0;
	virtual void collectMeasurementData() = 0;
	virtual void stopEvent() = 0;
	virtual void pauseEvent() = 0;
	virtual void unpauseEvent(bool retrigger) = 0;

	void waitForPlayComplete();

	EnginePlayingMessage& addMessage(const std::string& name, const PlayingMessageType& type);

	bool played;
	bool loaded;
	bool stopped;
	bool paused;

	double _time;
	std::vector<std::shared_ptr<Measurement>> measurements;
	std::vector<EnginePlayingMessage> loadMessages;
	std::vector<EnginePlayingMessage> playMessages;
	std::vector<EnginePlayingMessage> measureMessages;
	std::vector<EnginePlayingMessage> messages;

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
