
#include <sti/sti.h>
#include "TestDevice.h"


#include <string>
#include <memory>
#include <iostream>




int main(int argc, char **argv)
{
	std::string configFilename = "config.ini"; //default

	if (argc > 1) {
		configFilename = argv[1];
	}

	STI::Utils::ConfigFile config(configFilename);
    std::string nameServiceAddr = "192.168.1.6:2809";   //Address of OmniORB NameService (to connect to other Hubs)
	
	auto device = std::make_shared<TestDevice>(config);

	auto hub = std::make_shared<STI::Network::NetworkDeviceHub>(nameServiceAddr, config);

	hub->addDevice(device);

	hub->run(true);     //blocks until ctrl-c or Device terminates
	hub->shutdown();
	// int x;
	// std::cin >> x;

	return 0;
}
