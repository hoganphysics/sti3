
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
	addOutputChannel(3, MixedValueType::Vector, "list output");			//channel 3, vector tuple of outputs, format checked by device
	addOutputChannel(4, MixedValueType::String, "string output");

	// Input channels (make measurements that are recorded by the device)
	addInputChannel(10, MixedValueType::Number, "thermocouple voltage");		// measures a number

	// Input/Output channel
	addInputChannel(11, MixedValueType::Vector, MixedValueType::Number, "thermocouple voltage");	//outputs a number to measure a vector

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
	case 3:
		//list output
		std::cout << "Ch:" << channel << ", " << "list output: ";
		
		//Example of vector type checking
		if (value.isType({ MixedValueType::Number, MixedValueType::String, MixedValueType::Boolean })) {
			//do something...
		}

		//print args
		{
			auto& tuple = value.getVector();
			for (auto& arg : tuple) {
				std::cout << "  " << arg.print() << std::endl;
			}
			std::cout << std::endl;
			success = true;
		}
		break;
	case 4:
		//string output
		std::cout << "Ch:" << channel << ", " << "string output: " << value.getString() << std::endl;
		success = true;
		break;
	default:
		break;
	}

	return success;
}

bool TestDevice::readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
	return false;
}
