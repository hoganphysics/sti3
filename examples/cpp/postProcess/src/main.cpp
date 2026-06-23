#include <sti/sti.h>
#include <sti/device/PostProcessingManager.h>

#include "AnalysisDevice.h"

#include <memory>
#include <iostream>
#include <string>

using STI::Utils::Configuration;
using STI::Network::NetworkDeviceHub;


int main(int argc, char** argv)
{
	Configuration config({
		{"Device Name", "AnalysisDevice"},
		{"IP Address", "localhost"},
		{"Module", "0"},
		{"Target Server", "localhost/0/STI Server"} });

	auto device = std::make_shared<AnalysisDevice>(config);

	// Discovery: list the post-processing targets the device offers, each with its
	// name, description, and declared option hints.
	std::shared_ptr<STI::Device::PostProcessingManager> postProcessing;
	if (device->getPostProcessingManager(postProcessing) && postProcessing != 0) {
		for (const auto& target : postProcessing->getPostProcessingTargets()) {
			std::cout << "post-processing target: " << target.name
			          << " - " << target.description << std::endl;
			for (const auto& option : target.options) {
				std::cout << "    option: " << option.name
				          << " - " << option.description << std::endl;
			}
		}
	}

	std::string nameServiceAddr = "192.168.1.4:2809";   //OmniORB NameService
	auto hub = std::make_shared<NetworkDeviceHub>(nameServiceAddr);

	hub->addDevice(device);
	hub->run();     	//blocks until ctrl-c or Device terminates

	return 0;
}
