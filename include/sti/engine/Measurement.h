#ifndef STI_ENGINE_MEASUREMENT_H
#define STI_ENGINE_MEASUREMENT_H

#include <sti/fwd/Measurement_fwd.h>

#include <sti/utils/MixedValue.h>
#include <sti/device/DeviceID.h>
#include <sti/utils/GraphPathLabel.h>

#include <string>

namespace STI
{
namespace Engine
{

class RawEvent;

class Measurement
{
public:
	
	Measurement();	//for serialzation

	Measurement(double time, unsigned short channel, const STI::Device::DeviceID& device, 
							const STI::Utils::GraphPathLabel& measurementGraphPath);
	Measurement(const RawEvent& sourceEvent);
	Measurement(const Measurement& measurement);

	void setMeasurementResult(const STI::Utils::MixedValue& result);
	void extractMeasurementResult(STI::Utils::MixedValue& data);

	bool dataReady() const;

	double time() const;
	unsigned short channel() const;
	const STI::Utils::MixedValue& data() const;

	const STI::Device::DeviceID& device() const;

	const std::vector<unsigned>& getMeasurementGraphPath() const { return measurementGraphPath; }

	std::string print() const;

	bool operator<(const Measurement& rhs) const { 
		return time() < rhs.time() ||
			( time() == rhs.time() && ( device() < rhs.device() || 
			( device() == rhs.device() && channel() < rhs.channel() ) ) );
	}
	bool operator==(const Measurement& rhs) const { return measurementGraphPath == rhs.measurementGraphPath; }
	bool operator!=(const Measurement& rhs) const { return !((*this) == rhs); }

	template<class Archive>
	void serialize(Archive& archive);

private:

//	const RawEvent& sourceEvent;
	STI::Utils::MixedValue measurementResult;
	bool data_ready;
	
	double         _time;
	unsigned short _channel;

	STI::Utils::GraphPathLabel measurementGraphPath;

	STI::Device::DeviceID _device;

};


} //Engine
} //STI

#endif

