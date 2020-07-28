
#include "Measurement.h"
#include "RawEvent.h"

using STI::Engine::Measurement;
using STI::Engine::RawEvent;

Measurement::Measurement(const RawEvent& sourceEvent) : data_ready(false), _device(sourceEvent.targetDevice())
{
	_time = sourceEvent.time();
	_channel = sourceEvent.channel();

	measurementGraphPath = sourceEvent.getEventGraphPath();
}

Measurement::Measurement(const Measurement& measurement) : data_ready(false), _device(measurement._device)
{
	//This copy constructor does NOT copy the measurement data or the measurement status (data_ready).
	//It is called by SynchronousEvent::reset() when preparing the event for another run.
	//Deep copying or moving of data should be done during documentation.

	_time = measurement._time;
	_channel = measurement._channel;

	measurementGraphPath = measurement.getMeasurementGraphPath();
}

void Measurement::setMeasurementResult(const STI::Utils::MixedValue& result)
{
	measurementResult = std::move(result);
	data_ready = true;
}

void Measurement::extractMeasurementResult(STI::Utils::MixedValue& data)
{
	//Used to swap the result stored in measurementResult with the input parameter 'data'.
	data = std::move(measurementResult);
}

bool Measurement::dataReady() const
{
	return data_ready;
}

double Measurement::time() const
{
	return _time;
}

unsigned short Measurement::channel() const
{
	return _channel;
}

const STI::Utils::MixedValue& Measurement::data() const
{
	return measurementResult;
}

const STI::Device::DeviceID& Measurement::device() const
{
	return _device;
}

