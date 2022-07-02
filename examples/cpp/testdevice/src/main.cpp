
#include <sti/sti.h>
#include "TestDevice.h"


#include <string>
#include <memory>
#include <iostream>
#include <map>

void test(const std::map<std::string, std::string>& dict)
{

}

void test2(const std::map<std::string, std::map<std::string, std::string>>& dict)
{

}

int main(int argc, char **argv)
{

//	std::cout << STI::Network::NetworkDeviceHub::printNetwork("192.168.1.4:2809", "STI") << std::endl;

	std::string configFilename = "config.ini"; //default

	if (argc > 1) {
		configFilename = argv[1];
	}

	STI::Utils::ConfigFile config(configFilename);

	auto device = std::make_shared<TestDevice>(config);

    std::string nameServiceAddr = "192.168.1.6:2809";   //Address of OmniORB NameService (to connect to other Hubs)
	

	std::map<std::string, std::string> m = {{"Name", "TestDev"},{"Address", "localhost"}};

	test({{"Name", "TestDev"}, {"Address", "localhost"}});

	test2({
		{"dev1", 
			{
				{"Name", "TestDev"}, 
				{"Address", "localhost"}
			}
		},
		{"",
			{
				{"NameService", "192.168.1.0"}
			}
		}
	});

	STI::Utils::Configuration configure(
		{
			{"dev1", 
				{
					{"Name", "TestDev"}, 
					{"Address", "localhost"}
				}
			},
			{"",
				{
					{"NameService", "192.168.1.0"}
				}
			}
		});

	STI::Utils::Configuration configure2( {{"Name", "TestDev"}, {"Address", "localhost"}} );

	auto config3 = configure + configure2;

	// auto hub = std::make_shared<STI::Network::NetworkDeviceHub>(nameServiceAddr,
	// 	STI::Utils::Configuration({
	// 		{"omniORB", 
	// 			{
	// 				{"acceptBiDirectionalGIOP", "0"}, 
	// 				{"traceLevel", "20"}
	// 			}
	// 		}
	// 	}));
	auto hub = std::make_shared<STI::Network::NetworkDeviceHub>(nameServiceAddr, config);

	hub->addDevice(device);

	hub->run(true);     //blocks until ctrl-c or Device terminates
	hub->shutdown();
	// int x;
	// std::cin >> x;

	return 0;
}
