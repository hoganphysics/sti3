
#include "TestDevice.h"

#include <iostream>

using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;


TestDevice::TestDevice(const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(config)
{
	// *** Define channels *** //

	// Output channels (the device's output actuators)
	addOutputChannel(0, MixedValueType::Double, "coil current");		//channel 0, must be a double
	addOutputChannel(1, MixedValueType::Int, "temperature setpoint");	//channel 1, must be an integer
	addOutputChannel(2, MixedValueType::Number, "supply voltage");		//channel 2, any numeric type

	addAttribute("Downsample", 1)
		.setSetter([this](const std::string& value) -> bool {
			int ds;
			if (STI::Utils::stringToValue(value, ds) && ds > 0) {
				downsample = ds;
				return true;	//success
			}
			return false;	//illegal value; set failed
		})
		.setRefresher([this]() -> std::string {
			//refresh string attribute value with downsample integer
			return STI::Utils::valueToString(downsample);
		});
	
	addAttribute("Height", 1)
		.setSetter([this](const std::string& value) -> bool {
			double val;
			if (STI::Utils::stringToValue(value, val)) {
				height = val;
				return true;	//success
			}
			return false;	//illegal value; set failed
		})
		.setRefresher([this]() -> std::string {
			//refresh string attribute value with downsample integer
			return STI::Utils::valueToString(height);
		});

}

bool TestDevice::writeChannel(short channel, const STI::Utils::MixedValue& value)
{
	bool success = false;

	switch (channel)
	{
	case 0:
		//coil current (Double)
		std::cout << "Ch:" << channel << ", " << "coil current: " << value.getDouble() << std::endl;
		success = true;
		break;
	case 1:
		//temperature setpoint (Int)
		std::cout << "Ch:" << channel << ", " << "temperature setpoint: " << value.getInt() << std::endl;
		success = true;
		break;
	case 2:
		//supply voltage
		std::cout << "Ch:" << channel << ", " << "supply voltage: " << value.getNumber() << std::endl;
		success = true;
		break;
	default:
		break;
	}

	return success;
}

bool TestDevice::readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
	bool success = false;

	return success;
}
