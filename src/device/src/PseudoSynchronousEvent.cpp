

#include "PseudoSynchronousEvent.h"
#include <sti/engine/EventParsingException.h>
#include <sti/engine/Measurement.h>
#include <sti/utils/utils.h>


using STI::Engine::PseudoSynchronousEvent;


PseudoSynchronousEvent::PseudoSynchronousEvent(double time, const STI::Engine::RawEventVector& eventsIn, STI::Device::Device* device)
: SynchronousEvent(time), device(device)
{
	for (auto& e : eventsIn) {
		if (e.target().isAbstract()) {
			throw STI::Engine::EventParsingException(e, "Target channel is abstract.");
		}
		if (e.isMeasurementEvent()) {
			addMeasurement(e);
		}
		events.push_back(e);
	}
}

PseudoSynchronousEvent::~PseudoSynchronousEvent()
{
}

void PseudoSynchronousEvent::loadEvent()
{
}

void PseudoSynchronousEvent::playEvent()
{
	if (device == 0) return;

	bool hasMeasurements = false;
	operationsComplete = false;

	for (auto& e : events) {
		if (e.isMeasurementEvent()) {
			hasMeasurements = true;
		}
		else {
			if (!device->write(e.channel(), e.value())) {
				addError("Device write failed")
					<< "Device::write returned false for channel #" << e.channel()
					<< " at time " << STI::Utils::printTimeFormated(e.time()) << ".";
			}
		}
	}

	operationsComplete = !hasMeasurements;
}

void PseudoSynchronousEvent::collectMeasurementData()
{
	if (device == 0) return;

	unsigned measurementNumber = 0;

	for (auto& e : events) {
		if (e.isMeasurementEvent()) {
			STI::Utils::MixedValue data;

			const bool success = device->read(e.channel(), e.value(), data);
			if (!success) {
				addError("Device read failed")
					<< "Device::read returned false for channel #" << e.channel()
					<< " at time " << STI::Utils::printTimeFormated(e.time()) << ".";
			}
			else if (getMeasurements().size() > measurementNumber && getMeasurements().at(measurementNumber) != 0) {
				getMeasurements().at(measurementNumber)->setMeasurementResult(data);
			}
			measurementNumber++;
		}
	}

	operationsComplete = true;
}

void PseudoSynchronousEvent::stopEvent()
{
	if (device == 0) return;
	
	// An engine error stops every synchronous event.  Avoid recursively
	// cancelling the engine after this event's synchronous I/O has already
	// returned; stopRW() is only useful while measurement collection is active.
	if (!operationsComplete) {
		device->stopRW();
	}
}

