#include <sti/sti.h>

#include "TestDevice.h"

#include <memory>
#include <iostream>
#include <sstream>

using STI::Utils::Configuration;
using STI::Network::NetworkDeviceHub;


int main(int argc, char** argv)
{
	Configuration config1({
		{"Device Name", "TestServer"},
		{"IP Address", "localhost"},
		{"Module", "1"},
		{"Target Server", "localhost/0/STI Server"} });
	
	Configuration config2({
		{"Device Name", "TestDevice"},
		{"IP Address", "localhost"},
		{"Module", "2"},
		{"Target Server", "localhost/1/TestServer"} });

	auto device1 = std::make_shared<TestDevice>(config1);
	auto device2 = std::make_shared<TestDevice>(config2);

	std::string nameServiceAddr = "192.168.1.109:2809";   //OmniORB NameService
	auto hub = std::make_shared<NetworkDeviceHub>(nameServiceAddr);

	hub->addDevice(device1);
	hub->addDevice(device2);
	hub->run();     	//blocks until ctrl-c or Device terminates

	return 0;
}

