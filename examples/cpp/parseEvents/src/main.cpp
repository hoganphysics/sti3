#include <sti/sti.h>

#include "TestDevice.h"

#include <memory>
#include <iostream>
#include <sstream>

using STI::Utils::Configuration;
using STI::Network::NetworkDeviceHub;


int main(int argc, char** argv)
{
	Configuration config({
		{"Device Name", "TestDevice"},
		{"IP Address", "localhost"},
		{"Module", "0"},
		{"Target Server", "sr-magis/2/Frame2"} });

	auto device = std::make_shared<TestDevice>(config);

	std::string nameServiceAddr = "192.168.1.109:2809";   //OmniORB NameService
	auto hub = std::make_shared<NetworkDeviceHub>(nameServiceAddr);

	hub->addDevice(device);
	hub->run();     	//blocks until ctrl-c or Device terminates

	return 0;
}
