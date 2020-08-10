#ifndef STI_ENGINE_MEASUREMENT_H
#define STI_ENGINE_MEASUREMENT_H

#include "MixedValue.h"
#include "DeviceID.h"
#include "utils/GraphPathLabel.h"

namespace STI
{
namespace Engine
{

class RawEvent;

class Measurement
{
public:
	
	Measurement(const RawEvent& sourceEvent);
	Measurement::Measurement(const Measurement& measurement);

	void setMeasurementResult(const STI::Utils::MixedValue& result);
	void extractMeasurementResult(STI::Utils::MixedValue& data);

	bool dataReady() const;

	double time() const;
	unsigned short channel() const;
	const STI::Utils::MixedValue& data() const;

	const STI::Device::DeviceID& device() const;

	const std::vector<unsigned>& getMeasurementGraphPath() const { return measurementGraphPath; }

private:

//	const RawEvent& sourceEvent;
	STI::Utils::MixedValue measurementResult;
	bool data_ready;
	
	double         _time;
	unsigned short _channel;

	STI::Utils::GraphPathLabel measurementGraphPath;

	const STI::Device::DeviceID _device;

};


} //Engine
} //STI

#endif

