#include <sti/engine/SynchronousEvent.h>

#include <sti/engine/Measurement.h>
#include <sti/engine/RawEvent.h>
#include <sti/utils/MixedValue.h>

#include "LocalEventEngineScheduler.h"

using STI::Engine::SynchronousEvent;
using STI::Engine::RawEvent;
using STI::Engine::Measurement;
using STI::Engine::PlayingMessageType;
using STI::Engine::EnginePlayingMessage;


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

bool SynchronousEvent::setMeasurementResult(const STI::Utils::MixedValue& result)
{
	return setMeasurementResult(0, result);
}

bool SynchronousEvent::setMeasurementResult(unsigned index, const STI::Utils::MixedValue& result)
{
	if (measurements.size() > index) {
		measurements.at(index)->setMeasurementResult(result);
		return true;
	}
	return false;
}

bool SynchronousEvent::attachFile(const std::shared_ptr<STI::Utils::FileHolder>& file)
{
	if (measurements.size() == 0) return false;

	return measurements.at(0)->attachFile(file);	//can attach to any of the measurements (attaches to common VirtualFileServer)
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
	if (unloadEvent()) {
		loaded = false;
	}
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
	waitForPlayComplete();		//until play completes
	waitBeforeCollectData();	//optional custom waiter
	collectMeasurementData();	//pure virtual
}

void SynchronousEvent::stop()
{
	std::unique_lock<std::mutex> writeLock(evtMutex);
	
	stopEvent();

	stopped = true;
	condition.notify_all();
}


void SynchronousEvent::waitForPlayComplete()
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

	loadMessages.clear();
	playMessages.clear();
	measureMessages.clear();
	messages.clear();
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

std::vector<EnginePlayingMessage>& SynchronousEvent::getMessages()
{
	std::unique_lock<std::mutex> readLock(evtMutex);

	messages.clear();
	messages.insert(messages.end(), loadMessages.begin(), loadMessages.end());
	messages.insert(messages.end(), playMessages.begin(), playMessages.end());
	messages.insert(messages.end(), measureMessages.begin(), measureMessages.end());

	return messages;
}

EnginePlayingMessage& SynchronousEvent::addError(const std::string& name)
{
	return addMessage(name, PlayingMessageType::Error);
}

EnginePlayingMessage& SynchronousEvent::addWarning(const std::string& name)
{
	return addMessage(name, PlayingMessageType::Warning);
}

EnginePlayingMessage& SynchronousEvent::addInfoMessage(const std::string& name)
{
	return addMessage(name, PlayingMessageType::Information);
}

EnginePlayingMessage& SynchronousEvent::addMessage(const std::string& name, const PlayingMessageType& type)
{
	unsigned id = 0;	//default ID
	const auto& messageIDs = LocalEventEngineScheduler::getPlayMessageIDs();
	auto it = messageIDs.find(name);

	if (it != messageIDs.end()) {
		id = it->second;
	}

    if (loaded) {
        if (played) {
            measureMessages.emplace_back(type, id, name);
            return measureMessages.back();
        }

        playMessages.emplace_back(type, id, name);
        return playMessages.back();
    }

    loadMessages.emplace_back(type, id, name);
	
	return loadMessages.back();
}
