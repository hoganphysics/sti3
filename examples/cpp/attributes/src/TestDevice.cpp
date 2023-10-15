
#include "TestDevice.h"

#include <iostream>


TestDevice::TestDevice(const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(config), hardwareTrigger(true)
{
	// *** Define attributes *** //

	//Simple string attribute
	addAttribute("message", "A test message");

	//Float attrribute using class functions for setter/refresher
	addAttribute("Height", height)
		.setSetter(&TestDevice::setHeight, this)
		.setRefresher(&TestDevice::refreshHeight, this);

	//Integer attribute with lambda functions for setter/refresher
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

	//String attribute with list of allowed values
	addAttribute("TriggerSource", "Hardware", { "Hardware", "Software" })
		.setSetter([this](const std::string& value) -> bool {
			hardwareTrigger = value.compare("Hardware") == 0;
			return true;
		})
		.setRefresher([this]() -> std::string {
			return (hardwareTrigger ? "Hardware" : "Software");	//ensure bool and attribute are consistent
		});

	//Attribute with meta data
	addAttribute("Mode", "Mean", { "Mean", "Total" })
		.setSetter([](const std::string& value) -> bool {
			std::cout << "New mode is " << value << std::endl;
			return true;
			})
		.addMetaData("help", "Sets the mode of the device.")	//example meta data
		.addMetaData("type", "string attribute");				//example meta data



	// *** Attribute I/O examples *** //
	std::string modeResult = getAttribute("Mode");
	setAttribute("Downsample", "4");

}

bool TestDevice::setHeight(const std::string& value)
{
	double newHeight;
	STI::Utils::stringToValue(value, newHeight);

	if (newHeight > 0 && newHeight < 14.7) {	//bounds checking
		height = newHeight;
		return true;	//success
	}
	return false;
}

std::string TestDevice::refreshHeight()
{
	return STI::Utils::valueToString(height);
}
