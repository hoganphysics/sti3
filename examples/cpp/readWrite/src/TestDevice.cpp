
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
	addInputChannel(10, MixedValueType::Number, "thermocouple voltage");		// measures a number (input)

	// Input/Output channel
	addInputChannel(11, MixedValueType::Number, MixedValueType::Vector, "vector args");	//measures a number (input); accepts a vector argument (output)
	addInputChannel(12, MixedValueType::Vector, MixedValueType::Number, "vector measurement");	//measures a vector (input), accepts a number argument (output)

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
	bool success = false;

	switch (channel) {
	case 10:
		//thermocouple voltage
		data.setValue(34.5);
		success = true;
		break;
	case 11:
		//Input/Output
		//vector arguments (output)
		if (value.isType({ MixedValueType::Number, MixedValueType::String })) {
			data.setValue(12.2 * value.getVector().at(0));	//double measurement (input)
			success = true;	
		}
		break;
	case 12:
		//Input/Output
		auto arg = value.getNumber();	//number argument (output)
		
		data.addValue(3.2 * arg);	//vector measurement (input)
		data.addValue("example string result")
		data.addValue(true);

		success = true;
		break;
	default:
		break;
	}
	return success;
}
