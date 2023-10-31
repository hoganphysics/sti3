
#include "TestDevice.h"

#include <iostream>

using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;


TestDevice::TestDevice(const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(config)
{
	// *** Define channels *** //
	addOutputChannel(0, MixedValueType::Double, "coil current");		//channel 0, must be a double
	addOutputChannel(1, MixedValueType::Int, "temperature setpoint");	//channel 1, must be an integer
	addInputChannel(11, MixedValueType::Number, MixedValueType::Vector, "vector args");	//measures a number (input); accepts a vector argument (output)

	// *** Define attributes *** //
	addAttribute("x", "22");
	addAttribute("TriggerSource", "Hardware", {"Hardware", "Software"});


	// *** Logging examples *** //

	log() << "Constructing TestDevice";		//write to default log

	log("testing") << "A log comment";		//write to a custom log named "testing"

	// Examples of scheduling regularly repeating log tasks:
	// The time interval between tasks is a string with format hh:mm:ss
	
	// Every 5 seconds, log the value of attribute 'x'
	log().addAttributeLogTask("x", "00:00:05");

	// Every 6 seconds, set channel 0 to 11.2 and log the result
	log().addWriteLogTask(0, "00:00:06", 11.2);
	
	
	MixedValue val;
	val.addValue(5.7);
	val.addValue("example data");

	// Every 2 seconds, read channel 11 and log the result
	log("testing").addReadLogTask(11, "00:00:02", val);


	// Log tasks can also use a custom function to generate the values used by read and write tasks.

	testValue = 0.5;	// Member variable used by log task generating functions, below

	// Every 4 seconds, write to channel 0 with a custom value generating function, and log the result.
	// Using std::bind here so we can callback to a member function.
	log("testing").addWriteLogTask(0, "00:00:03", std::bind(&TestDevice::getTestValue, this));

	// Every 5 seconds, read ch 11 using generating function and log the result (here using a lambda function)
	log("testing").addReadLogTask(11, "00:00:05", 
		[this]() {
			std::cout << "Read task: testValue = " << testValue << std::endl;
			MixedValue val;
			val.addValue(testValue);
			val.addValue("more example data");
			return val;		// this will be used as the value argument for read(...) in the task
		});

}

double TestDevice::getTestValue()
{
	std::cout << "Write task: testValue = " << testValue << std::endl;
	testValue += 1;
    return testValue;	// this will be used as the value argument for write(...) in the task defined above
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
	}

	return success;
}

bool TestDevice::readChannel(short channel, const STI::Utils::MixedValue& value, STI::Utils::MixedValue& data)
{
	bool success = false;

	switch (channel) {
	case 11:
		//Input/Output
		//vector arguments (output)
		if (value.isType({ MixedValueType::Number, MixedValueType::String })) {
			data.setValue(12.2 * value.getVector().at(0).getNumber());	//double measurement (input)
			success = true;	
		}
		break;
	}
	return success;
}
