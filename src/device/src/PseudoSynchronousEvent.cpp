

#include "PseudoSynchronousEvent.h"
#include <sti/engine/EventParsingException.h>
#include <sti/engine/Measurement.h>


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

	for (auto& e : events) {
		if (!e.isMeasurementEvent()) {
			device->write(e.channel(), e.value());
		}
	}
}

void PseudoSynchronousEvent::collectMeasurementData()
{
	if (device == 0) return;

	unsigned measurementNumber = 0;

	for (auto& e : events) {
		if (e.isMeasurementEvent()) {
			STI::Utils::MixedValue data;
			
			device->read(e.channel(), e.value(), data);
			
			if (getMeasurements().size() > measurementNumber && getMeasurements().at(measurementNumber) != 0) {
				getMeasurements().at(measurementNumber)->setMeasurementResult(data);
			}
			measurementNumber++;
		}
	}
}

void PseudoSynchronousEvent::stopEvent()
{
	if (device == 0) return;
	
	device->stopRW();
}

