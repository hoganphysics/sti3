
#include "TestDevice.h"

#include <sti/utils/AppointmentTask.h>
#include <sti/utils/IntervalTask.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

using STI::Utils::AppointmentTask;
using STI::Utils::IntervalTask;
using STI::Utils::Task;

using STI::Utils::MixedValue;
using STI::Utils::MixedValueType;


// Optional: Define a custom Task class
// This is only necessary if the built-in Task types are not sufficient.
class CustomTask : public Task
{
public:

	CustomTask(const std::string& taskID) : Task(taskID) 
	{
		targetTime.add_sec(3);		//now + 3 seconds
	}

	// *** Implementing Task interface *** //

	double secondsToNextRun() const
	{
		STI::Utils::TimeStamp now;
		if (now < targetTime) {
			return 1;
		}
		else {
			return -1;
		}
	}

	void run()
	{
		std::cout << "***** custom task *****" << std::endl;

		STI::Utils::TimeStamp now;
		targetTime = now;
		targetTime.add_sec(3);	//now + 3 seconds
	}

	void skipTask() {}
	bool repeat() { return true; }

private:
	STI::Utils::TimeStamp targetTime;	//time of next task run
};



TestDevice::TestDevice(const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(config)
{
	// *** Define channels *** //
	addOutputChannel(0, MixedValueType::Double, "coil current");		//channel 0, must be a double
	addOutputChannel(1, MixedValueType::Int, "temperature setpoint");	//channel 1, must be an integer
	addInputChannel(11, MixedValueType::Number, "magnetic field");		//measures a number (input)

	// *** Define attributes *** //
	addAttribute("x", "22");
	addAttribute("TriggerSource", "Hardware", {"Hardware", "Software"});


	// *** Task examples *** //

	// Add a task that runs a lambda function every 2 seconds
	auto task1 = std::make_shared<IntervalTask>("task#1", "00:00:02", 
		[this]() {
			MixedValue data;
			read(11, data);		// do something...
			std::cout << "Running IntervalTask: " << data.print() << std::endl;
		});
	addTask(task1);


	STI::Utils::TimeStamp appointmentTime;	//time now, at startup
	appointmentTime.add_sec(5);				//now + 5 seconds

	// Add a task that runs once, five seconds after the program starts, and repeats everday at the same time
	auto task2 = std::make_shared<AppointmentTask>("task#2", appointmentTime.time_hh_mm_ss(":"), 
		AppointmentTask::AppointmentRepeatType::Everyday,
		std::bind(&TestDevice::taskFunction, this));	//using std::bind to attach a class member function

	addTask(task2);


	// Add a custom Task type
	auto task3 = std::make_shared<CustomTask>("task#3");
	addTask(task3);
}

void TestDevice::taskFunction()
{
	std::cout << ">>>>>> AppointmentTask!" << std::endl;
	write(1, 15);	// do something...
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
		//No arguments
        data.setValue(23.4);
		success = true;	
		break;
	}
	return success;
}
