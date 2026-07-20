
#include "TestDevice.h"

#include <iostream>


TestDevice::TestDevice(const STI::Utils::Configuration& config)
: STI::Device::LocalDevice(config), hardwareTrigger(true), downsample(1), height(4.6),
  regionWidth(640), regionHeight(480)
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

	// These attributes describe one hardware region. Changing either dimension
	// also changes the derived pixel count, so refresh them as one transaction.
	addAttribute("Region::Width", regionWidth)
		.setSetter([this](const std::string& value) -> bool {
			int width;
			if (STI::Utils::stringToValue(value, width) && width > 0) {
				regionWidth = width;
				return true;
			}
			return false;
		})
		.setRefresher([this]() {
			return STI::Utils::valueToString(regionWidth);
		});

	addAttribute("Region::Height", regionHeight)
		.setSetter([this](const std::string& value) -> bool {
			int height;
			if (STI::Utils::stringToValue(value, height) && height > 0) {
				regionHeight = height;
				return true;
			}
			return false;
		})
		.setRefresher([this]() {
			return STI::Utils::valueToString(regionHeight);
		});

	addAttribute("Region::PixelCount", regionWidth * regionHeight)
		.setSetter([](const std::string&) { return false; }) //derived, read-only
		.setRefresher([this]() {
			return STI::Utils::valueToString(regionWidth * regionHeight);
		});

	addAttributeRefreshGroup({
		"Region::Width",
		"Region::Height",
		"Region::PixelCount"
	});


	// *** Attribute I/O examples *** //
	std::string modeResult = getAttribute("Mode");
	setAttribute("Downsample", "4");

	// If member state changes outside setAttribute(), explicitly refresh the
	// attribute cache to publish the new string value.
	height = 6.2;
	refreshAttribute("Height");

	// The setter runs first, then Width, Height, and PixelCount each refresh once.
	setAttribute("Region::Width", "800");

	// A hardware-side change can be synchronized through any group member.
	regionWidth = 1024;
	regionHeight = 768;
	refreshAttribute("Region::PixelCount");

	downsample = 2;
	hardwareTrigger = false;
	refreshAttributes();

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
