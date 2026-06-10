
#include "ServerDevice.h"

#include "LocalEventEngineScheduler.h"

#include <sti/device/DeviceMessageReceiver.h>
#include <iostream>
#include <memory>
#include <vector>

using STI::Device::ServerDevice;
using STI::Engine::LocalEventEngineScheduler;
using STI::Engine::SequenceSchedulingMode;

namespace {

const std::string SequenceModeAttribute = "Sequence Mode";

} // namespace


ServerDevice::ServerDevice(const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(config)
{
	addChannel(0, STI::Device::ChannelType::Output, STI::Utils::MixedValueType::Empty, STI::Utils::MixedValueType::Double, "testch");

	addAttribute("test", "45");

	std::shared_ptr<LocalEventEngineScheduler> scheduler;
	if (getEngineScheduler(scheduler) && scheduler != 0) {
		auto currentMode = LocalEventEngineScheduler::sequenceSchedulingModeToString(
			scheduler->getSequenceSchedulingMode());

		std::shared_ptr<STI::Device::LocalAttribute> sequenceModeAttribute;
		addAttribute(SequenceModeAttribute, currentMode, {"Normal", "Interleaved"}, sequenceModeAttribute);

		if (sequenceModeAttribute != 0) {
			std::weak_ptr<LocalEventEngineScheduler> weakScheduler = scheduler;

			sequenceModeAttribute->setSetter(
				[weakScheduler](const std::string& value) {
					auto scheduler = weakScheduler.lock();
					if (scheduler == 0) return false;

					SequenceSchedulingMode mode;
					if (!LocalEventEngineScheduler::parseSequenceSchedulingMode(value, mode)) {
						return false;
					}

					scheduler->setSequenceSchedulingMode(mode);
					return true;
				});

			sequenceModeAttribute->setRefresher(
				[weakScheduler]() {
					auto scheduler = weakScheduler.lock();
					if (scheduler == 0) {
						return LocalEventEngineScheduler::sequenceSchedulingModeToString(
							SequenceSchedulingMode::Normal);
					}

					return LocalEventEngineScheduler::sequenceSchedulingModeToString(
						scheduler->getSequenceSchedulingMode());
				});
		}
	}
}

ServerDevice::~ServerDevice()
{
}

void ServerDevice::parseEvents(const STI::Engine::RawEventMap& eventsIn, STI::Engine::SynchronousEventVector& synchedEvents)
{
	bool inputEvent;

	for (auto& tuple : eventsIn) {
		// Here 'tuple' is of type { time, vector<RawEvent> }, where the vector is the set of events at this time.
		inputEvent = false;

		//check for input event
		for (auto& rawEvent : tuple.second) {
			if (rawEvent.isMeasurementEvent()) {
				inputEvent = true;
				break;
			}
		}

		// Example of error checking. When an error is encountered during parsing, an exception should be thrown.
		// This exception is handled by STI and converted to a parsing error message. 
		if (inputEvent && tuple.second.size() > 1) {
			// Error: In this example, when there is an input event, it must be the only event at that time.
			// The EventConflictException constuctor accepts reference to the two offending events, plus a message: 
			throw STI::Engine::EventConflictException(tuple.second.at(0), tuple.second.at(1), 
				"Error: An input event cannot be at the same time as an output event.");
		}

		if (!inputEvent) {
			//Output event
			auto testDeviceOutputEvent = std::make_shared<ServerDevice::TestDeviceOutputEvent>(tuple.first);	//time
			
			for (auto& rawEvent : tuple.second) {
				
				if (rawEvent.value().getNumber() > 10) {
					throw STI::Engine::EventParsingException(rawEvent, 
					"The value " + rawEvent.value().print() + " exceeds the maximium allowed value for this channel. Max value is 10.");
				}
				testDeviceOutputEvent->addValue(rawEvent.channel(), rawEvent.value());	//attach values to event for use later

			}
			synchedEvents.push_back(testDeviceOutputEvent);
		}
	}
}



// //////////////// TestDeviceOutputEvent //////////////////

// This subclass is used to specify the custom behavior controlling the hardware for each device output channel.

ServerDevice::TestDeviceOutputEvent::TestDeviceOutputEvent(double time)
: STI::Engine::SynchronousEvent(time)
{
}

void ServerDevice::TestDeviceOutputEvent::addValue(short channel, const STI::Utils::MixedValue& value)
{
	values[channel] = value;
}

void ServerDevice::TestDeviceOutputEvent::loadEvent()
{
	// This function is called at the beginning of each shot, before any events are played.
	// Use this function to setup the hardware to prepare for hard timing playback.
	// For example, if this device requires values to be preloaded into some buffer on the hardware
	// (e.g., an FPGA, or an arbitrary waveform generator), this can be done here.

	// Add hardware loading code here...
	for (auto& v : values) {
		std::cout << "Loading channel #" << v.first << " with value " << v.second.getNumber() << "." << std::endl;
	}
}

void ServerDevice::TestDeviceOutputEvent::playEvent()
{
	// This function will be called at time specified in the timing file.
	// Use this function to control the hardware to implement the change on the requested channel.

	// Add hardware play code here...
	for (auto& v : values) {
		std::cout << "Playing channel #" << v.first << " with value " << v.second.getNumber() << "." << std::endl;
	}
}
