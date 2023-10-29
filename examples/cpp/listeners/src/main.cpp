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
		{"Device Name", "TestListenerDevice"},
		{"IP Address", "localhost"},
		{"Module", "0"},
		{"Target Server", "localhost/0/STI Server"} });

	auto device = std::make_shared<TestDevice>(config);

	device->setAttribute("x", "34");
	device->write(0, 52.1);

	std::string nameServiceAddr = "192.168.1.4:2809";   //OmniORB NameService
	auto hub = std::make_shared<NetworkDeviceHub>(nameServiceAddr);

	hub->addDevice(device);
	hub->run();     	//blocks until ctrl-c or Device terminates
	hub->shutdown();

	return 0;
}
