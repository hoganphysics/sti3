
#include "Measurement.h"
#include "RawEvent.h"
#include "DeviceID.h"
#include "utils.h"

#include <sstream>

#include "CerealArchives.h"
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>

using STI::Engine::Measurement;
using STI::Engine::RawEvent;
using STI::Device::DeviceID;
using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;


Measurement::Measurement()
{
}

Measurement::Measurement(double time, unsigned short channel, const STI::Device::DeviceID& device, 
							const STI::Utils::GraphPathLabel& measurementGraphPath)
: _time(time), _channel(channel),  _device(device), measurementGraphPath(measurementGraphPath), data_ready(false)
{
}

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

std::string Measurement::print() const
{
	std::stringstream meas;

	//<Time=2.1, Channel=4, Type=Number, Value=3.4>
	meas << "<Time=" << STI::Utils::printTimeFormated(time());
	meas << ", Channel=" << channel();
	meas << ", Type=";

	meas << MixedValue::TypeToString(data().getType());
	meas << ", Value=" << data().print() << ">";
	
	return meas.str();
}

template<class Archive>
void Measurement::serialize(Archive& archive)
{
	archive(
		cereal::make_nvp("time", _time), 
		cereal::make_nvp("channel", _channel), 
		cereal::make_nvp("device", _device), 
		cereal::make_nvp("measurementResult", measurementResult),
		cereal::make_nvp("measurementGraphPath", measurementGraphPath), 
		cereal::make_nvp("data_ready", data_ready)
		);
}

template void Measurement::serialize<cereal::XMLOutputArchive>( cereal::XMLOutputArchive& );
template void Measurement::serialize<cereal::XMLInputArchive>( cereal::XMLInputArchive& );
